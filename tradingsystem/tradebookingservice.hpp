/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "executionservice.hpp"
#include "soa.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

    // ctor for a trade
    Trade() = default;
    Trade(const T& _product, string _tradeId, double _price, string _book, long _quantity, Side _side);

    // Get the product
    const T& GetProduct() const;

    // Get the trade ID
    const string& GetTradeId() const;

    // Get the mid price
    double GetPrice() const;

    // Get the book
    const string& GetBook() const;

    // Get the quantity
    long GetQuantity() const;

    // Get the side
    Side GetSide() const;

private:
    T product;
    string tradeId;
    double price;
    string book;
    long quantity;
    Side side;

};

/**
* Pre-declearations
*/
template<typename T>
class TradeBookingConnector;
template<typename T>
class TradeBookingToExecutionListener;

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string, Trade <T> >
{

private:
    map<string, Trade<T>> trades;
    vector<ServiceListener<Trade<T>>*> listeners;
    TradeBookingConnector<T>* connector;
    TradeBookingToExecutionListener<T>* listener;

public:

    //Ctor
    TradeBookingService()
    {
        trades = map<string, Trade<T>>();
        listeners = vector<ServiceListener<Trade<T>>*>();
        connector = new TradeBookingConnector<T>(this);
        listener = new TradeBookingToExecutionListener<T>(this);
    }

    // Get data by key
    Trade<T>& GetData(string _key){
        return trades[_key];
    };

    // Callback for any new or updated data
    void OnMessage(Trade<T>& _data){
        string _id = _data.GetTradeId();
        trades[_id] = _data;

        for (auto& l : listeners)
        {
            l->ProcessAdd(_data);
        }
    };

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Trade<T>>* _listener){
        listeners.push_back(_listener);
    };

    // Get all listeners on the Service
    const vector<ServiceListener<Trade<T>>*>& GetListeners() const{
        return listeners;
    };

    // Get the connector of the service
    TradeBookingConnector<T>* GetConnector(){
        return connector;
    };

    // Get the listener of the service
    TradeBookingToExecutionListener<T>* GetListener(){
        return listener;
    };

    // Book the trade
    void BookTrade(Trade<T>& _trade){
        for (auto& l : listeners)
        {
            l->ProcessAdd(_trade);
        }
    };
};

template<typename T>
Trade<T>::Trade(const T& _product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
        product(_product)
{
    tradeId = _tradeId;
    price = _price;
    book = _book;
    quantity = _quantity;
    side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
    return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
    return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
    return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
    return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
    return side;
}


/**
* Trade Booking Connector reading data to Trading Booking Service.
*/
template<typename T>
class TradeBookingConnector : public Connector<Trade<T>>
{

public:
    // Ctor
    TradeBookingConnector(TradeBookingService<T>* _service){
        service = _service;
    };

    // Publish data to the Connector
    void Publish(Trade<T>& _data){};

    // Subscribe data from the Connector
    void Subscribe(ifstream& _data);

private:
    TradeBookingService<T>* service;

};

// Read data from the txt file, build the trades and update into the system.
template<typename T>
void TradeBookingConnector<T>::Subscribe(ifstream& dataStream)
{
    for (string line; getline(dataStream, line); )
    {
        vector<string> cells = SplitLine(line);

        string productId = cells[0];
        string tradeId = cells[1];
        double price = ConvertStringToPrice(cells[2]);
        string book = cells[3];
        long quantity = stol(cells[4]);
        Side side = (cells[5] == "BUY") ? BUY : SELL;
        T product = RetrieveProduct(productId);

        Trade<T> trade(product, tradeId, price, book, quantity, side);
        service->OnMessage(trade);
    }
}

/**
* Trade Booking Service Listener reading data from Execution Service to Trading Booking Service.
*/
template<typename T>
class TradeBookingToExecutionListener : public ServiceListener<ExecutionOrder<T>>
{

public:

    // Ctor
    TradeBookingToExecutionListener(TradeBookingService<T>* _service){
        service = _service;
        tradeBookCount = 0;
    };

    // Listener callback to process an add event to the Service
    void ProcessAdd(ExecutionOrder<T>& _data);

    // Listener callback to process a remove event to the Service
    void ProcessRemove(ExecutionOrder<T>& _data) {};

    // Listener callback to process an update event to the Service
    void ProcessUpdate(ExecutionOrder<T>& _data) {};

private:

    TradeBookingService<T>* service;
    long tradeBookCount;

};

template<typename T>
void TradeBookingToExecutionListener<T>::ProcessAdd(ExecutionOrder<T>& executionData)
{
    static const std::vector<std::string> marketVector = {"TRSY1", "TRSY2", "TRSY3"};
    tradeBookCount++;

    T product = executionData.GetProduct();
    PricingSide pricingSide = executionData.GetPricingSide();
    std::string orderId = executionData.GetOrderId();
    double price = executionData.GetPrice();
    long visibleQuantity = executionData.GetVisibleQuantity();
    long hiddenQuantity = executionData.GetHiddenQuantity();

    Side tradeSide = (pricingSide == BID) ? SELL : BUY;

    std::string book = marketVector[tradeBookCount % marketVector.size()];
    long totalQuantity = visibleQuantity + hiddenQuantity;

    Trade<T> trade(product, orderId, price, book, totalQuantity, tradeSide);
    service->OnMessage(trade);
    service->BookTrade(trade);
}

#endif
