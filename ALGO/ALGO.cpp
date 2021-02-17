// ALGO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include "json/json.h"
#include "API_UTILS.h"


#define CURL_STATICLIB
#include <curl\curl.h>

const char *info_path = "C:/Users/bktor/Desktop/INFO.txt";

struct PATH_STRUCT
{
	std::string ACCOUNT_PATH;
	std::string MARKET_CLOSE;
	std::string CANDLES;
	std::string LATEST;

};

class CERBERUS_RISK
{
	public:
		static double marginRate;
		int openTradeCount, pendingOrderCount;
		int BROKERAGE;
		double balance, pl;

		CERBERUS_RISK(int BROKERAGE);
		std::string init_risk(std::string ACCOUNT_PATH, std::string API_TOKEN);

};

CERBERUS_RISK::CERBERUS_RISK(int BROKERAGE_INPUT)
{
	
	BROKERAGE = BROKERAGE_INPUT;

	
}

std::string CERBERUS_RISK::init_risk(std::string ACCOUNT_PATH, std::string API_TOKEN)
{
	std::string response = DownloadJSON(ACCOUNT_PATH, API_TOKEN);
	return response;
};

class CERBERUS: public CERBERUS_RISK
{
	public:
		
		int BROKERAGE, LIVE_DEMO;
		std::string BROKERAGE_STR, URL, ACCOUNT_ID, API_TOKEN;
		PATH_STRUCT PATH_STRUCT;

		CERBERUS(int BROKERAGE, int LIVE_DEMO);
		void set_info();
		Json::Value str_to_json(std::string response);

};

CERBERUS::CERBERUS(int BROKERAGE_INPUT, int LIVE_DEMO_INPUT) : CERBERUS_RISK(BROKERAGE_INPUT)
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
		}
	case 1:
		switch (BROKERAGE)
		{
			BROKERAGE_STR = "OANDA";
			URL = "https://api-fxpractice.oanda.com";

			PATH_STRUCT.ACCOUNT_PATH = URL + "/v3/accounts/" + ACCOUNT_ID;
		}
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
	const auto rawJsonLength = static_cast<int>(response.length());

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

	std::cout << CERBERUS_OBJ.ACCOUNT_ID << std::endl;
	std::cout << CERBERUS_OBJ.API_TOKEN << std::endl;

	std::string response = CERBERUS_OBJ.CERBERUS_RISK::init_risk(CERBERUS_OBJ.PATH_STRUCT.ACCOUNT_PATH, CERBERUS_OBJ.API_TOKEN);
	Json::Value root = CERBERUS_OBJ.str_to_json(response);

	std::cout << root["account"] << std::endl;
	//std::cout << response;

}