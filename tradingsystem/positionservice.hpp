/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

    // ctor for a position
    Position() = default;
    Position(const T& _product);

    // Get the product
    const T& GetProduct() const;

    // Get the position quantity
    long GetPosition(string& book);

    // Get the positions over books
    map<string, long> GetPositions();

    // Set the position quantity
    void AddPosition(string& _book, long _position);

    // Get the aggregate position
    long GetAggregatePosition();

    // Save attributes as strings
    vector<string> ToStrings() const;

private:

    T product;
    map<string, long> positions;

};

template<typename T>
Position<T>::Position(const T& _product) :
        product(_product) {}

template<typename T>
const T& Position<T>::GetProduct() const
{
    return product;
}

template<typename T>
long Position<T>::GetPosition(string& book)
{
    return positions[book];
}

template<typename T>
map<string, long> Position<T>::GetPositions()
{
    return positions;
}

template<typename T>
void Position<T>::AddPosition(string& _book, long _position)
{
    positions[_book] += _position;
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
    long aggregatePosition = 0;
    for (auto& p : positions) {
        aggregatePosition += p.second;
    }
    return aggregatePosition;
}

template<typename T>
vector<string> Position<T>::ToStrings() const
{
    string _product = product.GetProductId();
    vector<string> _positions;

    // storing the market and corresponding positions
    for (auto& p : positions)
    {
        string _book = p.first;
        string _position = to_string(p.second);
        _positions.push_back(_book);
        _positions.push_back(_position);
    }

    vector<string> _strings;
    _strings.push_back(_product);
    _strings.insert(_strings.end(), _positions.begin(), _positions.end());
    return _strings;
}


/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class PositionToTradeBookingListener;

/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string, Position <T> >
{

public:
    // Ctor
    PositionService(){
        positions = map<string, Position<T>>();
        listeners = vector<ServiceListener<Position<T>>*>();
        listener = new PositionToTradeBookingListener<T>(this);
    };

    // Get data on our service given a key
    Position<T>& GetData(string _key){
        return positions[_key];
    };

    // Callback for any new or updated data
    void OnMessage(Position<T>& _data){
        string _id = _data.GetProduct().GetProductId();
        positions[_id] = _data;
    };

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Position<T>>* _listener){
        listeners.push_back(_listener);
    };

    // Get all listeners
    const vector<ServiceListener<Position<T>>*>& GetListeners() const{
        return listeners;
    };

    // Get the listener of the service
    PositionToTradeBookingListener<T>* GetListener(){
        return listener;
    };

    // Add a trade to the service
    virtual void AddTrade(const Trade<T>& _trade);

private:
    map<string, Position<T>> positions;
    vector<ServiceListener<Position<T>>*> listeners;
    PositionToTradeBookingListener<T>* listener;
};


// add a trade into the system
template<typename T>
void PositionService<T>::AddTrade(const Trade<T>& _trade)
{
    T _product = _trade.GetProduct();
    string _productId = _product.GetProductId();
    double _price = _trade.GetPrice();
    string _book = _trade.GetBook();
    long _quantity = _trade.GetQuantity();
    Side _side = _trade.GetSide();
    Position<T> _positionTo(_product);

    if (_side == BUY) {
        _positionTo.AddPosition(_book, _quantity);
    }
    else
    {
        _positionTo.AddPosition(_book, -_quantity);
    }

    // update the book
    Position<T> _positionFrom = positions[_productId];
    map <string, long> _positionMap = _positionFrom.GetPositions();
    for (auto& p : _positionMap)
    {
        _book = p.first;
        _quantity = p.second;
        _positionTo.AddPosition(_book, _quantity);
    }
    positions[_productId] = _positionTo;

    // add back into the system.
    for (auto& l : listeners)
    {
        l->ProcessAdd(_positionTo);
    }
}

/**
* Position Service Listener subscribing data from trading_booking_service to position_service.
*/
template<typename T>
class PositionToTradeBookingListener : public ServiceListener<Trade<T>>
{

private:

    PositionService<T>* service;

public:

    // ctor
    PositionToTradeBookingListener(PositionService<T>* _service){
        service = _service;
    };

    // Process an add event to the Service
    void ProcessAdd(Trade<T>& _data){
        service->AddTrade(_data);
    };

    // Process a remove event to the Service
    void ProcessRemove(Trade<T>& _data) {};

    // Process an update event to the Service
    void ProcessUpdate(Trade<T>& _data) {};

};


#endif
