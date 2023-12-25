/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"
#include "utilities.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

    // ctor for an inquiry
    Inquiry() = default;
    Inquiry(string _inquiryId, const T& _product, Side _side, long _quantity, double _price, InquiryState _state);

    // Get the inquiry ID
    const string& GetInquiryId() const;

    // Get the product
    const T& GetProduct() const;

    // Get the side on the inquiry
    Side GetSide() const;

    // Get the quantity that the client is inquiring for
    long GetQuantity() const;

    // Get the price that we have responded back with
    double GetPrice() const;

    // Get the current state on the inquiry
    InquiryState GetState() const;

    // Set the price that we have responded back with
    void SetPrice(double _price);

    // Set the current state on the inquiry
    void SetState(InquiryState _state);

    // Store attributes as strings
    vector<string> ToStrings() const;

private:
    string inquiryId;
    T product;
    Side side;
    long quantity;
    double price;
    InquiryState state;

};

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T& _product, Side _side, long _quantity, double _price, InquiryState _state) :
        product(_product)
{
    inquiryId = _inquiryId;
    side = _side;
    quantity = _quantity;
    price = _price;
    state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
    return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
    return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
    return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
    return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
    return state;
}

template<typename T>
void Inquiry<T>::SetPrice(double _price)
{
    price = _price;
}

template<typename T>
void Inquiry<T>::SetState(InquiryState _state)
{
    state = _state;
}

template<typename T>
vector<string> Inquiry<T>::ToStrings() const
{
    vector<string> inquiryDetails;

    inquiryDetails.push_back(inquiryId);
    inquiryDetails.push_back(product.GetProductId());

    string sideString = (side == BUY) ? "BUY" : "SELL";
    inquiryDetails.push_back(sideString);

    inquiryDetails.push_back(to_string(quantity));
    inquiryDetails.push_back(PriceToString(price));

    string stateString;
    switch (state) {
        case RECEIVED: stateString = "RECEIVED"; break;
        case QUOTED: stateString = "QUOTED"; break;
        case DONE: stateString = "DONE"; break;
        case REJECTED: stateString = "REJECTED"; break;
        case CUSTOMER_REJECTED: stateString = "CUSTOMER_REJECTED"; break;
    }
    inquiryDetails.push_back(stateString);

    return inquiryDetails;
}

/**
* Pre-declearations.
*/
template<typename T>
class InquiryConnector;

/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string, Inquiry <T> >
{
private:

    map<string, Inquiry<T>> inquiries;
    vector<ServiceListener<Inquiry<T>>*> listeners;
    InquiryConnector<T>* connector;

public:

    // Ctor
    InquiryService(){
        inquiries = map<string, Inquiry<T>>();
        listeners = vector<ServiceListener<Inquiry<T>>*>();
        connector = new InquiryConnector<T>(this);
    }

    // Get data by key
    Inquiry<T>& GetData(string _key){
        return inquiries[_key];
    }

    // Callback for any new or updated data
    void OnMessage(Inquiry<T>& _data);

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Inquiry<T>>* _listener){
        listeners.push_back(_listener);
    }

    // Get all listeners
    const vector<ServiceListener<Inquiry<T>>*>& GetListeners() const{
        return listeners;
    }

    // Get the connector of the service
    InquiryConnector<T>* GetConnector(){
        return connector;
    }

    // Issue a price quote in response to a client's inquiry
    void SendQuote(const string& inquiryId, double price){
        Inquiry<T>& inquiry = inquiries[inquiryId];

        // Set the new price for the inquiry and notify all listeners
        inquiry.SetPrice(price);
        for (auto& listener : listeners) {
            listener->ProcessAdd(inquiry);
        }
    }

    // Reject an inquiry from the client
    void RejectInquiry(const string& _inquiryId){
        Inquiry<T>& _inquiry = inquiries[_inquiryId];

        // set the state as rejected.
        _inquiry.SetState(REJECTED);
    }
};

/** Handles inquiries in RECEIVED and QUOTED states
* and provides updates to the registered listeners
*/
template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T>& inquiry)
{
    InquiryState currentState = inquiry.GetState();

    // Handling inquiry in the RECEIVED state
    if (currentState == RECEIVED) {
        inquiries[inquiry.GetInquiryId()] = inquiry;
        connector->Publish(inquiry);
    }

    // Transitioning from QUOTED to DONE state
    if (currentState == QUOTED) {
        inquiry.SetState(DONE);
        inquiries[inquiry.GetInquiryId()] = inquiry;

        // Notifying all listeners about the inquiry update
        for (auto& listener : listeners) {
            listener->ProcessAdd(inquiry);
        }
    }
}

/**
* Inquiry Connector subscribing or publishing data from inquiry service and to inquiry service.
*/

template<typename T>
class InquiryConnector : public Connector<Inquiry<T>>
{

private:

    InquiryService<T>* service;

public:

    // Ctor
    InquiryConnector(InquiryService<T>* _service){
        service = _service;
    }

    // Publish data to the Connector
    void Publish(Inquiry<T>& _data){
        if (_data.GetState() == RECEIVED)
        {
            _data.SetState(QUOTED);
            this->Subscribe(_data);
        }
    }

    // Subscribe data from the Connector
    void Subscribe(ifstream& _data);

    // After updating status, Re-subscribe data from the Connector
    void Subscribe(Inquiry<T>& _data){
        service->OnMessage(_data);
    }

};

// core function here
// read the inquiries.
template<typename T>
void InquiryConnector<T>::Subscribe(ifstream& _data)
{
    string _line;
    while (getline(_data, _line))
    {
        auto _cells = SplitLine(_line);

        string _inquiryId = _cells[0];
        string _productId = _cells[1];
        Side _side = _cells[2] == "BUY" ? BUY : SELL;
        long _quantity = stol(_cells[3]);
        double _price = ConvertStringToPrice(_cells[4]);
        InquiryState _state;
        if (_cells[5] == "RECEIVED"){
            _state = RECEIVED;
        }
        else if (_cells[5] == "QUOTED") {
            _state = QUOTED;
        }
        else if (_cells[5] == "DONE") {
            _state = DONE;
        }
        else if (_cells[5] == "REJECTED") {
            _state = REJECTED;
        }
        else if (_cells[5] == "CUSTOMER_REJECTED") {
            _state = CUSTOMER_REJECTED;
        }

        T _product = RetrieveProduct(_productId);
        Inquiry<T> _inquiry(_inquiryId, _product, _side, _quantity, _price, _state);
        service->OnMessage(_inquiry);
    }
}

#endif
