#include <database.hpp>
#include <error.hpp>
#include <cache.hpp>
#include <challonge.hpp>
#include <algorithm>
#include <iostream>

namespace opt = gears::optparse;

namespace hypest {
opt::subcommand database() {
    opt::option_set args = {
        { "rank", 'r', "updates the rating period with the given bracket", opt::value<std::string>("url") },
        { "force", 'f', "force updating despite being the url(s) being processed already" },
        { "commit", 'c', "finalises a rating period and updates player's rankings" }
    };

    opt::subcommand result = { "database", "handles the hypest database", args };
    result.description = "Updates, deletes, or otherwise fiddles with the Hypest database that keeps track of matches\n"
                         "\nIn order to add things to the rating period you must use --rank <url> for every bracket."
                         "Once the rating period is seemingly complete, use --commit to finalise the rating period";
    return result;
}

void database(const opt::arguments& args) {
    auto&& opts = args.options;
    rank_cache cache = get_cache();
    bool forced = opts.is_active("force");
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
        rank(url);
    }

    if(opts.is_active("commit")) {
        commit();
    }

    if(not forced) {
        if(opts.is_active("commit")) {
            ++cache.current_rating_period;
        }
        update_cache(cache);
    }
}
} // hypest
