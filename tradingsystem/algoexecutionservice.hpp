/**
  * algoexecutionservice.hpp
  * Defines the data types and Service for executions.
  * @author Breman Thuraisingham & Lexie Zhu
  */

#ifndef ALGO_EXECUTION_SERVICE_HPP
#define ALGO_EXECUTION_SERVICE_HPP
#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"
#include "utilities.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:

	// ctor for an order
	ExecutionOrder() = default;
	ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

	// Get the product
	const T& GetProduct() const;

	// Get the pricing side
	PricingSide GetPricingSide() const;

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

	// Store attributes as strings
	vector<string> ToStrings() const;

private:
	T product;
	PricingSide side;
	string orderId;
	OrderType orderType;
	double price;
	long visibleQuantity;
	double hiddenQuantity;
	string parentOrderId;
	bool isChildOrder;

};

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
	product(_product)
{
	side = _side;
	orderId = _orderId;
	orderType = _orderType;
	price = _price;
	visibleQuantity = static_cast<long>(_visibleQuantity);
	hiddenQuantity = static_cast<long>(_hiddenQuantity);
	parentOrderId = _parentOrderId;
	isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
	return product;
}

template<typename T>
PricingSide ExecutionOrder<T>::GetPricingSide() const
{
	return side;
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

template<typename T>
vector<string> ExecutionOrder<T>::ToStrings() const
{
	string _product = product.GetProductId();
	string _side;
	_side = side == BID ? "BID" : "OFFER";
	string _orderId = orderId;
	string _orderType;
	if (orderType == FOK) {
		_orderType = "FOK";
	}
	if (orderType == IOC) {
		_orderType = "IOC";
	}
	if (orderType == MARKET) {
		_orderType = "MARKET";
	}
	if (orderType == LIMIT) {
		_orderType = "LIMIT";
	}
	if (orderType == STOP) {
		_orderType = "STOP";
	}
	
	string _price = PriceToString(price);
	string _visibleQuantity = to_string(visibleQuantity);
	_visibleQuantity = _visibleQuantity.substr(0, _visibleQuantity.find(".") + 1);
	string _hiddenQuantity = to_string(hiddenQuantity);
	_hiddenQuantity = _hiddenQuantity.substr(0, _hiddenQuantity.find(".") + 1);
	string _parentOrderId = parentOrderId;
	string _isChildOrder = isChildOrder ? "YES" : "NO";

	vector<string> _strings{ _product,_side,_orderId,_orderType,_price,
	_visibleQuantity, _hiddenQuantity,_parentOrderId,_isChildOrder };
	return _strings;
}

/* Declaration of the algo execution class
coming from an execution order*/
template<typename T>
class AlgoExecution
{
public:
	// ctor for an order
	AlgoExecution() = default;
	AlgoExecution(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

	// Get the order
	ExecutionOrder<T>* GetExecutionOrder() const;

private:
	ExecutionOrder<T>* executionOrder;

};

// implementation of algo execution
template<typename T>
AlgoExecution<T>::AlgoExecution(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder)
{
	executionOrder = new ExecutionOrder<T>(_product, _side, _orderId, _orderType, _price, _visibleQuantity, _hiddenQuantity, _parentOrderId, _isChildOrder);
}

template<typename T>
ExecutionOrder<T>* AlgoExecution<T>::GetExecutionOrder() const
{
	return executionOrder;
}


/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class AlgoExecutionToMarketDataListener;

/**
* Service for algo_executing orders.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{
public:

	// Constructor and destructor
	AlgoExecutionService();
	~AlgoExecutionService();

	// Get data on our service given a key
	AlgoExecution<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(AlgoExecution<T>& _data);

	// Add a listener to the Service for callbacks
	void AddListener(ServiceListener<AlgoExecution<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<AlgoExecution<T>>*>& GetListeners() const;

	// Get the algo_ex to market_data listener of the service
	AlgoExecutionToMarketDataListener<T>* GetListener();

	// Execute an order on a market
	void AlgoOrderExecution(OrderBook<T>& _orderBook);

private:
	map<string, AlgoExecution<T>> algoExecutions;
	vector<ServiceListener<AlgoExecution<T>>*> listeners;
	AlgoExecutionToMarketDataListener<T>* listener;
	double SPREAD_LIMIT;
	long executionCount;
};

// implementation of the algo execution service
// constructor; set the spread to be 1.0/128.0
template<typename T>
AlgoExecutionService<T>::AlgoExecutionService()
{
	algoExecutions = map<string, AlgoExecution<T>>();
	listeners = vector<ServiceListener<AlgoExecution<T>>*>();
	listener = new AlgoExecutionToMarketDataListener<T>(this);
	SPREAD_LIMIT = 1.0 / 128.0;
	executionCount = 0;
}

template<typename T>
AlgoExecutionService<T>::~AlgoExecutionService() {}

template<typename T>
AlgoExecution<T>& AlgoExecutionService<T>::GetData(string _id)
{
	return algoExecutions[_id];
}

template<typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecution<T>& _data)
{
	string _id = _data.GetExecutionOrder()->GetProduct().GetProductId();
	algoExecutions[_id] = _data;
}

template<typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<AlgoExecution<T>>*>& AlgoExecutionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
AlgoExecutionToMarketDataListener<T>* AlgoExecutionService<T>::GetListener()
{
	return listener;
}

// the core function of this class: algo order execution
// we only to the trade when the spread is within the limit.
template<typename T>
void AlgoExecutionService<T>::AlgoOrderExecution(OrderBook<T>& _orderBook)
{
	T _product = _orderBook.GetProduct();
	string _productId = _product.GetProductId();
	PricingSide _side;
	string _orderId = GenerateTradingId();
	double _price;
	long _quantity;

	BidOffer currBidOffer = _orderBook.GetBidOffer();
	Order bid_order = currBidOffer.GetBidOrder();
	Order offer_order = currBidOffer.GetOfferOrder();

	double bid_price = bid_order.GetPrice();
	long bid_quantity = bid_order.GetQuantity();
	double offer_price = offer_order.GetPrice();
	long offer_quantity = offer_order.GetQuantity();

	// trade only when the spread is within the limit!
	if (offer_price - bid_price <= SPREAD_LIMIT)
	{
		// we have: BID comes first then offer
		if (executionCount % 2) {
			_price = bid_price;
			_quantity = bid_quantity;
			_side = BID;
		}
		else {
			_price = offer_price;
			_quantity = offer_quantity;
			_side = OFFER;
		}
		executionCount++;

		AlgoExecution<T> algoOrder(_product, _side, _orderId, MARKET, _price, _quantity, 0, "PARENT_ORDER_ID", false);
		algoExecutions[_productId] = algoOrder;

		// notify the listners of the execution
		for (auto& l : listeners)
		{
			l->ProcessAdd(algoOrder);
		}
	}
}

/**
* The service listener connection algoexecution to marketdata listener
*/
template<typename T>
class AlgoExecutionToMarketDataListener : public ServiceListener<OrderBook<T>>
{
public:

	// ctor
	AlgoExecutionToMarketDataListener(AlgoExecutionService<T>* _service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(OrderBook<T>& _data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(OrderBook<T>& _data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(OrderBook<T>& _data);
private:
	AlgoExecutionService<T>* service;
};

template<typename T>
AlgoExecutionToMarketDataListener<T>::AlgoExecutionToMarketDataListener(AlgoExecutionService<T>* _service)
{
	service = _service;
}


template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessAdd(OrderBook<T>& _data)
{
	// request the order execution
	service->AlgoOrderExecution(_data);
}

// do nothing for these methods (not required)
template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessRemove(OrderBook<T>& _data) {}

template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessUpdate(OrderBook<T>& _data) {}

#endif //!ALGO_EXECUTION_SERVICE_HPP
