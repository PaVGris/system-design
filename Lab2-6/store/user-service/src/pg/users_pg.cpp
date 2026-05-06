#include "users_pg.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/logging/log.hpp>

namespace user_service {

UsersStorage::UsersStorage(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
        postgres_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()),
        redis_client_(
            context.FindComponent<userver::components::Redis>("token-cache")
            .GetClient("token-cache")
        ),
        redis_cc_{std::chrono::seconds{5}, std::chrono::seconds{15}, 3} {}

std::optional<int> UsersStorage::AddUser(const std::string& username,
                                         const std::string& password,
                                         const std::string& name,
                                         Role role) {
    try {
        const std::string role_str = (role == Role::Seller) ? "seller" : "client";

        auto result = postgres_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "INSERT INTO users (username, password, name, role) "
            "VALUES ($1, $2, $3, $4) RETURNING id",
            username, password, name, role_str
        );

        return result.AsSingleRow<int>();
        
    } catch (const userver::storages::postgres::UniqueViolation&) {
        return std::nullopt;  
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<User> UsersStorage::FindUser(const std::string& username,
                                           const std::string& password) const {
    try {
        auto result = postgres_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "SELECT id, username, password, name, role FROM users "
            "WHERE username = $1 AND password = $2",
            username, password
        );

        if (result.IsEmpty()) {
            return std::nullopt;
        }

        auto row = result[0];
        std::string role_str = row["role"].As<std::string>();
        Role role = (role_str == "seller") ? Role::Seller : Role::Client;

        return User{
            row["id"].As<int>(),
            row["username"].As<std::string>(),
            row["password"].As<std::string>(),
            row["name"].As<std::string>(),
            role
        };
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<User> UsersStorage::FindUserById(int id) const {
    try {
        auto result = postgres_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "SELECT id, username, password, name, role FROM users WHERE id = $1",
            id
        );

        if (result.IsEmpty()) {
            return std::nullopt;
        }

        auto row = result[0];
        std::string role_str = row["role"].As<std::string>();
        Role role = (role_str == "seller") ? Role::Seller : Role::Client;

        return User{
            row["id"].As<int>(),
            row["username"].As<std::string>(),
            row["password"].As<std::string>(),
            row["name"].As<std::string>(),
            role
        };
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

void UsersStorage::SaveToken(const std::string& token, int user_id) {
    try {
        postgres_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "INSERT INTO tokens (token, user_id) VALUES ($1, $2) "
            "ON CONFLICT (token) DO UPDATE SET user_id = $2",
            token, user_id
        );
    } catch (const std::exception& e) {
        LOG_ERROR() << "Failed to save token to Postgres: " << e.what();
    }

    try {
        const auto key = "token:" + token;
        const auto value = std::to_string(user_id);
        redis_client_->Set(key, value, redis_cc_).Get();
        redis_client_->Expire(key,
            std::chrono::seconds{kTokenTtlSeconds},
            redis_cc_)
            .Get();
        LOG_DEBUG() << "Cached token in Redis for user_id=" << user_id;
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to cache token in Redis: " << e.what();
    }
}

std::optional<int> UsersStorage::FindUserByToken(const std::string& token) const {
    try {
        const auto key = "token:" + token;
        auto cached = redis_client_->Get(key, redis_cc_).Get();
        if (cached) {
            LOG_DEBUG() << "Token cache HIT for key=" << key;
            return std::stoi(*cached);
        }
        LOG_DEBUG() << "Token cache miss in Redis for key=" << key;
    } catch (std::exception& e) {
        LOG_WARNING() << "Failed to access Redis, falling back to Postgres: " << e.what();
    }


    try {
        auto result = postgres_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "SELECT user_id FROM tokens WHERE token = $1",
            token
        );

        if (result.IsEmpty()) {
            return std::nullopt;
        }

        const int user_id = result[0]["user_id"].As<int>();
        try {
            const auto key = "token:" + token;
            redis_client_->Set(key, std::to_string(user_id), redis_cc_).Get();
            redis_client_->Expire(key, std::chrono::seconds{kTokenTtlSeconds}, redis_cc_).Get();
        } catch (const std::exception& e) {
            LOG_WARNING() << "Failed to warm-up token cache: " << e.what();
        }

        return user_id;
        
    } catch (const std::exception&) {
        return std::nullopt;
    }


}

userver::storages::redis::ClientPtr UsersStorage::GetRedisClient() const { 
        return redis_client_; 
}
    
userver::storages::redis::CommandControl UsersStorage::GetRedisCC() const { 
        return redis_cc_; 
}

}  // namespace user_service
