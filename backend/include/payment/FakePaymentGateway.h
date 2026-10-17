#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>

#include "payment/PaymentGateway.h"

namespace bitwave::payment {

// Test double: no network. Set the public fields to script what the
// "provider" does, e.g. gateway.statusToReport = ChargeStatus::Pending.
class FakePaymentGateway : public PaymentGateway {
public:
    bool failInitialize = false;
    bool failVerify = false;
    ChargeStatus statusToReport = ChargeStatus::Success;
    std::optional<long long> amountOverrideCents; // simulate a wrong amount

    ChargeInit initializeCharge(const std::string&, long long amountCents,
                                const std::string& currency,
                                const std::string& reference) override {
        ChargeInit result;
        if (failInitialize) {
            result.error = "fake network failure";
            return result;
        }
        charges_[reference] = {amountCents, currency};
        result.ok = true;
        result.authorizationUrl = "https://fake.paystack.test/pay/" + reference;
        return result;
    }

    ChargeVerification verifyCharge(const std::string& reference) override {
        ChargeVerification result;
        if (failVerify) {
            result.error = "fake network failure";
            return result;
        }
        auto it = charges_.find(reference);
        if (it == charges_.end()) {
            result.error = "unknown reference";
            return result;
        }
        result.ok = true;
        result.status = statusToReport;
        result.amountCents = amountOverrideCents.value_or(it->second.first);
        result.currency = it->second.second;
        return result;
    }

private:
    std::map<std::string, std::pair<long long, std::string>> charges_;
};

} // namespace bitwave::payment