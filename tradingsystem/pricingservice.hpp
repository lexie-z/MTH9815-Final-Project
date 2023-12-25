/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include "utilities.hpp"
#include <string>
#include "soa.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:
    // ctor for a price
    Price() = default;
    Price(const T & _product, double _mid, double _bidOfferSpread);

    // Get the product
    const T& GetProduct() const;

    // Get the mid price
    double GetMid() const;

    // Get the bid/offer spread around the mid
    double GetBidOfferSpread() const;

    // Change attributes to strings
    vector<string> ToStrings() const;
private:

    T product;
    double mid;
    double bidOfferSpread;

};

template<typename T>
Price<T>::Price(const T& _product, double _mid, double _bidOfferSpread) :
        product(_product)
{
    mid = _mid;
    bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
    return product;
}

template<typename T>
double Price<T>::GetMid() const
{
    return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
    return bidOfferSpread;
}

template<typename T>
vector<string> Price<T>::ToStrings() const
{
    string _product = product.GetProductId();
    string _mid = PriceToString(mid);
    string _bidOfferSpread = PriceToString(bidOfferSpread);

    vector<string> _strings;
    _strings.push_back(_product);
    _strings.push_back(_mid);
    _strings.push_back(_bidOfferSpread);
    return _strings;
}

/**
* Pre-declearations
*/
template<typename T>
class PricingConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string, Price <T> >
{
private:
    map<string, Price<T>> prices;
    vector<ServiceListener<Price<T>>*> listeners;
    PricingConnector<T>* connector;
public:
    //Ctor and Dtor
    PricingService()
    {
        prices = map<string, Price<T>>();
        listeners = vector<ServiceListener<Price<T>>*>();
        connector = new PricingConnector<T>(this);
    }
    ~PricingService() = default;

    //Get the price of a product_id
    Price<T>& GetData(string _key){
        return prices[_key];
    };

    // Callback for any new or updated data
    void OnMessage(Price<T>& _data){
        prices[_data.GetProduct().GetProductId()] = _data;

        for (auto& listener : listeners) {
            listener->ProcessAdd(_data);
        }
    };

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Price<T>>* _listener){
        listeners.push_back(_listener);
    };

    //Get all the listeners
    const vector<ServiceListener<Price<T>>*>& GetListeners() const{
        return listeners;
    };

    //Get the connector
    PricingConnector<T>* GetConnector(){
        return connector;
    };
};

/**
 * The pricing connector is responsible for subscribing to the pricing service.
 * Upon subscription, it retrieves data from the specified path.
 * The retrieved data is then utilized in calculations, as outlined in data.txt.
 */

template<typename T>
class PricingConnector : public Connector<Price<T>>
{
public:
    // Ctor
    PricingConnector(PricingService<T>* _service){
        service = _service;
    };

    // Publish data to the Connector
    void Publish(Price<T>& _data) {};

    // Subscribe data from the Connector
    void Subscribe(ifstream& _data);

private:
    PricingService<T>* service;
};

// Core function: reading from "data.txt"
// and process the data.
template<typename T>
void PricingConnector<T>::Subscribe(ifstream& _data)
{
    string line;
    while (getline(_data, line))
    {
        auto cells = SplitLine(line);

        // fetch the corresponding data features
        string _productId = cells[0];
        double bid_price = ConvertStringToPrice(cells[1]);
        double offer_price = ConvertStringToPrice(cells[2]);
        double mid_price = (bid_price + offer_price) / 2.0;
        double spread = offer_price - bid_price;
        T _product = RetrieveProduct(_productId);

        Price<T> _price(_product, mid_price, spread);

        // update the generated price Data to the service.
        service->OnMessage(_price);
    }
}


#endif
