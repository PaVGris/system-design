#include <storage/users_storage.hpp>
#include <random>
#include <sstream>
#include <iomanip>

namespace user_service {

std::optional<int> UsersStorage::AddUser(const std::string& username,
                                         const std::string& password,
                                         const std::string& name,
                                         Role role) 
{
    if (by_username_.count(username)) {
        return std::nullopt;
    } 
                            
    int id = next_id_++;
    users_[id] = User{id, username, password, name, role};
    by_username_[username] = id;
    return id;
}

std::optional<User> UsersStorage::FindUser(const std::string& username,
                                           const std::string& password) const 
{
    auto it = by_username_.find(username);
    if (it == by_username_.end()) {
        return std::nullopt;
    }

    const User& user = users_.at(it->second);
    if (user.password != password) {
        return std::nullopt;
    }

    return user;
}

std::optional<User> UsersStorage::FindUserById(int id) const 
{
    auto it = users_.find(id);
    if (it == users_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void UsersStorage::SaveToken(const std::string& token, int user_id) 
{
    tokens_[token] = user_id;
}

std::optional<int> UsersStorage::FindUserByToken(const std::string& token) const
{
    auto it = tokens_.find(token);
    if (it == tokens_.end()) {
        return std::nullopt;
    }

    return it->second;
}

}  // namespace user_service