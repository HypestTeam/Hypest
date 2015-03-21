#include <database.hpp>
#include <ranking.hpp>

namespace opt = gears::optparse;

int main(int argc, char** argv) {
    opt::option_parser parser;
    parser.description = "A program that handles a lot of Hypest affairs.";
    parser.add_subcommand(hypest::database());
    parser.add_subcommand(hypest::ranking());
    auto&& args = parser.parse(argv, argv + argc);

    if(argc < 2) {
        std::cout << parser.format_help();
        return 0;
    }

    try {
        if(args.subcommand == "database") {
            hypest::database(args);
        }
        else if(args.subcommand == "ranking") {
            hypest::ranking(args);
        }
    }
    catch(const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

}
