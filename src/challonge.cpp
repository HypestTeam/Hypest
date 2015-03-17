#include <config.hpp>
#include <games.hpp>
#include <user.hpp>
#include <requests.hpp>
#include <cstdio>
#include <error.hpp>
#include <challonge.hpp>
#include <regex>
#include <iomanip>
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
        if(subdomain == "www" || subdomain.empty()) {
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
        { "include_matches", "1" },
        { "include_participants", "1" }
    };
    auto api_url = "https://api.challonge.com/v1/tournaments/" + domain + ".json";
    auto r = get_response(api_url, params);
    json::value js;
    json::parse(r.text, js);
    return js["tournament"].as<json::object>();
}

static void verify_tournament(const json::object& tournament) {
    std::vector<std::string> notes;
    if(tournament.at("state").as<std::string>("") != "complete") {
        notes.emplace_back("tournament is not complete");
    }
    if(not notes.empty()) {
        notes.emplace_back("url is: " + tournament.at("full_challonge_url").as<std::string>(""));
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
        int id = participant["id"].as<int>();
        int final_rank = participant["final_rank"].as<int>(0);
        std::string name;

        // if someone does not have a challonge username then
        // they are assigned a "guest" name of "#"
        // these guests do not have anything special to them outside of
        // being an intermediate format to make it valid.
        auto&& username = participant["challonge_username"];
        if(not username.is<std::string>()) {
            name = "#";
        }
        else {
            name = username.as<std::string>();
            if(name.empty()) {
                name = "#";
            }
        }

        mapping.emplace(id, std::make_pair(name, final_rank));
    }
    return mapping;
}

static user guest;

static user& retrieve_user(const std::string& username, users_t& users) {
    if(username == "#") {
        return guest;
    }
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

struct game_match {
    std::string player1;
    std::string player2;
    std::string game;
    int result;
};

std::istream& operator>>(std::istream& in, game_match& r) {
    return in >> std::quoted(r.player1) >> std::quoted(r.player2) >> r.result >> std::quoted(r.game);
}

std::ostream& operator<<(std::ostream& out, const game_match& r) {
    return out << std::quoted(r.player1) << ' ' << std::quoted(r.player2) << ' ' << r.result << ' '<< std::quoted(r.game);
}

static void update_rating_period(const json::array& matches, users_t& users, const mapping_t& mapping, const game& g) {
    std::ofstream rating("rating_period.cache", std::ofstream::app | std::ofstream::out);
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

        // check for ties
        if(not match["winner_id"].is<int>()) {
            rating << game_match{p1_name, p2_name, g.filename, match::tie} << '\n';
            rating << game_match{p2_name, p1_name, g.filename, match::tie} << '\n';
            ++player1.ties;
            ++player2.ties;
            continue;
        }

        // check for winner
        int winner_id = match["winner_id"].as<int>();
        if(winner_id == p1_id) {
            rating << game_match{p1_name, p2_name, g.filename, match::win} << '\n';
            rating << game_match{p2_name, p1_name, g.filename, match::loss} << '\n';
            ++player1.wins;
            ++player2.losses;
        }
        else {
            rating << game_match{p1_name, p2_name, g.filename, match::loss} << '\n';
            rating << game_match{p2_name, p1_name, g.filename, match::win} << '\n';
            ++player1.losses;
            ++player2.wins;
        }
    }
}

static std::vector<game_match> get_matches() {
    std::ifstream in("rating_period.cache");
    std::vector<game_match> matches;
    game_match match;
    while(in >> match) {
        matches.push_back(match);
    }
    return matches;
}

static game get_game(json::object& tournament) {
    auto&& list = games();
    auto game_id = tournament["game_id"].as<int>(0);
    auto it = std::find_if(std::begin(list), std::end(list), [&game_id](const game& g) {
        return g.id == game_id;
    });

    if(it == std::end(list)) {
        throw fatal_error("game not supported: game id " + std::to_string(game_id));
    }
    return *it;
}

void rank(const std::string& url) {
    auto conf = get_config();
    auto tournament = get_tournament(conf, url);
    verify_tournament(tournament);
    auto smash_game = get_game(tournament);
    auto users = get_users(smash_game);
    auto mapping = get_mapping(tournament["participants"].as<json::array>({}));
    update_rating_period(tournament["matches"].as<json::array>({}), users, mapping, smash_game);

    for(auto&& p : mapping) {
        auto& player = retrieve_user(p.second.first, users);
        if(p.second.second == 1) {
            ++player.tournaments_won;
        }
        ++player.times_participated;
    }

    update_users(users, smash_game);
}

struct match_compare {
    bool operator()(const std::string& filename, const game_match& g) const noexcept {
        return filename < g.game;
    }

    bool operator()(const game_match& g, const std::string& filename) const noexcept {
        return g.game < filename;
    }

    bool operator()(const game_match& lhs, const game_match& rhs) const noexcept {
        return lhs.game < rhs.game;
    }
};

void commit() {
    auto smash_games = games();
    auto matches = get_matches();
    // sort by game type
    std::sort(begin(matches), end(matches), match_compare());

    for(auto&& smash : smash_games) {
        auto p = std::equal_range(begin(matches), end(matches), smash.filename, match_compare());
        if(p.first == p.second) {
            continue;
        }
        auto users = get_users(smash);

        // group by player name
        std::map<std::string, std::vector<match>> grouped_matches;
        for(; p.first != p.second; ++p.first) {
            auto&& m = *p.first;
            auto& p2 = retrieve_user(m.player2, users);
            grouped_matches[m.player1].emplace_back(p2.ranking, m.result);
        }

        for(auto&& u : users) {
            auto& player = u.second;
            auto it = grouped_matches.find(player.name);
            if(it == grouped_matches.end()) {
                // did not participate so..
                player.ranking.update(std::vector<match>{});
                continue;
            }
            player.ranking.has_participated();
            player.ranking.update(it->second);
        }

        update_users(users, smash);
    }

    std::remove("rating_period.cache");
}
} // hypest
