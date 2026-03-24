#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <userver/components/component_base.hpp>

namespace user_service {

enum class Role {
    Client,
    Seller
};

struct User {
    int id;
    std::string username;
    std::string password;
    std::string name;
    Role role;
};

class UsersStorage final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "users-storage";

    UsersStorage(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {}
    
    std::optional<int> AddUser(const std::string& username,
                               const std::string& password,
                               const std::string& name,
                               Role role);

    std::optional<User> FindUser(const std::string& username,
                                 const std::string& password) const;

    std::optional<User> FindUserById(int id) const;

    void SaveToken(const std::string& token, int user_id);

    std::optional<int> FindUserByToken(const std::string& token) const;

private:
    std::unordered_map<int, User> users_;
    std::unordered_map<std::string, int> by_username_;
    std::unordered_map<std::string, int> tokens_;
    int next_id_ = 1;
};

}  // namespace user_service