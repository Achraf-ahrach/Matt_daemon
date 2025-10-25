#include "Auth.hpp"

bool Auth::authenticate(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(authMutex);
    return (username == ADMIN_USERNAME && password == ADMIN_PASSWORD);
}