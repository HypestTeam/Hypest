#include <database.hpp>
#include <error.hpp>
#include <config.hpp>
#include <cache.hpp>
#include <challonge.hpp>
#include <algorithm>
#include <iostream>
#include <boost/filesystem.hpp>

namespace opt = gears::optparse;
namespace fs  = boost::filesystem;

namespace hypest {
opt::subcommand database() {
    opt::option_set args = {
        { "rank", 'r', "updates the rating period with the given bracket", opt::value<std::string>("url") },
        { "force", 'f', "force updating despite being the url(s) being processed already" },
        { "commit", 'c', "finalises a rating period and updates player's rankings" },
        { "rebuild", "rebuilds the database with the db.cache file" },
        { "verbose", "verbose output" },
        { "dump", "dumps the tournament response JSON to stdout and exits", opt::value<std::string>("url") }
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
    if(cache.current_rating_period < rating_period) {
        if(verbose) {
            std::cout << "Rating period (" << rating_period << ") has completed\n";
        }
        commit();
    }
}

void database(const opt::arguments& args) {
    auto&& opts = args.options;
    rank_cache cache = get_cache();
    bool forced = opts.is_active("force");
    bool verbose = opts.is_active("verbose");

    if(opts.is_active("dump")) {
        auto tournament = get_tournament(get_config(), opts.get<std::string>("dump"));
        json::dump(std::cout, tournament);
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
