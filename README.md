# MTH9815_FinalProject

Author: Yiqing(Lexie) Zhu

# Description:
- Develop a bond trading system for US Treasuries with seven securities: 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y.
- Ids I have are: 91282CJL6, 91282CHY0, 91282CHX2, 91282CJM4, 91282CJJ1, 912810TM0, 912810TL2 for 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y respectively. I obtained the information online from https://treasurydirect.gov
- We have a new definition of a Service in soa.hpp, with the concept of a ServiceListener and Connector also defined. A ServiceListener is a listener to events on the service where data is added to the service, updated on the service, or removed from the service. A Connector is a class that flows data into the Service from some connectivity source (e.g. a socket, file, etc) via the Service.OnMessage() method. The Publish() method on the Connector publishes data to the connectivity source and can be invoked from a Service. Some Connectors are publish-only that do not invoke Service.OnMessage(). Some Connectors are subscribe-only where Publish() does nothing. Other Connectors can do both publish and subscribe.

# To run the codes:
- To compile it using g++, use g++ -std=c++17 main.cpp -o test -I /usr/local/Cellar/boost/1.83.0/include -L /usr/local/Cellar/boost/1.83.0/lib and then run ./test on macos. Remember to change the line for windows users and specify your boost path.


