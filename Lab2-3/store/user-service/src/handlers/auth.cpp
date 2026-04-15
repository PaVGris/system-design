#include <handlers/auth.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/components/component_context.hpp>

#include <schemas/user.hpp>
#include <random>
#include <sstream>
#include <iomanip>

namespace user_service {

namespace {
std::string GenerateToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    return oss.str();
}
}

RegisterHandler::RegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<UsersStorage>()) {}

std::string RegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto body = userver::formats::json::FromString(request.RequestBody());
    auto req = body.As<RegisterRequest>();

    Role role = (req.role == "seller") ? Role::Seller : Role::Client;

    auto id = storage_.AddUser(req.username, req.password, req.name, role);
    if (!id) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Username already taken"});
    }

    userver::formats::json::ValueBuilder result;
    result["id"] = *id;
    result["username"] = req.username;

    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kCreated);
    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(result.ExtractValue());
}

LoginHandler::LoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<UsersStorage>()) {}

std::string LoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    auto body = userver::formats::json::FromString(request.RequestBody());
    auto req = body.As<LoginRequest>();

    auto user = storage_.FindUser(req.username, req.password);
    if (!user) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Invalid username or password"});
    }

    const auto token = GenerateToken();
    storage_.SaveToken(token, user->id);

    userver::formats::json::ValueBuilder result;
    result["token"] = token;
    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(result.ExtractValue());
}

GetUserHandler::GetUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<UsersStorage>()) {}

std::string GetUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& token = request.GetHeader("Authorization");
    if (token.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing token"});
    }

    const std::string bearer_prefix = "Bearer ";
    if (token.substr(0, bearer_prefix.size()) != bearer_prefix) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Invalid token format"});
    }
    const auto actual_token = token.substr(bearer_prefix.size());

    if (!storage_.FindUserByToken(actual_token)) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Invalid token"});
    }

    const auto& id_str = request.GetPathArg("id");
    int id = std::stoi(id_str);

    auto user = storage_.FindUserById(id);
    if (!user) {
        throw userver::server::handlers::ResourceNotFound();
    }

    userver::formats::json::ValueBuilder result;
    result["id"] = user->id;
    result["username"] = user->username;
    result["name"] = user->name;
    result["role"] = (user->role == Role::Seller) ? "seller" : "client";

    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(result.ExtractValue());
}

ValidateTokenHandler::ValidateTokenHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<UsersStorage>()) {}

std::string ValidateTokenHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& token = request.GetHeader("Authorization");
    if (token.empty()) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Missing token"});
    }

    const std::string bearer_prefix = "Bearer ";
    if (token.substr(0, bearer_prefix.size()) != bearer_prefix) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Invalid token format"});
    }
    const auto actual_token = token.substr(bearer_prefix.size());

    auto user_id = storage_.FindUserByToken(actual_token);
    if (!user_id) {
        throw userver::server::handlers::Unauthorized(
            userver::server::handlers::ExternalBody{"Invalid token"});
    }

    auto user = storage_.FindUserById(*user_id);
    
    userver::formats::json::ValueBuilder result;
    result["user_id"] = *user_id;
    result["valid"] = true;
    result["role"] = (user->role == Role::Seller) ? "seller" : "client";

    request.GetHttpResponse().SetContentType("application/json");
    return userver::formats::json::ToString(result.ExtractValue());
}

}  // namespace user_service