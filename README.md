# MTH9815_FinalProject

Author: Yiqing(Lexie) Zhu


# Project Structure Overview
- Framework for Bond Services: We start by establishing template classes in the product.hpp file. These templates are designed to work with a broad range of products, but within main.cpp, we explicitly define our focus on Bonds. This approach ensures a flexible and adaptable system, easily extendable to various products.
- Data Generation Approach: The data for this project is synthesized following specific rules laid out in the project documentation. The primary work occurs within datageneration.hpp, where initializing the system in the main file triggers data creation. For testing purposes, we generate 10,000 instead of 1,000,000 prices for each security due to the size and save it in the SampleData folder.
- Between Sevices Interactions: To facilitate interaction between different services, we employ a listener-based architecture as suggested. An example of this is the AlgoExecutionToExecutionListener, which forms a bridge between the AlgoExecution and Execution services. Similar patterns are observable in other service connections, playing a crucial role in the overall design.
- Data Handling and Utilization: We implement a Publish-Subscribe system using connectors described in the project specifications.
- Code Adaptation and Enhancements: Our codebase builds upon the foundational code provided by the Professor, with some elective modifications for enhanced functionality.

# Description of the Project:
- Develop a bond trading system for US Treasuries with seven securities: 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y.
- Ids I have are: 91282CJL6, 91282CHY0, 91282CHX2, 91282CJM4, 91282CJJ1, 912810TM0, 912810TL2 for 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y respectively. I obtained the information online from https://treasurydirect.gov
- We have a new definition of a Service in soa.hpp, with the concept of a ServiceListener and Connector also defined. A ServiceListener is a listener to events on the service where data is added to the service, updated on the service, or removed from the service. A Connector is a class that flows data into the Service from some connectivity source (e.g. a socket, file, etc) via the Service.OnMessage() method. The Publish() method on the Connector publishes data to the connectivity source and can be invoked from a Service. Some Connectors are publish-only that do not invoke Service.OnMessage(). Some Connectors are subscribe-only where Publish() does nothing. Other Connectors can do both publish and subscribe.

# Instructions to run the codes:
- To compile it using g++, use g++ -std=c++17 main.cpp -o test -I /usr/local/Cellar/boost/1.83.0/include -L /usr/local/Cellar/boost/1.83.0/lib and then run ./test on macos. Remember to change the line for windows users and specify your boost path.

# File Overview:
The system's architecture revolves around services keyed to the product ID, encompassing various components:

## Algorithm Execution Service (algoexecutionservice.hpp):
Models order execution with classes for ExecutionOrder, AlgoExecution, and service-level functionalities.
Includes listener design, like AlgoExecutionToMarketDataListener, linking to the MarketDataService.
## Algorithm Streaming Service (algostreamingservice.hpp):
Handles order stream modeling with classes like PriceStreamOrder and PriceStream.
Incorporates the AlgoStreamingService and a listener for connection to the PricingService.
## Data Generation Module (datageneration.hpp):
Functional programming approach for generating data across different bond-related datasets.
## Execution Service (executionservice.hpp):
Models the order execution process with a listener for AlgoExecutionService integration.
## GUI Service (GUIservice.hpp):
Manages price streaming with a throttle mechanism and connects to PricingService through a listener.
## Historical Data Service (historicaldataservice.hpp):
Connects various services, storing information from multiple sources into designated .txt files.
## Inquiry Service (inquiryservice.hpp):
Processes incoming inquiries and updates the system with new data through a connector.
## Market Data Service (marketdataservice.hpp):
Manages market data and order books, updating the system with new information through a connector.
## Position Service (positionservice.hpp):
Handles position management across multiple books and securities, with a listener for TradeBookingService integration.
## Pricing Service (pricingeservice.hpp):
Manages product pricing and updates the system through a connector.
## Product Base Class (product.hpp):
The foundational class for modeling different products, with a focus on bonds.
## Risk Service (riskservice.hpp):
Manages risk assessment with a listener for PositionService integration.
## Service Oriented Architecture Base Class (soa.hpp):
The core class for all services, defining essential components like ServiceListener and Connector.
## Streaming Service (streamingservice.hpp):
Manages streaming services and integrates with AlgoStreamingService through a listener.
## Trade Booking Service (tradingbookservice.hpp):
Handles trade booking and updates the system with new trade data through a connector.
## Utility Functions (utilityfunctions.hpp):
A collection of utility functions supporting various operational aspects of the project.
## Main Test File (main.cpp):
The primary testing and initialization file for the project, outlining the entire process flow from service generation to data processing and output generation.


