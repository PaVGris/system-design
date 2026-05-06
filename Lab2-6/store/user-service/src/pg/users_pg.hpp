#pragma once
#include <string>
#include <optional>
#include <userver/components/component_base.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/redis/client.hpp>
#include <userver/storages/redis/component.hpp>

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
                const userver::components::ComponentContext& context);

    std::optional<int> AddUser(const std::string& username,
                               const std::string& password,
                               const std::string& name,
                               Role role);

    std::optional<User> FindUser(const std::string& username,
                                 const std::string& password) const;

    std::optional<User> FindUserById(int id) const;

    void SaveToken(const std::string& token, int user_id);

    std::optional<int> FindUserByToken(const std::string& token) const;

    userver::storages::redis::ClientPtr GetRedisClient() const;
    userver::storages::redis::CommandControl GetRedisCC() const;
private:
    userver::storages::postgres::ClusterPtr postgres_;
    userver::storages::redis::ClientPtr redis_client_;
    userver::storages::redis::CommandControl redis_cc_;

    static constexpr int kTokenTtlSeconds = 3600;
    static std::string MakeTokenKey(const std::string& token);
};

}  // namespace user_service