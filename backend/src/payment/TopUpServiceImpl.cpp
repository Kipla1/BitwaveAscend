#include "payment/TopUpServiceImpl.h"

#include <cstdint>
#include <random>
#include <sstream>

namespace bitwave::payment {

namespace {

// Placeholders: our own limits and exchange rate, not Paystack's. Agree on
// real values with the team.
constexpr int kMinAmountKes = 10;
constexpr int kMaxAmountKes = 10000;
constexpr long long kCoinsPerKes = 10;
constexpr const char* kCurrency = "KES";

// Not a secret, just a unique label. Real protection comes from verifying
// with the provider; uniqueness is enforced by the UNIQUE column.
std::string makeReference(std::int64_t userId) {
    std::random_device rd;
    const std::uint64_t random = (static_cast<std::uint64_t>(rd()) << 32) | rd();
    std::ostringstream out;
    out << "bw_" << userId << "_" << std::hex << random;
    return out.str();
}

} // namespace

TopUpServiceImpl::TopUpServiceImpl(persistence::Database& db,
                                   PaymentGateway& gateway,
                                   persistence::PaymentOrderRepository& orders,
                                   Wallet& wallet,
                                   const auth::Session& session)
    : db_(db), gateway_(gateway), orders_(orders), wallet_(wallet), session_(session) {}

TopUpStart TopUpServiceImpl::startTopUp(const std::string& email, int amountKes) {
    TopUpStart out;

    if (!session_.isActive()) {
        out.message = "You must be logged in to add funds.";
        return out;
    }
    if (email.find('@') == std::string::npos) {
        out.message = "Enter a valid email address.";
        return out;
    }
    if (amountKes < kMinAmountKes || amountKes > kMaxAmountKes) {
        out.message = "Amount must be between KES 10 and KES 10,000.";
        return out;
    }

    const long long amountCents = static_cast<long long>(amountKes) * 100;
    const long long coins = static_cast<long long>(amountKes) * kCoinsPerKes;
    const std::string reference = makeReference(session_.user()->id);

    // Record the attempt BEFORE contacting the provider, so every payment
    // that could have happened has a row we can re-check later.
    orders_.create(session_.user()->id, reference, amountCents, kCurrency, coins);

    const auto init = gateway_.initializeCharge(email, amountCents, kCurrency, reference);
    if (!init.ok) {
        orders_.markFailed(reference);
        out.message = "Could not start the payment. Please try again.";
        return out;
    }

    out.success = true;
    out.reference = reference;
    out.paymentUrl = init.authorizationUrl;
    out.message = "Open the payment link to complete the payment.";
    return out;
}

TopUpResult TopUpServiceImpl::confirmTopUp(const std::string& reference) {
    TopUpResult out;

    if (!session_.isActive()) {
        out.message = "You must be logged in to confirm a payment.";
        return out;
    }

    // Same message for "doesn't exist" and "belongs to someone else", so
    // references can't be probed.
    const auto order = orders_.findByReference(reference);
    if (!order || order->userId != session_.user()->id) {
        out.message = "Payment not found.";
        return out;
    }
    if (order->status == persistence::OrderStatus::Succeeded) {
        out.success = true;
        out.message = "This payment was already credited.";
        return out;
    }
    if (order->status == persistence::OrderStatus::Failed) {
        out.message = "This payment did not go through.";
        return out;
    }

    const auto check = gateway_.verifyCharge(reference);
    if (!check.ok) {
        out.message = "Could not reach the payment provider. Try again shortly.";
        return out; // order stays pending, so retrying is safe
    }
    if (check.status == ChargeStatus::Pending) {
        out.message = "Payment not completed yet.";
        return out;
    }
    if (check.status == ChargeStatus::Failed) {
        orders_.markFailed(reference);
        out.message = "The payment was not successful.";
        return out;
    }

    // Provider says paid. Compare against OUR stored order, not anything
    // the client told us.
    if (check.amountCents != order->amountCents || check.currency != order->currency) {
        // Production would flag this for manual review rather than just failing it.
        orders_.markFailed(reference);
        out.message = "Payment amount did not match. Nothing was credited.";
        return out;
    }

    db_.beginTransaction();
    try {
        if (!orders_.markSucceeded(reference)) {
            // Someone else credited it between our check and now.
            db_.rollbackTransaction();
            out.success = true;
            out.message = "This payment was already credited.";
            return out;
        }
        wallet_.addFunds(order->coins);
        db_.commitTransaction();
    } catch (...) {
        db_.rollbackTransaction();
        throw;
    }

    out.success = true;
    out.coinsAdded = order->coins;
    out.message = "Funds added.";
    return out;
}

} // namespace bitwave::payment