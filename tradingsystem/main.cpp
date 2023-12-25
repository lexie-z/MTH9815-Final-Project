/** This is the main file for the final project
* @author: Lexie Zhu
*/
#include <iostream>
#include "products.hpp"
#include "algoexecutionservice.hpp"
#include "algostreamingservice.hpp"
#include "executionservice.hpp"
#include "GUIservice.hpp"
#include "historicaldataservice.hpp"
#include "inquiryservice.hpp"
#include "marketdataservice.hpp"
#include "positionservice.hpp"
#include "pricingservice.hpp"
#include "riskservice.hpp"
#include "streamingservice.hpp"
#include "tradebookingservice.hpp"
#include "datageneration.hpp"
#include "utilities.hpp"
#include <random>

void initialize() {
    GenerateAllPrices();
    GenerateAllTradeData();
    GenerateAllMarketData();
    GenerateAllInquiryData();
}

int main() {
    std::cout << GetTimeStamp() << " Program Started. " << std::endl;
    initialize(); // run if there are no existing data txts
    std::cout << GetTimeStamp() << " Data Prepared." << std::endl;

    // Initialization.
    MarketDataService<Bond> BondMarketDataService;
    PricingService<Bond> BondPricingService;
    TradeBookingService<Bond> BondTradeBookingService;
    PositionService<Bond> BondPositionService;
    RiskService<Bond> BondRiskService;
    AlgoExecutionService<Bond> BondAlgoExecutionService;
    AlgoStreamingService<Bond> BondAlgoStreamingService;
    ExecutionService<Bond> BondExecutionService;
    StreamingService<Bond> BondStreamingService;
    InquiryService<Bond> BondInquiryService;
    GUIService<Bond> BondGUIService;
    std::cout << "Services initialized.\n";

    //historical service initialization.
    HistoricalDataService<Position<Bond>> histPositionService(POSITION);
    HistoricalDataService<PV01<Bond>> histRiskService(RISK);
    HistoricalDataService<ExecutionOrder<Bond>> histExecutionService(EXECUTION);
    HistoricalDataService<PriceStream<Bond>> histStreamingService(STREAMING);
    HistoricalDataService<Inquiry<Bond>> histInquiryService(INQUIRY);
    std::cout << "Historical services initialized." << std::endl;

    // Linking
    BondPricingService.AddListener(BondGUIService.GetListener()); //GUI listens to PricingService
    BondPricingService.AddListener(BondAlgoStreamingService.GetListener()); //histStreaming -> streaming -> AlgoStreaming -> Pricing
    BondAlgoStreamingService.AddListener(BondStreamingService.GetListener());
    BondStreamingService.AddListener(histStreamingService.GetServiceListener());
    BondMarketDataService.AddListener(BondAlgoExecutionService.GetListener());//histExe -> Exe -> AlgoExe -> MarketData
    BondAlgoExecutionService.AddListener(BondExecutionService.GetListener());
    BondExecutionService.AddListener(histExecutionService.GetServiceListener());
    BondExecutionService.AddListener(BondTradeBookingService.GetListener()); // TradeBooking -> Execution.
    BondTradeBookingService.AddListener(BondPositionService.GetListener());//histPos -> Pos histRisk -> Risk
    BondPositionService.AddListener(BondRiskService.GetListener());
    BondPositionService.AddListener(histPositionService.GetServiceListener());//Risk -> Pos
    BondRiskService.AddListener(histRiskService.GetServiceListener()); //Pos -> Trade Booking
    BondInquiryService.AddListener(histInquiryService.GetServiceListener());//histInquiry -> inquiry
    std::cout << GetTimeStamp() << " Services linked successfully." << std::endl;

    //load data
    ifstream priceData("prices.txt");
    ifstream tradeData("trades.txt");
    ifstream inquiryData("inquiries.txt");
    ifstream marketData("marketdata.txt");
    std::cout << GetTimeStamp() << " Data linked successfully." << std::endl;

    //read prices
    BondPricingService.GetConnector()->Subscribe(priceData);
    std::cout << GetTimeStamp() << " Price data processed." << std::endl;
    //trade
    BondTradeBookingService.GetConnector()->Subscribe(tradeData);
    std::cout << GetTimeStamp() << " Trade data processed." << std::endl;
    //market
    BondMarketDataService.GetConnector()->Subscribe(marketData);
    std::cout << GetTimeStamp() << " Market data processed." << std::endl;
    //inquiry
    BondInquiryService.GetConnector()->Subscribe(inquiryData);
    std::cout << GetTimeStamp() << " Inquiry data processed." << std::endl;

    std::cout << GetTimeStamp() << "Finished." << std::endl;
    system("sleep 5");
}