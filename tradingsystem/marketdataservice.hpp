/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include "soa.hpp"
#include "utilities.hpp"


using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

    // ctor for an order
    Order() = default;
    Order(double _price, long _quantity, PricingSide _side);

    // Get the price on the order
    double GetPrice() const;

    // Get the quantity on the order
    long GetQuantity() const;

    // Get the side on the order
    PricingSide GetSide() const;

private:
    double price;
    long quantity;
    PricingSide side;

};

Order::Order(double _price, long _quantity, PricingSide _side)
{
    price = _price;
    quantity = _quantity;
    side = _side;
}

double Order::GetPrice() const
{
    return price;
}

long Order::GetQuantity() const
{
    return quantity;
}

PricingSide Order::GetSide() const
{
    return side;
}

/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

    // ctor for bid/offer
    BidOffer(const Order& _bidOrder, const Order& _offerOrder);

    // Get the bid order
    const Order& GetBidOrder() const;

    // Get the offer order
    const Order& GetOfferOrder() const;

private:
    Order bidOrder;
    Order offerOrder;

};

BidOffer::BidOffer(const Order& _bidOrder, const Order& _offerOrder) :
        bidOrder(_bidOrder), offerOrder(_offerOrder) {}

const Order& BidOffer::GetBidOrder() const
{
    return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
    return offerOrder;
}

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

    // ctor for the order book
    OrderBook()=default;
    OrderBook(const T& _product, const vector<Order>& _bidStack, const vector<Order>& _offerStack);

    // Get the product
    const T& GetProduct() const{
        return product;
    }

    // Get the bid stack
    const vector<Order>& GetBidStack() const{
        return bidStack;
    }

    // Get the offer stack
    const vector<Order>& GetOfferStack() const{
        return offerStack;
    }

    // Get the best bid/offer order
    const BidOffer GetBidOffer() const;

private:
    T product;
    vector<Order> bidStack;
    vector<Order> offerStack;
};

template<typename T>
OrderBook<T>::OrderBook(const T& _product, const vector<Order>& _bidStack, const vector<Order>& _offerStack) :
        product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

//get the best bid and offer
template<typename T>
const BidOffer OrderBook<T>::GetBidOffer() const {
    // Get the highest bid
    Order top_bid = bidStack.front();
    double top_bid_price = top_bid.GetPrice();
    for (const auto& bid : bidStack) {
        double current_bid_price = bid.GetPrice();
        if (current_bid_price > top_bid_price) {
            top_bid = bid;
            top_bid_price = current_bid_price;
        }
    }

    // Get the lowest offer
    Order bottom_offer = offerStack.front();
    double bottom_offer_price = bottom_offer.GetPrice();
    for (const auto& offer : offerStack) {
        double current_offer_price = offer.GetPrice();
        if (current_offer_price < bottom_offer_price) {
            bottom_offer = offer;
            bottom_offer_price = current_offer_price;
        }
    }

    return BidOffer(top_bid, bottom_offer);
}


/**
 * predeclaration
 */
template <typename T>
class MarketDataConnector;

/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string, OrderBook <T> >
{

private:
    map<string, OrderBook<T>> orderBooks;
    vector<ServiceListener<OrderBook<T>>*> listeners;
    MarketDataConnector<T>* connector;
    int bookDepth;

public:
    // ctor
    MarketDataService(){
        orderBooks = map<string, OrderBook<T>>();
        listeners = vector<ServiceListener<OrderBook<T>>*>();
        connector = new MarketDataConnector<T>(this);
        bookDepth = 10;
    }

    ~MarketDataService(){
        delete connector;
    }

    // get order book by product id
    OrderBook<T>& GetData(string _key){
        return orderBooks[_key];
    }

    // call back function for the connector
    void OnMessage(OrderBook<T>& _data) {
        string product_id = _data.GetProduct().GetProductId();
        orderBooks[product_id] = _data;

        for (auto& listener : listeners) {
            listener->ProcessAdd(_data);
        }
    }

    // add listener to the service
    void AddListener(ServiceListener<OrderBook<T>>* listener){
        listeners.push_back(listener);
    }

    // fetch the active listeners on the service
    const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const{
        return listeners;
    }

    // fetch the connector
    MarketDataConnector<T>* GetConnector(){
        return connector;
    }

    // fetch the current orderbook depth
    int GetOrderBookDepth() const{
        return bookDepth;
    }

    // Get the best bid/offer order
    const BidOffer GetBestBidOffer(const string& _id){
        return orderBooks[_id].GetBidOffer();
    }

    // Aggregate the order book
    const OrderBook<T>& AggregateDepth(const string& _id);
};

// Aggregate the order book
template<typename T>
const OrderBook<T>& MarketDataService<T>::AggregateDepth(const string& instrumentId) {
    T& product = orderBooks[instrumentId].GetProduct();

    auto& bids = orderBooks[instrumentId].GetBidStack();
    auto& offers = orderBooks[instrumentId].GetOfferStack();

    unordered_map<double, long> aggregatedBids;
    unordered_map<double, long> aggregatedOffers;

    for (const Order& bidOrder : bids) {
        double price = bidOrder.GetPrice();
        long quantity = bidOrder.GetQuantity();
        aggregatedBids[price] += quantity;
    }
    vector<Order> consolidatedBids;
    for (const auto& bid : aggregatedBids) {
        consolidatedBids.emplace_back(bid.first, bid.second, BID);
    }

    for (const Order& offerOrder : offers) {
        double price = offerOrder.GetPrice();
        long quantity = offerOrder.GetQuantity();
        aggregatedOffers[price] += quantity;
    }
    vector<Order> consolidatedOffers;
    for (const auto& offer : aggregatedOffers) {
        consolidatedOffers.emplace_back(offer.first, offer.second, OFFER);
    }

    return OrderBook<T>(product, consolidatedBids, consolidatedOffers);
}


/**
 * MarketDataConnector, update connectors
* When subscribe, we receive the orders from data
 */

template <typename T>
class MarketDataConnector : public Connector<OrderBook<T>> {

private:
    MarketDataService<T>* service;

public:

    // ctor
    MarketDataConnector(MarketDataService<T>* _service){
        service = _service;
    }

    // Publish data to the Connector
    void Publish(OrderBook<T>& _data){}

    // Subscribe Ddata from the Connector
    void Subscribe(ifstream& data);
};

template<typename T>
void MarketDataConnector<T>::Subscribe(ifstream& marketDataStream)
{
    // Processing data
    int depthOfBook = service->GetOrderBookDepth();
    int processThreshold = depthOfBook * 2;
    long totalOrdersProcessed = 0;

    vector<Order> bids;
    vector<Order> offers;
    string record;

    while (getline(marketDataStream, record))
    {
        vector<string> tokens = SplitLine(record);
        string productId = tokens[0];
        double price = ConvertStringToPrice(tokens[1]);
        long quantity = stol(tokens[2]);

        PricingSide side = tokens[3] == "BID" ? BID : OFFER;
        Order newOrder(price, quantity, side);

        if (side == BID) bids.push_back(newOrder);
        else offers.push_back(newOrder);

        totalOrdersProcessed++;

        // Trigger updates at specific intervals
        if (totalOrdersProcessed % processThreshold == 0)
        {
            T product = RetrieveProduct(productId);
            OrderBook<T> currentOrderBook(product, bids, offers);
            service->OnMessage(currentOrderBook);

            // Clearing the order stacks
            bids.clear();
            offers.clear();
        }
    }
}

#endif