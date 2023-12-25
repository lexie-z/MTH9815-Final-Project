/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "marketdataservice.hpp"
#include "algostreamingservice.hpp"

/**
 * A price stream order with price and quantity (visible and hidden)
 */
/*
class PriceStreamOrder
{

public:

  // ctor for an order
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // The side on this order
  PricingSide GetSide() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity on this order
  long GetHiddenQuantity() const;

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;

};
*/
/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
 /*
template<typename T>
class PriceStream
{

public:

  // ctor
  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;

};
*/

/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class StreamingToAlgoStreamingListener;

/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{

private:

    map<string, PriceStream<T>> priceStreams;
    vector<ServiceListener<PriceStream<T>>*> listeners;
    ServiceListener<AlgoStream<T>>* listener;

public:
    // Ctor
    StreamingService() : priceStreams(), listeners(), listener(new StreamingToAlgoStreamingListener<T>(this)) {}

    // Get data by  key
    PriceStream<T>& GetData(string key) { return priceStreams[key]; }

    // Callback for new or updated data
    void OnMessage(PriceStream<T>& data) {
        string id = data.GetProduct().GetProductId();
        priceStreams[id] = data;
    }

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<PriceStream<T>>* _listener) { listeners.push_back(_listener); }

    // Get all listeners
    const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const { return listeners; }

    // Get the listener of the service
    ServiceListener<AlgoStream<T>>* GetListener() { return listener; }

    // Publish two-way prices
    void PublishPrice(PriceStream<T>& priceStream);
};

template<typename T>
void StreamingService<T>::PublishPrice(PriceStream<T>& _priceStream)
{
    for (auto& l : listeners)
    {
        l->ProcessAdd(_priceStream);
    }
}

/*
PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  side = _side;
}

double PriceStreamOrder::GetPrice() const
{
  return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
  return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
  product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
  return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
  return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
  return offerOrder;
}
*/

/**
* Streaming Service Listener reading data from Algo Streaming Service to Streaming Service.
*/

template<typename T>
class StreamingToAlgoStreamingListener : public ServiceListener<AlgoStream<T>>
{

private:
    StreamingService<T>* service;

public:

    // Connector and Destructor
    StreamingToAlgoStreamingListener(StreamingService<T>* _service) : service(_service) {}
    virtual ~StreamingToAlgoStreamingListener() {}

    // Listener callback to process an add event
    void ProcessAdd(AlgoStream<T>& data) {
        PriceStream<T>* priceStream = data.GetPriceStream();
        service->OnMessage(*priceStream);
        service->PublishPrice(*priceStream);
    }

    // Process a remove event to the Service
    void ProcessRemove(AlgoStream<T>& data){}

    // Process an update event
    void ProcessUpdate(AlgoStream<T>& data){}
};

#endif
