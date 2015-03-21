// The MIT License (MIT)

// Copyright (c) 2015 Danny "Rapptz" Y.

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef HYPEST_USER_HPP
#define HYPEST_USER_HPP

#include <elo.hpp>
#include <string>
#include <jsonpp.hpp>

namespace hypest {
struct user {
    elo ranking;
    std::string name = "#";
    std::string reddit;
    int wins = 0;
    int losses = 0;
    int ties = 0;
    int tournaments_won = 0;
    int games_played = 0;
    int times_participated = 0;

    double win_loss_ratio() const noexcept {
        if(losses == 0) {
            return wins;
        }
        return static_cast<double>(wins) / losses;
    }

    template<typename Matches>
    void update(const Matches& matches) noexcept {
        ranking.update(matches);

        // get the proper ranking floor:
        // AF = 100 + 4 * wins + 2 * draws + times_participated
        int floor = 100 + 4 * wins + 2 * ties + times_participated;
        if(ranking.rating() < floor) {
            ranking.rating(floor);
        }
    }
};

inline json::value to_json(const user& u) {
    return json::object{
        { "wins", u.wins },
        { "losses", u.losses },
        { "ties", u.ties },
        { "tournaments_won", u.tournaments_won },
        { "games_played", u.games_played },
        { "times_participated", u.times_participated },
        { "challonge_username", u.name },
        { "reddit_username", u.reddit },
        { "rating", u.ranking.rating() },
        { "tournament_miss_streak", u.ranking.tournament_miss_streak() }
    };
}

inline user user_from_json(const json::value& v) {
    if(!v.is<json::object>()) {
        return {};
    }
    auto&& obj = v.as<json::object>();
    user result;
    result.ranking.tournament_miss_streak(obj["tournament_miss_streak"].as<int>(1));
    result.ranking.rating(obj["rating"].as<int>(initial_rating));
    result.wins = obj["wins"].as<int>(0);
    result.losses = obj["losses"].as<int>(0);
    result.ties = obj["ties"].as<int>(0);
    result.times_participated = obj["times_participated"].as<int>(0);
    result.games_played = obj["games_played"].as<int>(0);
    result.tournaments_won = obj["tournaments_won"].as<int>(0);
    result.name = obj["challonge_username"].as<std::string>("");
    result.reddit = obj["reddit_username"].as<std::string>("unknown");
    return result;
}
} // hypest

#endif // HYPEST_USER_HPP
