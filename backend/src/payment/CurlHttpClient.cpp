#include "payment/CurlHttpClient.h"

#include <curl/curl.h>
#include <memory>

namespace bitwave::payment {

namespace {

struct CurlGlobal {
    CurlGlobal() { curl_global_init(CURL_GLOBAL_DEFAULT); }
    ~CurlGlobal() { curl_global_cleanup(); }
};

size_t writeCallback(char* data, size_t size, size_t count, void* userdata) {
    static_cast<std::string*>(userdata)->append(data, size * count);
    return size * count;
}

// postBody == nullptr means GET.
HttpResponse perform(const std::string& url,
                     const std::vector<std::string>& headers,
                     const std::string* postBody) {
    HttpResponse response;

    // Same RAII idea as the sqlite Statement alias: cleanup runs on every exit path.
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), &curl_easy_cleanup);
    if (!curl) {
        response.error = "Could not start the HTTP client.";
        return response;
    }

    curl_slist* rawHeaders = nullptr;
    for (const auto& header : headers) {
        rawHeaders = curl_slist_append(rawHeaders, header.c_str());
    }
    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headerList(rawHeaders, &curl_slist_free_all);

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headerList.get());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 20L);

    if (postBody) {
        curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, postBody->c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(postBody->size()));
    }

    const CURLcode code = curl_easy_perform(curl.get());
    if (code != CURLE_OK) {
        response.error = curl_easy_strerror(code);
        return response;
    }

    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.status);
    response.ok = true;
    return response;
}

} // namespace

CurlHttpClient::CurlHttpClient() {
    static CurlGlobal global;
    (void)global;
}

HttpResponse CurlHttpClient::post(const std::string& url,
                                  const std::vector<std::string>& headers,
                                  const std::string& body) {
    return perform(url, headers, &body);
}

HttpResponse CurlHttpClient::get(const std::string& url,
                                 const std::vector<std::string>& headers) {
    return perform(url, headers, nullptr);
}

} // namespace bitwave::payment