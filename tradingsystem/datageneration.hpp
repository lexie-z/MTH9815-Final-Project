/**
 * Used to generate the data.
 * I utilized ChatGPT's recommendations to generate data as required.
 * @author Lexie Zhu
*/
#ifndef DATA_GENERATION_HPP
#define DATA_GENERATION_HPP


#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <map>
#include <thread>
#include <random>
#include "products.hpp"
#include "utilities.hpp"
#include "tradebookingservice.hpp"
#include "marketdataservice.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace std;
using namespace boost::gregorian;

/**
 * Prices oscillate between 99 and 101.
 * Movement in price should be in increments of 1/256.
 * Bid/Offer spread oscillates between 1/128 and 1/64.
 * Generate 1,000,000 prices for each security. Set size to 10,000 for testing purposes.
 * @param _id
 * @param _size
 * @param file
 */

// Function to generate prices for a single product/security.
void GeneratePrice(const string& _id, int _size, ofstream& file) {

    // Initialize random number generators
    thread_local random_device rd;
    thread_local mt19937_64 gen(rd());

    // Distribution for the spread, between 1/128 and 1/64
    uniform_real_distribution<double> spread_dist(1 / 128.0, 1 / 64.0);

    double minTick = 1 / 256.0; // Minimum price movement
    double amplitude = 1.0 - 2 * minTick; // Amplitude for the sinusoidal price oscillation

    // Generate _size number of prices
    for (int i = 0; i < _size; i++) {

        // Calculate central price using a sinusoidal function for natural oscillation
        double central_price = 100.0 + amplitude * sin(i * minTick * M_PI / amplitude);

        // Determine the spread randomly within the specified range
        double spread = spread_dist(gen);

        // Calculate bid and ask prices based on the central price and spread
        double bid = central_price - spread / 2;
        double ask = central_price + spread / 2;

        // Write the generated prices to the file
        file << _id << "," << PriceToString(bid) << "," << PriceToString(ask) << endl;
    }
}

// Function to generate prices for all securities
void GenerateAllPrices() {
    const string save_path("prices.txt");
    ofstream file(save_path);

    // For testing purposes, we set it to 10000; can change to 1000000 as required
    const int orderSize = 10000;

    for (const auto& [mat, bond] : bondMap) {
        cout << "Generating prices for security " << bond.first<< " ...\n";
        GeneratePrice(bond.first, orderSize, file);
    }
}

void GenerateMarketData(string _id, int _size, ofstream& file) {
    double mintick = 1.0 / 256.0; // Smallest increment for US Treasuries
    vector<int> volumeVec{ 10000000, 20000000, 30000000, 40000000, 50000000 }; // Volumes for each order level

    double LOW = 99.0 + mintick; // Lower limit for central price oscillation
    double UPPER = 101.0 - mintick; // Upper limit for central price oscillation
    bool up = true; // Flag to track direction of price movement
    double central_price = LOW; // Starting central price

    // we use size / 10, as we will generate 5 bid and 5 offer
    for (int i = 0; i < _size / 10; i++) {
        // Calculate spread oscillation between 1/128th and 1/32nd in 1/128th intervals
        double spread_increment = mintick * (2 + (i % 4));
        if(spread_increment > (1.0/32.0)) spread_increment = 1.0 / 128.0;
        double _spread = spread_increment * 0.5;

        double _top_buy = central_price - _spread; // Top buy price
        double _bottom_offer = central_price + _spread; // Bottom offer price

        // Generate 5 bids and 5 offers for each spread case
        for (int j = 0; j <= 4; j++) {
            double _buy = _top_buy - (double)j * mintick; // Calculate buy price
            double _sell = _bottom_offer + (double)j * mintick; // Calculate sell price
            int _volume = volumeVec[j % 5]; // Assign volume

            // Writing to file
            file << _id << "," << PriceToString(_buy) << "," << _volume << "," << "BID" << std::endl;
            file << _id << "," << PriceToString(_sell) << "," << _volume << "," << "OFFER" << std::endl;
        }

        // Adjust central price based on current direction
        if (central_price == UPPER) {
            up = false; // Change direction when upper limit is reached
        } else if (central_price == LOW) {
            up = true; // Change direction when lower limit is reached
        }

        // Increment or decrement central price based on direction
        central_price = up ? central_price + mintick : central_price - mintick;
    }
}

// Function to generate market data for all securities
void GenerateAllMarketData() {
    const string save_path("marketdata.txt");
    ofstream file(save_path);

    // For testing purposes, we set it to 10000; can change to 1000000 as required
    const int orderSize = 10000;

    for (const auto& [mat, bond] : bondMap) {
        std::cout << "Generating market data for security " << bond.first << " ...\n";
        GenerateMarketData(bond.first, orderSize, file);
    }
}

/** generate the trade data
 * The price should oscillate between 99.0 (BUY) and 100.0 (SELL).
 * Positions should be across books TRSY1, TRSY2, and TRSY3.
 * Trades for each security should alternate between BUY and SELL
 * and cycle from 1000000, 2000000, 3000000, 4000000, and 5000000 for quantity, and then repeat back from 1000000.
 */

void GenerateTradeData(string _id, int _size, ofstream& file) {
    double mintick = 1.0 / 256.0;
    vector<int> volumeVec{ 10000000,20000000,30000000,40000000,50000000 };

    thread_local random_device rd;
    thread_local mt19937_64 gen(rd());
    thread_local uniform_real_distribution<double> d(0.0, 1.0);

    for (int i = 0; i < _size; i++) {
        int _n = (int)(d(gen) * 512);
        Side _side = (i % 2) ? BUY : SELL;
        string _string_side = _side == BUY ? "BUY" : "SELL";
        int _volume = volumeVec[i % 5];
        int _market = (int)(d(gen) * 3) % 3 + 1;
        string _book_name = "TRSY" + to_string(_market);
        double _price = 99.0 + mintick * (double)_n;
        string trading_id = GenerateTradingId(12);
        file << _id << "," << trading_id << "," << PriceToString(_price) << "," << _book_name << "," << _volume << "," << _string_side << std::endl;
    }
}

// Create 10 trades for each security.
void GenerateAllTradeData() {
    const string save_path("trades.txt");
    ofstream file(save_path);
    const int tradeSize = 10;
    for (const auto& [mat, bond] : bondMap) {
        std::cout << "Generating trades for security " << bond.first << " ...\n";
        GenerateTradeData(bond.first, tradeSize, file);
    }
}

void GenerateInquiryData(string _id, int _size, ofstream& file) {
    double mintick = 1.0 / 256.0;
    vector<int> volumeVec{ 10000000,20000000,30000000,40000000,50000000 };

    thread_local random_device rd;
    thread_local mt19937_64 gen(rd());
    thread_local uniform_real_distribution<double> d(0.0, 1.0);

    for (int i = 0; i < _size; i++) {
        int _n = (int)(d(gen) * 512);
        string _string_side = (i % 2) ? "BUY" : "SELL";
        int _volume = volumeVec[i % 5];
        int _numTicks = rand() % 512;
        double _price = 99.0 + mintick * (double)_n;
        string _inquiry_id = GenerateTradingId(9);
        _inquiry_id = "INQ" + _inquiry_id;		// indicating it's "INQUIRY"
        file << _inquiry_id << "," << _id << "," << _string_side << "," << _volume << "," << PriceToString(_price) << "," << "RECEIVED" << std::endl;
    }
}

//Create 10 inquiries for each security.
void GenerateAllInquiryData() {
    const string save_path("inquiries.txt");
    ofstream file(save_path);
    const int inqSize = 10;
    for (const auto& [mat, bond] : bondMap) {
        std::cout << "Generating inquiries for security " << bond.first << " ...\n";
        GenerateInquiryData(bond.first, inqSize, file);
    }
}

#endif // !DATA_GENERATION_HPP

