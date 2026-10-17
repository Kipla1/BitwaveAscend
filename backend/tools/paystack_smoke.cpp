// Runs one real TEST-mode payment against Paystack, end to end.
// Not part of the game: a developer tool for checking the integration.

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "payment/CurlHttpClient.h"
#include "payment/PaystackGateway.h"

int main() {
    const char* key = std::getenv("PAYSTACK_SECRET_KEY");
    // Refuse anything but a test key, so this tool can never touch real money.
    if (!key || std::string(key).rfind("sk_test_", 0) != 0) {
        std::cerr << "Set PAYSTACK_SECRET_KEY to a TEST secret key (sk_test_...) first.\n";
        return 1;
    }

    bitwave::payment::CurlHttpClient http;
    bitwave::payment::PaystackGateway gateway(http, key);

    const std::string reference = "smoke_" + std::to_string(std::time(nullptr));
    const auto init = gateway.initializeCharge("test@example.com", 10000 /* KES 100 */, "KES", reference);
    if (!init.ok) {
        std::cerr << "Initialize failed: " << init.error << "\n";
        return 1;
    }

    std::cout << "Open this link and pay with a test method:\n  " << init.authorizationUrl << "\n"
              << "Test card: 4084 0840 8408 4081, any future expiry, CVV 408.\n"
              << "Press Enter once you've paid...\n";
    std::cin.get();

    const auto check = gateway.verifyCharge(reference);
    if (!check.ok) {
        std::cerr << "Verify failed: " << check.error << "\n";
        return 1;
    }

    using bitwave::payment::ChargeStatus;
    const char* status = check.status == ChargeStatus::Success ? "success"
                       : check.status == ChargeStatus::Failed  ? "failed"
                                                               : "pending";
    std::cout << "status: " << status << "\namount: " << check.amountCents
              << "\ncurrency: " << check.currency << "\n";
    return 0;
}