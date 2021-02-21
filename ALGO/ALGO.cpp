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
		Json::Value str_to_json(std::string response);
		Json::Value init_risk(std::string ACCOUNT_PATH, std::string API_TOKEN);
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

Json::Value CERBERUS::init_risk(std::string ACCOUNT_PATH, std::string API_TOKEN)
{
	std::string response = DownloadJSON(ACCOUNT_PATH, API_TOKEN);
	Json::Value root = str_to_json(response);

	if (root.isObject() && root.isMember("errorMessage"))
	{
		std::string errorMessage = root["errorMessage"].asString();
		msg_writer(error_file, errorMessage);
		std::exit(0);
	}

	return root;
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

	std::cout << CERBERUS_OBJ.ACCOUNT_ID << std::endl;
	std::cout << CERBERUS_OBJ.API_TOKEN << std::endl;

	Json::Value response = CERBERUS_OBJ.init_risk(CERBERUS_OBJ.PATH_STRUCT.ACCOUNT_PATH, CERBERUS_OBJ.API_TOKEN);
	std::cout << response;
	//std::cout << root << std::endl;
	//std::cout << response;

}