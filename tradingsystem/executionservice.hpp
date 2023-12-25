/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include "soa.hpp"
#include <string>
#include "marketdataservice.hpp"
#include "algoexecutionservice.hpp"

//enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

//enum Market { BROKERTEC, ESPEED, CME };



/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
/*
template<typename T>
class ExecutionOrder
{

public:

 // ctor for an order
 ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

 // Get the product
 const T& GetProduct() const;

 // Get the order ID
 const string& GetOrderId() const;

 // Get the order type on this order
 OrderType GetOrderType() const;

 // Get the price on this order
 double GetPrice() const;

 // Get the visible quantity on this order
   long GetVisibleQuantity() const;

  // Get the hidden quantity
  long GetHiddenQuantity() const;

  // Get the parent order ID
  const string& GetParentOrderId() const;

  // Is child order?
  bool IsChildOrder() const;

private:
  T product;
  PricingSide side;
  string orderId;
  OrderType orderType;
  double price;
  double visibleQuantity;
  double hiddenQuantity;
  string parentOrderId;
  bool isChildOrder;

};
*/

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */

/*
template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
  product(_product)
{
  side = _side;
  orderId = _orderId;
  orderType = _orderType;
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  parentOrderId = _parentOrderId;
  isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
  return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
  return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
  return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
  return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
  return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
  return isChildOrder;
}
*/

/**
 * Pre-declearations
 * establishes a listener connection that activates upon triggering
 * from the algorithm execution class defined in the hpp file.
 * links algo execution class to this broader execution service class.
*/
template<typename T>
class AlgoExecutionToExecutionListener;

/**
* General order execution service.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class ExecutionService : public Service<string, ExecutionOrder<T>>
{
private:
    map<string, ExecutionOrder<T>> executionOrders;
    vector<ServiceListener<ExecutionOrder<T>>*> listeners;
    AlgoExecutionToExecutionListener<T>* listener;

public:

    // Ctor
    ExecutionService(){
        executionOrders = map<string, ExecutionOrder<T>>();
        listeners = vector<ServiceListener<ExecutionOrder<T>>*>();
        listener = new AlgoExecutionToExecutionListener<T>(this);
    }

    // Get data by key
    ExecutionOrder<T>& GetData(string _id){
        return executionOrders[_id];
    }

    // Callback for any new or updated data
    void OnMessage(ExecutionOrder<T>& _data){
        string _id = _data.GetProduct().GetProductId();
        executionOrders[_id] = _data;
    }

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<ExecutionOrder<T>>* _listener){
        listeners.push_back(_listener);
    }

    // Get all listeners
    const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const{
        return listeners;
    }

    // Get the listener of the service
    AlgoExecutionToExecutionListener<T>* GetListener(){
        return listener;
    }

    // Execute an order
    void ExecuteOrder(ExecutionOrder<T>& _executionOrder);
};

template<typename T>
void ExecutionService<T>::ExecuteOrder(ExecutionOrder<T>& _executionOrder)
{
    executionOrders[_executionOrder.GetProduct().GetProductId()] = _executionOrder;

    // call the listeners
    for (auto& l : listeners)
    {
        l->ProcessAdd(_executionOrder);
    }
}

/**
 * Execution Service Listener.
 * Facilitates data subscription from algorithm execution to execution service.
 * This class primarily interfaces with the algorithm execution.
 * Type T is the product type.
*/
template<typename T>
class AlgoExecutionToExecutionListener : public ServiceListener<AlgoExecution<T>>
{
public:
    // ctor
    AlgoExecutionToExecutionListener(ExecutionService<T>* _service){
        service = _service;
    }

    // Process an add event to the Service
    void ProcessAdd(AlgoExecution<T>& _data);

    // Process a remove event to the Service
    void ProcessRemove(AlgoExecution<T>& _data) {}

    // Process an update event to the Service
    void ProcessUpdate(AlgoExecution<T>& _data) {}

private:
    ExecutionService<T>* service;
};

/**
 * We call algo in this function.
 * @tparam T
 * @param _data
 */
template<typename T>
void AlgoExecutionToExecutionListener<T>::ProcessAdd(AlgoExecution<T>& _data)
{
    // call algo to execute the order
    ExecutionOrder<T>* execution_order = _data.GetExecutionOrder();

    // update the info and execute
    service->OnMessage(*execution_order);
    service->ExecuteOrder(*execution_order);
}

#endif
