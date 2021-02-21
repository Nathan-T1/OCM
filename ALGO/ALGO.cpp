// ALGO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <ctime>
#include <chrono>
#include <map>
#include <sstream>
#include <fstream>
#include <limits>
#include <iomanip>
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

struct candle
{
	double c, h, l, o;
	std::string complete;
	double volume;
	std::time_t time_t;
	std::string time;
};

const char *instruments[3] = { "EUR_USD", "USD_JPY", "EUR_JPY" };

template <typename T>
std::string NumberToString(T Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

std::time_t time_from_string(std::string time_details)
{
	struct std::tm tm;
	std::istringstream ss(time_details);
	ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
	std::time_t time = mktime(&tm);

	return time;
}

class CERBERUS
{
	private:
		std::string ACCOUNT_ID, API_TOKEN;

	public:
		
		int BROKERAGE, LIVE_DEMO, past_history, price_at_switch;
		std::string price_at_long;
		std::string BROKERAGE_STR, URL;
		std::string price_at, granularity;
		PATH_STRUCT PATH_STRUCT;

		double NAV, balance, marginRate, marginUsed, unrealizedPL;
		int openPositionCount, openTradeCount, pendingOrderCount, lastTransactionID;
		std::map<std::string, std::vector<candle>> candle_dict;


		CERBERUS(int BROKERAGE, int LIVE_DEMO);
		void set_info();
		void get_changes();
		void candles(std::string instrument);
		Json::Value str_to_json(std::string response);
		Json::Value init_risk();
		void msg_writer(const char filename[], std::string message);

};

CERBERUS::CERBERUS(int BROKERAGE_INPUT, int LIVE_DEMO_INPUT)
{
	BROKERAGE = BROKERAGE_INPUT;
	LIVE_DEMO = LIVE_DEMO_INPUT;
	set_info();

	price_at = "M";
	price_at_switch = 0;
	granularity = "S5";
	past_history = 10;

	switch (LIVE_DEMO)
	{
	case 0:
		switch (BROKERAGE)
		{
		case 0:
			BROKERAGE_STR = "OANDA";
			URL = "https://api-fxtrade.oanda.com";

			PATH_STRUCT.ACCOUNT_PATH = URL + "/v3/accounts/" + ACCOUNT_ID;
			PATH_STRUCT.INSTRUMENTS = URL + "/v3/instruments/";
			PATH_STRUCT.CHANGES = URL + "/v3/accounts/" + ACCOUNT_ID + "/changes";
			PATH_STRUCT.ACCOUNTS = URL + "/v3/accounts";
		}
	case 1:
		switch (BROKERAGE)
		{
			BROKERAGE_STR = "OANDA";
			URL = "https://api-fxpractice.oanda.com";

			PATH_STRUCT.ACCOUNT_PATH = URL + "/v3/accounts/" + ACCOUNT_ID;
			PATH_STRUCT.INSTRUMENTS = URL + "/v3/instruments/";
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

void CERBERUS::candles(std::string instrument)
{
	std::string candle_url = PATH_STRUCT.INSTRUMENTS + instrument + "/candles?count=" + NumberToString(past_history);
	candle_url = candle_url + "&price=" + price_at + "&granularity=" + granularity;
	
	std::string response = DownloadJSON(candle_url, API_TOKEN);
	Json::Value root = str_to_json(response);

	if (root.isObject() && root.isMember("errorMessage"))
	{
		std::string errorMessage = "Error in candles(): " + root["errorMessage"].asString();
		msg_writer(error_file, errorMessage);
		std::exit(0);
	}
	else if (root.isObject())
	{
		switch (price_at_switch)
		{
		case 0:
			price_at_long = "mid";
		case 1:
			const char* strarray[] = { "bid", "ask"};
		}
		

		std::cout << root;
		std::vector<candle> values;
		for (Json::Value::ArrayIndex i = 0; i != root["candles"].size(); i++)
		{
			if (root["candles"][i].isMember("time"))
			{
				candle candle_stuct;

				candle_stuct.complete = root["candles"][i]["complete"].asString();
				candle_stuct.time = root["candles"][i]["time"].asString();
				candle_stuct.time_t = time_from_string(candle_stuct.time);

				candle_stuct.c = stod(root["candles"][i][price_at_long]["c"].asString());
				candle_stuct.h = stod(root["candles"][i][price_at_long]["h"].asString());
				candle_stuct.l = stod(root["candles"][i][price_at_long]["l"].asString());
				candle_stuct.o = stod(root["candles"][i][price_at_long]["o"].asString());
				candle_stuct.volume = stod(root["candles"][i]["volume"].asString());
				
				values.push_back(candle_stuct);

				std::cout << candle_stuct.time << std::endl;
			}
		}
		candle_dict[instrument] = values;
	}
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
	CERBERUS_OBJ.candles(instruments[0]);
	//CERBERUS_OBJ.get_changes();
	//std::cout << response;

	//std::cout << root << std::endl;
	//std::cout << response;

}