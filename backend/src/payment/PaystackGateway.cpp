#include "payment/PaystackGateway.h"

#include <algorithm>
#include <cctype>
#include <nlohmann/json.hpp>

namespace bitwave::payment {

namespace {

constexpr const char* kBaseUrl = "https://api.paystack.co";

// The reference goes into a URL path, so only allow characters that are
// safe there. Our own references (bw_<id>_<hex>) always pass.
bool isSafeReference(const std::string& reference) {
    return !reference.empty() &&
           std::all_of(reference.begin(), reference.end(), [](unsigned char c) {
               return std::isalnum(c) || c == '_' || c == '-';
           });
}

// Paystack replies {"status": true/false, "message": "...", "data": {...}}.
// Returns true only for a well-formed, successful reply.
bool parseReply(const HttpResponse& response, nlohmann::json& out, std::string& error) {
    if (!response.ok) {
        error = "Network error: " + response.error;
        return false;
    }
    out = nlohmann::json::parse(response.body, nullptr, false);
    if (out.is_discarded() || !out.is_object()) {
        error = "Unexpected reply from Paystack (HTTP " + std::to_string(response.status) + ").";
        return false;
    }
    if (!out.value("status", false)) {
        error = out.value("message", std::string("Paystack rejected the request."));
        return false;
    }
    return true;
}

// Only an exact "success" ever leads to credited coins. Anything unfamiliar
// counts as Pending, which is safe: Pending never credits, and the player
// can confirm again later. "abandoned" is Pending too, since the player may
// still finish paying.
ChargeStatus mapStatus(const std::string& status) {
    if (status == "success") return ChargeStatus::Success;
    if (status == "failed" || status == "reversed") return ChargeStatus::Failed;
    return ChargeStatus::Pending;
}

} // namespace

PaystackGateway::PaystackGateway(HttpClient& http, std::string secretKey)
    : http_(http), secretKey_(std::move(secretKey)) {}

ChargeInit PaystackGateway::initializeCharge(const std::string& email,
                                             long long amountCents,
                                             const std::string& currency,
                                             const std::string& reference) {
    ChargeInit result;
    try {
        const nlohmann::json body = {
            {"email", email},
            {"amount", amountCents},
            {"currency", currency},
            {"reference", reference},
        };
        const std::vector<std::string> headers = {
            "Authorization: Bearer " + secretKey_,
            "Content-Type: application/json",
        };

        const auto response = http_.post(std::string(kBaseUrl) + "/transaction/initialize",
                                         headers, body.dump());

        nlohmann::json reply;
        if (!parseReply(response, reply, result.error)) {
            return result;
        }

        result.authorizationUrl = reply.at("data").at("authorization_url").get<std::string>();
        result.ok = !result.authorizationUrl.empty();
        if (!result.ok) {
            result.error = "Paystack did not return a payment link.";
        }
    } catch (const nlohmann::json::exception&) {
        result.ok = false;
        result.error = "Unexpected reply from Paystack.";
    }
    return result;
}

ChargeVerification PaystackGateway::verifyCharge(const std::string& reference) {
    ChargeVerification result;
    if (!isSafeReference(reference)) {
        result.error = "Invalid payment reference.";
        return result;
    }

    try {
        const std::vector<std::string> headers = {"Authorization: Bearer " + secretKey_};
        const auto response = http_.get(std::string(kBaseUrl) + "/transaction/verify/" + reference,
                                        headers);

        nlohmann::json reply;
        if (!parseReply(response, reply, result.error)) {
            return result;
        }

        const auto& data = reply.at("data");
        result.status = mapStatus(data.at("status").get<std::string>());
        result.amountCents = data.at("amount").get<long long>();
        result.currency = data.at("currency").get<std::string>();
        result.ok = true;
    } catch (const nlohmann::json::exception&) {
        result.ok = false;
        result.error = "Unexpected reply from Paystack.";
    }
    return result;
}

} // namespace bitwave::payment