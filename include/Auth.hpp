#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>
#include <mutex>

class Auth {
public:
    Auth() = default;
    ~Auth() = default;

    bool authenticate(const std::string& username, const std::string& password);

    static constexpr const char* ADMIN_USERNAME = "admin";
    static constexpr const char* ADMIN_PASSWORD = "admin";

private:
    std::mutex authMutex;
};

#endif