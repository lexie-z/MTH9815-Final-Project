/**
 * algostreamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */

#ifndef ALGO_STREAMING_SERVICE_HPP
#define ALGO_STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "utilities.hpp"
#include "marketdataservice.hpp"
#include "pricingservice.hpp"

/**
 * A price stream order featuring price and visible and hidden quantity.
 */
class PriceStreamOrder
{

public:

    // Ctor
    PriceStreamOrder() = default;
    PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side){
        price = _price;
        visibleQuantity = _visibleQuantity;
        hiddenQuantity = _hiddenQuantity;
        side = _side;
    }

    // Get the side on the order
    PricingSide GetSide() const{
        return side;
    }

    // Get the price on this order
    double GetPrice() const{
        return price;
    }

    // Get the visible quantity on this order
    long GetVisibleQuantity() const{
        return visibleQuantity;
    }

    // Get the hidden quantity on this order
    long GetHiddenQuantity() const{
        return hiddenQuantity;
    }

    // Store attributes as strings
    vector<string> ToStrings() const {
        vector<string> orderDetails;
        orderDetails.push_back(PriceToString(price));
        orderDetails.push_back(std::to_string(visibleQuantity));
        orderDetails.push_back(std::to_string(hiddenQuantity));
        orderDetails.push_back(side == BID ? "BID" : "OFFER");

        return orderDetails;
    }

private:
    double price;
    long visibleQuantity;
    long hiddenQuantity;
    PricingSide side;

};


/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

    // ctor
    PriceStream() = default;
    PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder);

    // Get the product
    const T& GetProduct() const{
        return product;
    }

    // Get the bid order
    const PriceStreamOrder& GetBidOrder() const{
        return bidOrder;
    }

    // Get the offer order
    const PriceStreamOrder& GetOfferOrder() const{
        return offerOrder;
    }

    // Convert to strings
    vector<string> ToStrings() const{
        vector<string> streamDetails;

        streamDetails.emplace_back(product.GetProductId());

        for (const auto& bidDetail : bidOrder.ToStrings()) {
            streamDetails.push_back(bidDetail);
        }

        for (const auto& offerDetail : offerOrder.ToStrings()) {
            streamDetails.push_back(offerDetail);
        }

        return streamDetails;
    }

private:
    T product;
    PriceStreamOrder bidOrder;
    PriceStreamOrder offerOrder;

};

template<typename T>
PriceStream<T>::PriceStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder) :
        product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder){
}


/**
 * An algorithm stream processor for handling algorithmic streaming.
 * Maintains a price stream object.
 * Type T is the product type.
*/
template<typename T>
class AlgoStream
{

public:
    // ctor
    AlgoStream() = default;
    AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder){
        priceStream = new PriceStream<T>(_product, _bidOrder, _offerOrder);
    }


    // Get the price stream
    PriceStream<T>* GetPriceStream() const{
        return priceStream;
    }

private:
    PriceStream<T>* priceStream;
};

/**
* Pre-declearations
*/
template<typename T>
class AlgoStreamingToPricingListener;

/**
* Service for algo streaming orders on an exchange.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>>
{
public:

    // Ctor
    AlgoStreamingService(){
        algoStreams = map<string, AlgoStream<T>>();
        listeners = vector<ServiceListener<AlgoStream<T>>*>();
        listener = new AlgoStreamingToPricingListener<T>(this);
        pricePublishCount = 0;
    }
    ~AlgoStreamingService() {}

    // Get data by key
    AlgoStream<T>& GetData(string _key){
        return algoStreams[_key];
    }

    // Callback for any new or updated data
    void OnMessage(AlgoStream<T>& _data){
        string _id = _data.GetPriceStream()->GetProduct().GetProductId();
        algoStreams[_id] = _data;
    }

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<AlgoStream<T>>* _listener){
        listeners.push_back(_listener);
    }

    // Get all listeners
    const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const{
        return listeners;
    }

    // Get the listener of the service
    ServiceListener<Price<T>>* GetListener(){
        return listener;
    }

    // Publish two-way prices
    void AlgoPublishPrice(Price<T>& _price);

private:
    map<string, AlgoStream<T>> algoStreams;
    vector<ServiceListener<AlgoStream<T>>*> listeners;
    ServiceListener<Price<T>>* listener;
    long pricePublishCount;
};

/**
 * publish price updates
 * set alternating visible quantity 1M and 2M
 */
const long MILLION = 1000000;
const long HIDDEN_QUANTITY_MULTIPLIER = 2;

long calculateVisibleQuantity(int count) {
    return ((count % 2) + 1) * MILLION;
}

template<typename T>
void AlgoStreamingService<T>::AlgoPublishPrice(Price<T>& price) {
    T product = price.GetProduct();
    string productId = product.GetProductId();

    double midPrice = price.GetMid();
    double spread = price.GetBidOfferSpread();
    double bidPrice = midPrice - spread / 2.0;
    double offerPrice = midPrice + spread / 2.0;

    long visibleQuantity = calculateVisibleQuantity(pricePublishCount);
    long hiddenQuantity = visibleQuantity * HIDDEN_QUANTITY_MULTIPLIER;

    pricePublishCount++;
    PriceStreamOrder bidOrder(bidPrice, visibleQuantity, hiddenQuantity, BID);
    PriceStreamOrder offerOrder(offerPrice, visibleQuantity, hiddenQuantity, OFFER);
    AlgoStream<T> algoStream(product, bidOrder, offerOrder);
    algoStreams[productId] = algoStream;

    for (auto& listener : listeners) {
        listener->ProcessAdd(algoStream);
    }
}


/**
* Algo Streaming Service Listener subscribing data from Pricing Service to Algo Streaming Service.
* Type T is the product type.
*/
template<typename T>
class AlgoStreamingToPricingListener : public ServiceListener<Price<T>>
{

private:
    AlgoStreamingService<T>* service;

public:

    // ctor
    AlgoStreamingToPricingListener(AlgoStreamingService<T>* _service){
        service = _service;
    }

    // Process an add event to the Service
    void ProcessAdd(Price<T>& _data){
        service->AlgoPublishPrice(_data);
    }

    // Process a remove event to the Service
    void ProcessRemove(Price<T>& _data) {}

    // Process an update event to the Service
    void ProcessUpdate(Price<T>& _data) {}

};

#endif // !ALGO_STREAMING_SERVICE_HPP
