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
        { "update", 'u', "updates the database with the given bracket", opt::value<std::string>("url") },
        { "with", 'w', "specifies the top cut bracket", opt::value<std::string>("url") },
        { "force", 'f', "force updating despite being the url(s) being processed already" }
    };

    opt::subcommand result = { "database", "handles the hypest database", args };
    result.description = "Updates, deletes, or otherwise fiddles with the Hypest database that keeps track of matches\n"
                         "\nIn order to update the database with a tournament with no top-cut then --update <url> is used.\n"
                         "However, if a top-cut is present then --update <url> --with <top-cut-url> should be used instead.\n"
                         "This makes sure that the rating period is fair for all players";
    return result;
}

void database(const opt::arguments& args) {
    auto&& opts = args.options;
    cache_t cache;
    cache_entry entry;
    bool forced = opts.is_active("force");
    bool updated = opts.is_active("update");
    bool with   = opts.is_active("with");
    if(not forced) {
        cache = get_cache();
    }

    if(with && not updated) {
        throw opt::error("hypest", "using '--with' without '--update' is prohibited", "");
    }

    if(updated) {
        auto notes = {
            "use --force to reprocess"
        };

        entry.url = opts.get<std::string>("update");
        if(std::binary_search(cache.begin(), cache.end(), entry)) {
            throw noted_error(entry.url + " is already in the cache", notes);
        }

        if(with) {
            entry.url = opts.get<std::string>("with");
            if(std::binary_search(cache.begin(), cache.end(), entry)) {
                throw noted_error(entry.url + " is already in the cache", notes);
            }
            update(opts.get<std::string>("update"), opts.get<std::string>("with"));
        }
        else {
            update(opts.get<std::string>("update"));
        }
    }

    if(not forced) {
        int id = static_cast<int>(cache.size() + 1);
        int next = static_cast<int>(cache.size() + 2);
        if(updated and not with) {
            cache.push_back({ opts.get<std::string>("update"), id, 0 });
        }
        else if(updated and with) {
            cache.push_back({ opts.get<std::string>("update"), id, next });
            cache.push_back({ opts.get<std::string>("with"), next, id });
        }
        update_cache(cache);
    }
}
} // hypest
