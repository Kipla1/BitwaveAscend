#pragma once

#include <string>

// Generic result wrapper used across backend services so that failures are
// represented explicitly instead of via exceptions or sentinel values.
// This is deliberately minimal during repository initialization; concrete
// result types (LoginResult, PurchaseResult, ...) are introduced alongside
// the systems that produce them.

namespace bitwave::common {

struct Result {
    bool success = false;
    std::string message;

    static Result ok(std::string msg = "") {
        return Result{true, std::move(msg)};
    }

    static Result fail(std::string msg) {
        return Result{false, std::move(msg)};
    }
};

} // namespace bitwave::common
