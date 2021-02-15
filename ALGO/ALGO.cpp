// ALGO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
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

class CERBERUS
{
	public:
		int BROKERAGE, LIVE_DEMO;
		std::string BROKERAGE_STR, URL, ACCOUNT_ID, API_TOKEN;
		PATH_STRUCT PATH_STRUCT;

		CERBERUS(int BROKERAGE, int LIVE_DEMO);
		void set_info();

};

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

int main()
{
	int BROKERAGE = 0;
	int LIVE_DEMO = 0;

	CERBERUS CERBERUS_OBJ(BROKERAGE,LIVE_DEMO);

	std::cout << CERBERUS_OBJ.ACCOUNT_ID << std::endl;
	std::cout << CERBERUS_OBJ.API_TOKEN << std::endl;

	std::string response = DownloadJSON(CERBERUS_OBJ.PATH_STRUCT.ACCOUNT_PATH, CERBERUS_OBJ.API_TOKEN);

	std::cout << response;
