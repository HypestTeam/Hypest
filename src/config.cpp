#include <config.hpp>
#include <error.hpp>
#include <fstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace hypest {
static json::object load_obj(const char* filename) {
    std::ifstream in(filename);
    if(!in.good()) {
        // file does not exist.
        in.close();
        // so just return nothing
        return {};
    }
    json::value v;
    json::parse(in, v);
    if(!v.is<json::object>()) {
        // not an object..
        return {};
    }
    return v.as<json::object>();
}

json::object get_config() {
    auto&& result = load_obj("config.json");
    if(not result["challonge_api_key"].is<std::string>() or not result["challonge_username"].is<std::string>()) {
        auto notes = {
            "challonge_username and challonge_api_key are required",
            "challonge_username and challonge_api_key must be of type string",
            "config.json might be missing"
        };
        throw noted_error("invalid configuration", notes, true);
    }
    return result;
}

void update_config(const json::object& obj) {
    std::ofstream out("config.json");
    json::dump(out, obj);
}

users_t get_users(const game& g) {
    fs::create_directory("database");
    auto&& path = "database/" + g.filename;
    auto&& obj = load_obj(path.c_str());
    if(obj.empty()) {
        return {};
    }
    users_t result;
    for(auto&& p : obj) {
        result.emplace(p.first, user_from_json(p.second));
    }
    return result;
}

void update_users(const users_t& u, const game& g) {
    auto&& path = "database/" + g.filename;
    std::ofstream out(path.c_str());
    if(not out.is_open()) {
        throw fatal_error("unable to load " + g.filename);
    }
    json::format_options opt;
    opt.precision = 15; // 15 digits for double
    json::dump(out, u, opt);
}
} // hypest
