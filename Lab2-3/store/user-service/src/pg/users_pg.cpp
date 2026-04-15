#include "users_pg.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace user_service {

UsersStorage::UsersStorage(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
        postgres_(context.FindComponent<userver::components::Postgres>("postgres-db")
          .GetCluster()) {}

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
    } catch (const std::exception&) {
        // Fail silently, token may not be saved
    }
}

std::optional<int> UsersStorage::FindUserByToken(const std::string& token) const {
    try {
        auto result = postgres_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "SELECT user_id FROM tokens WHERE token = $1",
            token
        );

        if (result.IsEmpty()) {
            return std::nullopt;
        }

        return result[0]["user_id"].As<int>();
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

}  // namespace user_service
