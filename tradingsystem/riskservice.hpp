/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"
#include "utilities.hpp"

/**
 * PV01 risk.
 * Type T is the product type.
 */
#include <string>
#include <vector>
using namespace std;

template<typename T>
class PV01
{

public:

    // ctor for a PV01 value
    PV01() = default;
    PV01(const T& _product, double _pv01, long _quantity) : product(_product), pv01(_pv01), quantity(_quantity) {}

    // Get the product on this PV01 value
    const T& GetProduct() const { return product; }

    // Get the PV01 value
    double GetPV01() const { return pv01; }

    // Get the quantity associated with this risk value
    long GetQuantity() const { return quantity; }

    // Set the quantity associated with this risk value
    void SetQuantity(long _q) { quantity = _q; }

    //Convert output to strings to store
    vector<string> ToStrings() const {
        return vector<string>{
                product.GetProductId(), //product
                to_string(pv01), //pv01 value
                to_string(quantity) //quantity
        };
    }

private:
    T product;
    double pv01;
    long quantity;
};


/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

    // ctor for a bucket sector
    BucketedSector(const vector<T>& _products, string _name) : products(_products), name(_name) {}

    // Get the products associated with this bucket
    const vector<T>& GetProducts() const { return products; }

    // Get the name of the bucket
    const string& GetName() const { return name; }

private:
    vector<T> products;
    string name;

};

/**
* Pre-declearations to avoid errors.
*/
template<typename T>
class RiskToPositionListener;		// the listener that connects risk and position service

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string, PV01 <T> >
{

public:

    // ctor
    RiskService() : pv01s(), listeners(), listener(new RiskToPositionListener<T>(this)) {}

    // Add a position
    void AddPosition(Position<T>& position);

    // Get the bucketed risk for the bucket sector
    const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T>& sector) const;

    // Get data by key
    PV01<T>& GetData(string key) { return pv01s[key]; }

    // The callback function upon receiving new data
    void OnMessage(PV01<T>& _data);

    // Add a listener to the Service
    void AddListener(ServiceListener<PV01<T>>* _listener) { listeners.push_back(_listener); }

    // Get all listeners
    const vector<ServiceListener<PV01<T>>*>& GetListeners() const { return listeners; }

    //Get the listener specific to the risk position
    RiskToPositionListener<T>* GetListener() { return listener; }

private:
    map<string, PV01<T>> pv01s;
    vector<ServiceListener<PV01<T>>*> listeners;
    RiskToPositionListener<T>* listener;
};

template<typename T>
void RiskService<T>::OnMessage(PV01<T>& _data)
{
    string id = _data.GetProduct().GetProductId();
    pv01s[id] = _data;
}

// add position, in connection with the Position class
template<typename T>
void RiskService<T>::AddPosition(Position<T>& _position)
{
    T _product = _position.GetProduct();
    string _id = _product.GetProductId();
    double _pv01Value = GetPV01(_id); //utility function
    long _quantity = _position.GetAggregatePosition();
    PV01<T> _pv01(_product, _pv01Value, _quantity);
    pv01s[_id] = _pv01;

    for (auto& l : listeners)
    {
        l->ProcessAdd(_pv01);
    }
}

template<typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& _sector) const
{
    BucketedSector<T> _product = _sector;
    double _pv01 = 0.0;
    long _quantity = 1;

    vector<T>& _products = _sector.GetProducts();

    for (auto& p : _products)
    {
        string _pId = p.GetProductId();
        long _q = pv01s[_pId].GetQuantity();
        double _val = pv01s[_pId].GetPV01();
        _pv01 += _val * (double)_q;
    }

    return PV01<BucketedSector<T>>(_product, _pv01, _quantity);
}

/**
* Risk Service Listener reading data from Position Service to Risk Service.
*/
template<typename T>
class RiskToPositionListener : public ServiceListener<Position<T>>
{
private:
    RiskService<T>* service;

public:

    // Ctor
    RiskToPositionListener(RiskService<T>* _service) : service(_service) {}

    // Listener callback to process an add event to the Service
    void ProcessAdd(Position<T>& data) { service->AddPosition(data); }

    // ProcessRemove and ProcessUpdate do nothing
    void ProcessRemove(Position<T>& data) {}
    void ProcessUpdate(Position<T>& data) {}

};

#endif
