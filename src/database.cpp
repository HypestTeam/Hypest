#include <database.hpp>
#include <error.hpp>
#include <config.hpp>
#include <cache.hpp>
#include <games.hpp>
#include <challonge.hpp>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <boost/filesystem.hpp>

namespace opt = gears::optparse;
namespace fs  = boost::filesystem;

namespace hypest {
struct username_mapping {
    std::string challonge;
    std::string reddit;
};

struct mapping_action {
    username_mapping operator()(const std::string&, const std::string& value) const {
        username_mapping result;
        auto&& pos = value.find(',');
        if(pos == value.npos) {
            throw std::runtime_error("invalid format given, must be challonge,reddit");
        }
        result.challonge = value.substr(0, pos);
        result.reddit = value.substr(pos + 1);
        return result;
    }
};

opt::subcommand database() {
    opt::option_set args = {
        { "rank", 'r', "updates the rating period with the given bracket", opt::value<std::string>("url") },
        { "force", 'f', "force updating despite being the url(s) being processed already" },
        { "commit", 'c', "finalises a rating period and updates player's rankings" },
        { "rebuild", "rebuilds the database with the db.cache file" },
        { "verbose", "verbose output" },
        { "dump", "dumps the tournament response JSON to stdout and exits", opt::value<std::string>("url") },
        { "reddit-mapping", "updates the reddit username mapping with a .json file", opt::value<std::string>("file") },
        { "reddit", "updates the reddit username of a challonge username (e.g. \"challonge,reddit\")", opt::custom<username_mapping>(mapping_action{}) }
    };

    opt::subcommand result = { "database", "handles the hypest database", args };
    result.description = "Updates, deletes, or otherwise fiddles with the Hypest database that keeps track of matches\n"
                         "\nIn order to add things to the rating period you must use --rank <url> for every bracket."
                         "Once the rating period is seemingly complete, use --commit to finalise the rating period";
    return result;
}

void rebuild_database(const rank_cache& cache, bool verbose) {
    if(fs::exists("rating_period.cache")) {
        std::cerr << "hypest: warning: rating period is currently in place\n";
    }

    // clear the databases
    fs::remove_all("database");

    int rating_period = 1;
    for(auto&& entry : cache) {
        if(entry.rating_period != rating_period) {
            if(verbose) {
                std::cout << "Rating period (" << rating_period << ") has completed\n";
            }
            commit();
            ++rating_period;
        }
        if(verbose) {
            std::cout << "Processing " << entry.url << '\n';
        }
        rank(entry.url);
    }
    if(cache.current_rating_period > rating_period) {
        if(verbose) {
            std::cout << "Rating period (" << rating_period << ") has completed\n";
        }
        commit();
    }
}

void reddit_mapping(const std::string& filename) {
    // parse the JSON file
    json::value v;
    std::ifstream in(filename);
    json::parse(in, v);
    if(not v.is<json::object>()) {
        throw fatal_error("reddit mapping JSON must be an object of key value pairs of challonge:reddit");
    }

    auto&& obj = v.as<json::object>();
    auto smash_games = games();
    for(auto&& smash : smash_games) {
        auto users = get_users(smash);
        for(auto&& u : users) {
            auto username = u.first;
            std::transform(username.begin(), username.end(), username.begin(), [](char c) { return std::tolower(c); });
            auto it = obj.find(username);
            if(it != obj.end() && it->second.is<std::string>()) {
                u.second.reddit = it->second.as<std::string>();
            }
        }
        update_users(users, smash);
    }
}

void update_reddit(const username_mapping& mapping) {
    auto smash_games = games();
    for(auto&& smash : smash_games) {
        auto users = get_users(smash);
        auto it = users.find(mapping.challonge);
        if(it != users.end()) {
            it->second.reddit = mapping.reddit;
        }
        update_users(users, smash);
    }
}

void database(const opt::arguments& args) {
    auto&& opts = args.options;
    rank_cache cache = get_cache();
    bool forced = opts.is_active("force");
    bool verbose = opts.is_active("verbose");

    if(opts.is_active("dump")) {
        auto tournament = get_tournament(get_config(), opts.get<std::string>("dump"));
        json::format_options opt;
        opt.precision = 15;
        json::dump(std::cout, tournament, opt);
        return;
    }

    if(opts.is_active("rank")) {
        auto&& url = opts.get<std::string>("rank");
        if(not forced) {
            if(cache.in(url)) {
                auto notes = {
                    "use --force to reprocess"
                };
                throw noted_error("bracket has already been processed", notes);
            }
            cache.add(url);
        }
        if(verbose) {
            std::cout << "Processing " << url << '\n';
        }
        rank(url);
    }

    if(opts.is_active("reddit")) {
        update_reddit(opts.get<username_mapping>("reddit"));
    }

    if(opts.is_active("reddit-mapping")) {
        reddit_mapping(opts.get<std::string>("reddit-mapping"));
    }

    if(opts.is_active("rebuild")) {
        // sort database by rating period
        std::sort(std::begin(cache), std::end(cache), [](const auto& lhs, const auto& rhs) {
            return lhs.rating_period < rhs.rating_period;
        });
        rebuild_database(cache, verbose);
        return;
    }

    if(opts.is_active("commit")) {
        commit();
    }

    if(not forced) {
        if(opts.is_active("commit")) {
            ++cache.current_rating_period;
        }
        if(verbose) {
            std::cout << "Updating cache...\n";
        }
        update_cache(cache);
    }
}
} // hypest
