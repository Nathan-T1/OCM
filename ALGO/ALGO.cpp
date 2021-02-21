// ALGO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <ctime>
#include <chrono>
#include <sstream>
#include <fstream>
#include <limits>
#include "json/json.h"
#include "API_UTILS.h"


#define CURL_STATICLIB
#include <curl\curl.h>

const char *info_path = "C:/Users/bktor/Desktop/INFO.txt";
const char error_file[] = "errors.txt";

struct PATH_STRUCT
{
	std::string ACCOUNTS;
	std::string ACCOUNT_PATH;
	std::string CHANGES;
	std::string INSTRUMENTS;
	std::string MARKET_CLOSE;
	std::string CANDLES;
	std::string LATEST;

};

template <typename T>
std::string NumberToString(T Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

class CERBERUS
{
	private:
		std::string ACCOUNT_ID, API_TOKEN;

	public:
		
		int BROKERAGE, LIVE_DEMO;
		std::string BROKERAGE_STR, URL;
		PATH_STRUCT PATH_STRUCT;

		double NAV, balance, marginRate, marginUsed, unrealizedPL;
		int openPositionCount, openTradeCount, pendingOrderCount, lastTransactionID;

		CERBERUS(int BROKERAGE, int LIVE_DEMO);
		void set_info();
		void get_changes();
		Json::Value str_to_json(std::string response);
		Json::Value init_risk();
		void msg_writer(const char filename[], std::string message);

};

CERBERUS::CERBERUS(int BROKERAGE_INPUT, int LIVE_DEMO_INPUT)
{
	BROKERAGE = BROKERAGE_INPUT;
	LIVE_DEMO = LIVE_DEMO_INPUT;
	set_info();

	switch (LIVE_DEMO)
	{
	case 0:
		switch (BROKERAGE)
		{
		case 0:
			BROKERAGE_STR = "OANDA";
			URL = "https://api-fxtrade.oanda.com";

			PATH_STRUCT.ACCOUNT_PATH = URL + "/v3/accounts/" + ACCOUNT_ID;
			PATH_STRUCT.INSTRUMENTS = URL + "/v3/accounts/" + ACCOUNT_ID + "/instruments";
			PATH_STRUCT.CHANGES = URL + "/v3/accounts/" + ACCOUNT_ID + "/changes";
			PATH_STRUCT.ACCOUNTS = URL + "/v3/accounts";
		}
	case 1:
		switch (BROKERAGE)
		{
			BROKERAGE_STR = "OANDA";
			URL = "https://api-fxpractice.oanda.com";

			PATH_STRUCT.ACCOUNT_PATH = URL + "/v3/accounts/" + ACCOUNT_ID;
			PATH_STRUCT.INSTRUMENTS = URL + "/v3/accounts/" + ACCOUNT_ID + "/instruments";
			PATH_STRUCT.CHANGES = URL + "/v3/accounts/" + ACCOUNT_ID + "/changes";
			PATH_STRUCT.ACCOUNTS = URL + "/v3/accounts";
		}
	}
}

void CERBERUS::msg_writer(const char filename[], std::string message)
{
	std::fstream appendFileToWorkWith;
	appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);

	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << now_time;
	std::string ts = ss.str();


	if (!appendFileToWorkWith)
	{
		appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::trunc);
		appendFileToWorkWith << ts + ": " + message << "\n";;
		appendFileToWorkWith.close();

	}
	else
	{    
		appendFileToWorkWith << ts + ": " + message << "\n";
		appendFileToWorkWith.close();

	}
}

Json::Value CERBERUS::init_risk()
{
	std::string response = DownloadJSON(PATH_STRUCT.ACCOUNT_PATH, API_TOKEN);
	Json::Value root = str_to_json(response);

	if (root.isObject() && root.isMember("errorMessage"))
	{
		std::string errorMessage = "Error in init_risk(): " + root["errorMessage"].asString();
		msg_writer(error_file, errorMessage);
		std::exit(0);
	}

	else if (root.isObject(), !root.isMember("errorMessage"))
	{
		NAV = stod(root["account"]["NAV"].asString());
		balance = stod(root["account"]["balance"].asString());
		marginRate = stod(root["account"]["marginRate"].asString());
		marginUsed = stod(root["account"]["marginUsed"].asString());

		openPositionCount = stoi(root["account"]["openPositionCount"].asString());
		openTradeCount = stoi(root["account"]["openTradeCount"].asString());
		pendingOrderCount = stoi(root["account"]["pendingOrderCount"].asString());
		lastTransactionID = stoi(root["account"]["lastTransactionID"].asString());
	}
	return root;
};

void CERBERUS::get_changes()
{
	std::string chng_url = PATH_STRUCT.CHANGES + "?sinceTransactionID=" + NumberToString(lastTransactionID);
	std::string response = DownloadJSON(chng_url, API_TOKEN);
	Json::Value root = str_to_json(response);

	if (root.isObject() && root.isMember("errorMessage"))
	{
		std::string errorMessage = "Error in get_changes(): " + root["errorMessage"].asString();
		msg_writer(error_file, errorMessage);
		std::exit(0);
	}
	else if (root.isObject(), !root.isMember("errorMessage"))
	{
		lastTransactionID = stoi(root["lastTransactionID"].asString());
		unrealizedPL = stoi(root["state"]["unrealizedPL"].asString());
	}


	std::cout << root;

}

void CERBERUS::set_info() 
{
	std::ifstream input(info_path);
	std::string file;
	int i = 0;

	switch (BROKERAGE)
	{
	case 0:
		for (std::string line; std::getline(input, line);)
		{
			if (i == 0)
			{
				ACCOUNT_ID = line;
			}
			else if (i == 1)
			{
				API_TOKEN = line;
			}
			i++;
		}
	}
}

Json::Value CERBERUS::str_to_json(std::string response)
{
	JSONCPP_STRING err;
	Json::Value root;
	Json::CharReaderBuilder builder;
	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	const int rawJsonLength = static_cast<int>(response.length());

	if (!reader->parse(response.c_str(), response.c_str() + rawJsonLength, &root,
		&err)) {
		std::cout << "error" << std::endl;
		return EXIT_FAILURE;
	}

	return root;
}

int main()
{
	int BROKERAGE = 0;
	int LIVE_DEMO = 0;

	CERBERUS CERBERUS_OBJ(BROKERAGE, LIVE_DEMO);

	
	Json::Value response = CERBERUS_OBJ.init_risk();
	CERBERUS_OBJ.get_changes();
	//std::cout << response;

	//std::cout << root << std::endl;
	//std::cout << response;

}