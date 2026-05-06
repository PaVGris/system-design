#pragma once

#include <userver/storages/redis/client.hpp>
#include <userver/storages/redis/command_control.hpp>
#include <userver/utils/datetime.hpp>
#include <string>
#include <chrono>

namespace user_service {

class RateLimiter {
public:
    RateLimiter(userver::storages::redis::ClientPtr redis_client,
                userver::storages::redis::CommandControl redis_cc)
        : redis_client_(std::move(redis_client)), redis_cc_(redis_cc) {}

    bool IsAllowed(const std::string& ip_address, int limit, int window_seconds) const {
        std::string key = "rate_limit:login:" + ip_address;
        
        auto req = redis_client_->Incr(key, redis_cc_);
        auto count = req.Get();
        
        if (count == 1) {
            redis_client_->Expire(key, std::chrono::seconds(window_seconds), redis_cc_).Get();
        }
        
        return count <= limit;
    }

private:
    userver::storages::redis::ClientPtr redis_client_;
    userver::storages::redis::CommandControl redis_cc_;
};

} // namespace user_service