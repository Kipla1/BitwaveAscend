#pragma once

#include <string>
#include <vector>

namespace bitwave::payment {

struct HttpResponse {
    bool        ok = false;   // false = never got a reply (network error, timeout)
    long        status = 0;   // HTTP status code when ok
    std::string body;
    std::string error;
};

// Headers are full "Name: value" lines.
class HttpClient {
public:
    virtual ~HttpClient() = default;

    virtual HttpResponse post(const std::string& url,
                              const std::vector<std::string>& headers,
                              const std::string& body) = 0;

    virtual HttpResponse get(const std::string& url,
                             const std::vector<std::string>& headers) = 0;
};

} // namespace bitwave::payment