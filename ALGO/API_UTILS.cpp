#include "pch.h"
#include <iostream>
#include <sstream>

#define CURL_STATICLIB
#include <curl\curl.h>



static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}


std::string DownloadJSON(std::string URL, std::string API_KEY)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL; // init to NULL is important 
	std::string readBuffer;

	std::string _auth_string = "Authorization: Bearer " + API_KEY;
	headers = curl_slist_append(headers, _auth_string.c_str());
	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (CURLE_OK == res)
		{
			char *ct;
			res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
			if ((CURLE_OK == res) && ct)
				return readBuffer;
		}
	}

	curl_slist_free_all(headers);
}
