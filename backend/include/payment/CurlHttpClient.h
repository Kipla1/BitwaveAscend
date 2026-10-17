#pragma once

#include "payment/HttpClient.h"

namespace bitwave::payment {

class CurlHttpClient : public HttpClient {
public:
    CurlHttpClient(); // initializes libcurl once per process

    HttpResponse post(const std::string& url,
                      const std::vector<std::string>& headers,
                      const std::string& body) override;

    HttpResponse get(const std::string& url,
                     const std::vector<std::string>& headers) override;
};

} // namespace bitwave::payment