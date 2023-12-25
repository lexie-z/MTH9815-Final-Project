/**
 * GUIservice.hpp
 * Defines  data types and different services for GUI-related information
 * @author Breman Thuraisingham & Lexie Zhu
*/

#ifndef GUIservice_hpp
#define GUIservice_hpp

#include "soa.hpp"
#include "pricingservice.hpp"
#include "utilities.hpp"

/**
 * Pre-declearations
 */
template<typename T>
class GUIConnector;
template<typename T>
class GUIToPricingListener;

/**
 * GUI service
 * T is the product type
*/

template <typename T>
class GUIService : Service<string, Price<T>> {

private:
    map<string, Price<T>> GUIs;
    vector<ServiceListener<Price<T>>*>listeners;
    GUIConnector<T>* connector;
    ServiceListener<Price<T>>* listener;

    int accelerator;
    int time;

public:

    //Ctor and Dtor
    GUIService(){
        GUIs = map<string, Price<T>>();
        listeners = vector<ServiceListener<Price<T>>*>();
        connector = new GUIConnector<T>(this);
        listener = new GUIToPricingListener<T>(this);
        accelerator = 300;
        time = 0;
    }

    ~GUIService() {}

    // Get data by product id
    Price<T>& GetData(string _key){
        return GUIs[_key];
    }

    // call back function for the connector
    void OnMessage(Price<T>& _data){
        // publish the data
        string product_id = _data.GetProduct().GetProductId();
        GUIs[product_id] = _data;
        connector->Publish(_data);
    }

    // add listener to the service
    void AddListener(ServiceListener<Price<T>>* listener){
        listeners.push_back(listener);
    }

    // fetch the active listeners on the service
    const vector<ServiceListener<Price<T>>*>& GetListeners() const{
        return listeners;
    }

    // fetch the connector
    GUIConnector<T>* GetConnector(){
        return connector;
    }

    // fetch the listener
    ServiceListener<Price<T>>* GetListener(){
        return listener;
    }

    int GetAccelerator() const
    {
        return accelerator;
    }

    int GetTime() const
    {
        return time;
    }

    void SetAccelerator(int acc)
    {
        accelerator = acc;
    }

    void SetTime(int t)
    {
        time = t;
    }
};


template<typename T>
class GUIConnector :public Connector<Price<T>> {
public:
    GUIConnector(GUIService<T>* _service){
        service = _service;
    };

    // interactions with connector
    // publish the data, save the records
    void Publish(Price<T>& _data);

    // subscribe data from connector (not needed)
    void Subscribe(ifstream& _data) {};

private:
    GUIService<T>* service;
};

template<typename T>
void GUIConnector<T>::Publish(Price<T>& data){
    int accelerator = service->GetAccelerator();
    int time = service->GetTime();
    int currentTime = GetTime();

    // update time
    while (currentTime < time) {
        currentTime += 1000;
    }
    if (currentTime - time >= accelerator)
    {
        service->SetTime(currentTime);
        ofstream _file;
        _file.open("gui.txt", ios::app);

        // update the information
        _file << GetTimeStamp() << ",";
        for (auto& s : data.ToStrings())
        {
            _file << s << ",";
        }
        _file << "\n";
    }
}

/**
 * GUI Service listener on the BondPricingService class
* Update the information upon receiving messages
*/

template<typename T>
class GUIToPricingListener : public ServiceListener<Price<T>> {
private:
    GUIService<T>* service;

public:
    GUIToPricingListener(GUIService<T>* _service){
        service = _service;
    }

    // Process an add event to the Service
    void ProcessAdd(Price<T>& _data){
        service->OnMessage(_data);
    }

    // Process a remove event to the Service
    void ProcessRemove(Price<T>& _data) {}

    // Process an update event to the Service
    void ProcessUpdate(Price<T>& _data) {}
};

#endif
