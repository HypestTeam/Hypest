#include <database.hpp>
#include <challonge.hpp>
#include <iostream>

namespace opt = gears::optparse;

namespace hypest {
opt::subcommand database() {
    opt::option_set args = {
        { "update", 'u', "updates the database with the given bracket", opt::value<std::string>("bracket") },
        { "with", 'w', "updates the database with the bracket but assumes this is a top cut", opt::value<std::string>("bracket") },
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
    if(opts.is_active("with") && not opts.is_active("update")) {
        throw opt::error("hypest", "using '--with' without '--update' is prohibited", "");
    }

    if(opts.is_active("update")) {
        if(opts.is_active("with")) {
            update(opts.get<std::string>("update"), opts.get<std::string>("with"));
        }
        else {
            update(opts.get<std::string>("update"));
        }
    }
}
} // hypest
