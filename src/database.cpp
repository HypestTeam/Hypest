#include <database.hpp>
#include <challonge.hpp>
#include <iostream>

namespace opt = gears::optparse;

namespace hypest {
opt::subcommand database() {
    opt::option_set args = {
        { "update", 'u', "updates the database with the given bracket", opt::value<std::string>("bracket") }
    };

    opt::subcommand result = { "database", "handles the hypest database", args };
    result.description = "Updates, deletes, or otherwise fiddles with the Hypest database that keeps track of matches";
    return result;
}

void database(const opt::arguments& args) {
    if(args.options.is_active("update")) {
        update(args.options.get<std::string>("update"));
    }
}
} // hypest
