#pragma once

#include <string>

#include "payment/HttpClient.h"
#include "payment/PaymentGateway.h"

namespace bitwave::payment {

class PaystackGateway : public PaymentGateway {
public:
    // `http` must outlive this object. `secretKey` is sent only in the
    // Authorization header, never in a URL or body.
    PaystackGateway(HttpClient& http, std::string secretKey);

    ChargeInit initializeCharge(const std::string& email,
                                long long amountCents,
                                const std::string& currency,
                                const std::string& reference) override;

    ChargeVerification verifyCharge(const std::string& reference) override;

private:
    HttpClient& http_;
    std::string secretKey_;
};

} // namespace bitwave::payment