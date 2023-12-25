/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham & Lexie Zhu
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP

#include "soa.hpp"
#include "utilities.hpp"
#include "positionservice.hpp"
#include "riskservice.hpp"
#include "executionservice.hpp"
#include "streamingservice.hpp"
#include "inquiryservice.hpp"

enum ServiceType { POSITION, RISK, EXECUTION, STREAMING, INQUIRY };

/**
* Pre-declearations
*/
template<typename V>
class HistoricalDataConnector;
template<typename V>
class HistoricalDataListener;


/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type V is the data type to persist (for some services)
 */
template<typename V>
class HistoricalDataService : Service<string, V>
{

public:

    // Ctor
    //default set as inquiry
    HistoricalDataService(){
        historicalDatas = map<string, V>();
        listeners = vector<ServiceListener<V>*>();
        connector = new HistoricalDataConnector<V>(this);
        listener = new HistoricalDataListener<V>(this);
        type = INQUIRY;
    }
    HistoricalDataService(ServiceType _type){
        historicalDatas = map<string, V>();
        listeners = vector<ServiceListener<V>*>();
        connector = new HistoricalDataConnector<V>(this);
        listener = new HistoricalDataListener<V>(this);
        type = _type;
    }

    // Get data by key
    V& GetData(string _key){
        return historicalDatas[_key];
    }

    // Callback for any new or updated data
    void OnMessage(V& _data){
        historicalDatas[_data.GetProduct().GetProductId()] = _data;
    }

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<V>* _listener){
        listeners.push_back(_listener);
    }

    // Get all listeners on the Service
    const vector<ServiceListener<V>*>& GetListeners() const{
        return listeners;
    }

    // Get the connector of the service
    HistoricalDataConnector<V>* GetConnector(){
        return connector;
    }

    // Get the listener of the service
    ServiceListener<V>* GetServiceListener(){
        return listener;
    }

    // Get the service type that historical data comes from
    ServiceType GetServiceType() const{
        return type;
    }

    // Persist data to a store
    void PersistData(string _persistKey, V& _data){
        connector->Publish(_data);
    }

private:

    map<string, V> historicalDatas;
    vector<ServiceListener<V>*> listeners;
    HistoricalDataConnector<V>* connector;
    ServiceListener<V>* listener;
    ServiceType type;
};

/**
* Connector for Historical Data, responsible for disseminating information
* from the Historical Data Service.
* It handles data of type V for archiving purposes.
*/

template<typename V>
class HistoricalDataConnector : public Connector<V>
{

private:

    HistoricalDataService<V>* service;

public:

    // Ctor
    HistoricalDataConnector(HistoricalDataService<V>* _service){
        service = _service;
    }

    // Publish data to the Connector
    void Publish(V& _data);

    // Subscribe data from the Connector (not implemented)
    void Subscribe(ifstream& _data) {}

};

/** Publish is the core function.
 *
 * @tparam V
 * @param _data
 */
template<typename V>
void HistoricalDataConnector<V>::Publish(V& _data)
{
    ServiceType _type = service->GetServiceType();
    ofstream _file;
    if (_type == POSITION) {
        _file.open("positions.txt", ios::app);
    }
    if (_type == RISK) {
        _file.open("risk.txt", ios::app);
    }
    if (_type == EXECUTION) {
        _file.open("executions.txt", ios::app);
    }
    if (_type == STREAMING) {
        _file.open("streaming.txt", ios::app);
        //cout << "Start writing." << endl;
    }
    if (_type == INQUIRY) {
        _file.open("allinquiries.txt", ios::app);
    }

    _file << GetTimeStamp() << ",";

    // Call ToStrings() to write data into files.
    vector<string> _dataStrings = _data.ToStrings();
    for (auto& s : _dataStrings)
    {
        _file << s << ",";
    }
    _file << endl;
}

/**
* Listener for Historical Data Service, dedicated to subscribing and
* processing data for historical records.
* Operates on data of type V for archival operations.
* Utilizes persist key to identify and manage relevant data updates.
*/

template<typename V>
class HistoricalDataListener : public ServiceListener<V>
{

private:

    HistoricalDataService<V>* service;

public:

    // Connector and Destructor
    HistoricalDataListener(HistoricalDataService<V>* _service){
        service = _service;
    }

    ~HistoricalDataListener() {}

    // Listener callback to process an add event to the Service
    void ProcessAdd(V& _data){
        string _persistKey = _data.GetProduct().GetProductId();
        service->PersistData(_persistKey, _data);
    }

    // Process a remove event to the Service (not implemented)
    void ProcessRemove(V& _data) {}

    // Process an update event to the Service (not implemented)
    void ProcessUpdate(V& _data) {}

};

#endif
