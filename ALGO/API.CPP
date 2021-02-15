
#define CURL_STATICLIB

#include "pch.h"
#include <iostream>
#include <sstream>

#include <curl\curl.h>


static std::string *DownloadedResponse;
static int writer(char *data, size_t size, size_t nmemb, std::string *buffer_in)
{

	// Is there anything in the buffer?  
	if (buffer_in != NULL)
	{
		// Append the data to the buffer    
		buffer_in->append(data, size * nmemb);

		// How much did we write?   
		DownloadedResponse = buffer_in;

		return size * nmemb;
	}

	return 0;

}

std::string DownloadJSON(std::string URL)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL; // init to NULL is important 
	std::ostringstream oss;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charset: utf-8");
	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		res = curl_easy_perform(curl);

		if (CURLE_OK == res)
		{
			char *ct;
			res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
			if ((CURLE_OK == res) && ct)
				return *DownloadedResponse;
		}
	}

	curl_slist_free_all(headers);
}