#include <ranking.hpp>
#include <games.hpp>
#include <config.hpp>
#include <error.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <ctime>

namespace opt = gears::optparse;
namespace fs  = boost::filesystem;

namespace hypest {
opt::subcommand ranking() {
    opt::option_set args = {
        { "game", 'g', "specify what game to dump rankings for", opt::value<std::string>("filename") },
        { "all", 'a', "dumps all game rankings" },
        { "verbose", "shows verbose output" }
    };

    opt::subcommand result = { "ranking", "handles the reddit rankings", args };
    result.description = "Dumps the rankings and stats of players for specific games.\n"
                         "This is dependent on the results of the database subcommand. When using --game you should "
                         "specify the game filename for the database. It will be the same filename as the one found "
                         "in the resulting subdirectory for the ranking markdown files.";

    return result;
}

void show_ranks(const game& smash) {
    fs::path filename = smash.filename;
    fs::path db = "database" / filename;
    filename.replace_extension(".md");
    fs::path p  = "ranking" / filename;
    if(not fs::exists(db)) {
        throw fatal_error("unable to find database file: " + db.string());
    }

    fs::create_directory("ranking");
    std::ofstream out(p.string());
    auto&& users = get_users(smash);
    std::vector<user> players;
    std::transform(users.begin(), users.end(), std::back_inserter(players), [](const auto& p) {
        return p.second;
    });

    std::sort(players.begin(), players.end(), [](const user& lhs, const user& rhs) {
        return rhs.ranking.rating() < lhs.ranking.rating();
    });

    // remove players who haven't participated in more than 8 weeks.
    players.erase(std::remove_if(players.begin(), players.end(), [](const user& player) {
        return player.ranking.tournament_miss_streak() >= 8;
    }), players.end());

    std::time_t time = std::time(nullptr);
    std::tm* today = std::gmtime(&time);
    char today_str[100];
    std::strftime(today_str, sizeof(today_str), "%A, %B %d, %Y %I:%M:%S %p UTC", today);

    out << "### Welcome to the rankings for " + smash.name << "\n\n";
    out << "Welcome to the /r/smashbros ranking board! Here, we chronicle the top players of each tournament the"
           " subreddit hosts, and rank them, which in turn give them a chance to rise to the top!\n"
           "Note that only the top 100 are shown."
           "This page doesn't explain how the rankings work, for that you'll need to head over to the "
           "[Q&A](https://www.reddit.com/r/smashbros/wiki/rankings_qa)\n\n"
           "**Please note that if your reddit username shows up as \"Unknown\" or there is a mistake "
           "then you should PM /u/Rapptz your challonge username and he will fix it when the ranking page updates**\n\n";

    out << "Last Updated: " << today_str << "\n\n";
    out << "Place|Challonge|Reddit|Ranking|Wins|Losses|W/L Ratio|T. Wins\n"
           ":----|:--------|:-----|:------|:---|:-----|:--------|:------\n";
    using size_type = decltype(players.size());
    auto limit = std::min(static_cast<size_type>(100), players.size());
    for(size_type i = 0; i < limit; ++i) {
        auto& player = players[i];
        std::string reddit = player.reddit.empty() ? "**Unknown**" : "/u/" + player.reddit;
        out << i + 1 << '|' << '[' << player.name << "](http://www.challonge.com/users/" << player.name << ')' << '|'
            << reddit << '|' << player.ranking.rating() << '|'
            << player.wins << '|' << player.losses << '|'
            << player.win_loss_ratio() << '|' << player.tournaments_won << '\n';
    }

    out << '\n';
}

void ranking(const opt::arguments& args) {
    auto&& opts = args.options;
    auto&& smash_games = games();
    bool verbose = opts.is_active("verbose");

    if(opts.is_active("game")) {
        auto str = opts.get<std::string>("game");
        auto it = std::find_if(smash_games.begin(), smash_games.end(), [&str](const auto& g) {
            return g.filename == str;
        });
        if(it == smash_games.end()) {
            throw fatal_error("game filename not found: " + str);
        }
        if(verbose) {
            std::cout << "Processing " << it->name << " rankings...\n";
        }
        show_ranks(*it);
    }

    if(opts.is_active("all")) {
        for(auto&& smash : smash_games) {
            if(verbose) {
                std::cout << "Processing " << smash.name << " rankings...\n";
            }
            show_ranks(smash);
        }
    }
}
} // hypest
