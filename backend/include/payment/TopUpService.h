#pragma once

#include <string>

namespace bitwave::payment {

struct TopUpStart {
    bool        success = false;
    std::string reference;   // keep this; confirmTopUp needs it
    std::string paymentUrl;  // open in the player's browser
    std::string message;
};

struct TopUpResult {
    bool        success = false;
    long long   coinsAdded = 0; // 0 when already credited earlier
    std::string message;
};

// Buying in-game coins with real money. Separate from PaymentService,
// which spends coins on store items.
class TopUpService {
public:
    virtual ~TopUpService() = default;

    virtual TopUpStart startTopUp(const std::string& email, int amountKes) = 0;
    virtual TopUpResult confirmTopUp(const std::string& reference) = 0;
};

} // namespace bitwave::payment