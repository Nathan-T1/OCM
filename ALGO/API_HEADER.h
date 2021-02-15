#pragma once

#ifndef API_HEADER
#define API_HEADER


#define CURL_STATICLIB
#include <iostream>

std::string DownloadJSON(std::string URL);

#endif // !API_HEADER

