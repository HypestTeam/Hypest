#include <config.hpp>
#include <games.hpp>
#include <user.hpp>
#include <requests.hpp>
#include <error.hpp>
#include <challonge.hpp>
#include <regex>
#include <fstream>
#include <algorithm>
#include <cassert>

namespace hypest {
std::string api_domain(const std::string& str) {
    static std::regex r(R"re((?:https?:\/\/)?(\w*?)?\.?challonge\.com\/(.+))re");
    std::smatch m;
    if(std::regex_match(str, m, r)) {
        auto&& subdomain = m[1].str();
        auto&& url = m[2].str();
        if(subdomain == "www") {
            return url;
        }
        subdomain.push_back('-');
        return subdomain + url;
    }
    return "";
}

template<typename Pair>
static requests::response get_response(const std::string& url, const Pair& params) {
    auto r = requests::get(url, params);
    if(r.status_code != 200) {
        auto notes = {
            "the URL attempted to connect to was",
            url.c_str(),
            "reported error by challonge has been dumped to errors.txt"
        };
        std::ofstream err("errors.txt");
        err << r.text;
        throw noted_error("unable to retrieve challonge bracket", notes);
    }
    return r;
}

json::object get_tournament(const json::object& conf, const std::string& url) {
    auto&& domain = api_domain(url);
    std::vector<std::pair<const char*, const char*>> params = {
        { "api_key", conf.at("challonge_api_key").as<const char*>() },
        { "include_matches", "1" }
    };
    auto api_url = "https://api.challonge.com/v1/tournaments/" + domain + ".json";
    auto r = get_response(api_url, params);
    json::value js;
    json::parse(r.text, js);
    return js["tournament"].as<json::object>();
}

json::array get_participants(const json::object& conf, const std::string& url) {
    auto&& domain = api_domain(url);
    std::initializer_list<std::pair<const char*, const char*>> params = {
        { "api_key", conf.at("challonge_api_key").as<const char*>() }
    };

    auto api_url = "https://api.challonge.com/v1/tournaments/" + domain + "/participants.json";
    auto r = get_response(api_url, params);
    json::value js;
    json::parse(r.text, js);
    return js.as<json::array>({});
}

static void verify_tournament(const json::object& tournament) {
    std::vector<std::string> notes;
    if(tournament.at("state").as<std::string>("") != "complete") {
        notes.emplace_back("tournament is not complete");
    }
    if(not notes.empty()) {
        throw noted_error("invalid tournament", notes, true);
    }
}

// represents the type that maps ids to username and other participant information
using mapping_t = std::map<int, std::pair<std::string, int>>;

static mapping_t get_mapping(const json::array& participants) {
    mapping_t mapping;
    for(auto&& p : participants) {
        auto&& participant = p["participant"].as<json::object>({});
        if(participant.empty()) {
            continue;
        }

        auto&& username = participant["challonge_username"];
        if(not username.is<std::string>()) {
            continue; // no challonge username? no ranking.
        }

        // empty challonge username?
        if(username.as<std::string>().empty()) {
            // no entry.
            continue;
        }

        mapping.emplace(participant["id"].as<int>(), std::make_pair(username.as<std::string>(), participant["final_rank"].as<int>(0)));
    }
    return mapping;
}

static user& retrieve_user(const std::string& username, users_t& users) {
    auto it = users.find(username);
    if(it == users.end()) {
        user entry;
        entry.name = username;
        auto pair = users.emplace(username, entry);
        assert(pair.second);
        return pair.first->second;
    }
    return it->second;
}

static std::vector<match> get_matches(const json::array& matches, users_t& users, const mapping_t& mapping) {
    std::vector<match> result;
    for(auto&& m : matches) {
        auto&& match = m["match"].as<json::object>({});
        auto p1_id = match["player1_id"].as<int>(0);
        auto p2_id = match["player2_id"].as<int>(0);
        auto&& p1_name = mapping.at(p1_id).first;
        auto&& p2_name = mapping.at(p2_id).first;
        auto& player1 = retrieve_user(p1_name, users);
        auto& player2 = retrieve_user(p2_name, users);
        ++player1.games_played;
        ++player2.games_played;
        player1.ranking.has_participated();
        player2.ranking.has_participated();

        // check for ties
        if(not match["winner_id"].is<int>()) {
            result.emplace_back(&player1.ranking, &player2.ranking, match::tie);
            result.emplace_back(&player2.ranking, &player1.ranking, match::tie);
            ++player1.ties;
            ++player2.ties;
            continue;
        }

        // check for winner
        int winner_id = match["match_id"].as<int>();
        if(winner_id == p1_id) {
            result.emplace_back(&player1.ranking, &player2.ranking, match::win);
            result.emplace_back(&player2.ranking, &player1.ranking, match::loss);
            ++player1.wins;
            ++player2.losses;
        }
        else {
            result.emplace_back(&player1.ranking, &player2.ranking, match::loss);
            result.emplace_back(&player2.ranking, &player1.ranking, match::win);
            ++player1.losses;
            ++player2.wins;
        }
    }

    // check for 'tournaments_won' statistic
    for(auto&& p : mapping) {
        auto& player = retrieve_user(p.second.first, users);
        if(p.second.second == 1) {
            ++player.tournaments_won;
        }
    }

    return result;
}

void update(const std::string& url) {
    auto conf = get_config();
    auto tournament = get_tournament(conf, url);
    verify_tournament(tournament);
    auto games_list = games();
    auto game_id = tournament["game_id"].as<int>(0);
    auto it = std::find_if(std::begin(games_list), std::end(games_list), [&game_id](const game& g) {
        return g.id == game_id;
    });

    if(it == std::end(games_list)) {
        throw fatal_error("game not supported: game id " + std::to_string(game_id));
    }
    auto users = get_users(*it);
    auto participants = get_participants(conf, url);
    auto mapping = get_mapping(participants);
    auto&& matches = get_matches(tournament["matches"].as<json::array>({}), users, mapping);

    for(auto&& u : users) {
        u.second.update(matches);
    }

    update_users(users, *it);
}
} // hypest
