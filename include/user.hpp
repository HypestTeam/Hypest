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

#include <glicko2.hpp>
#include <string>
#include <jsonpp.hpp>

namespace hypest {
struct user {
    glicko ranking;
    std::string name = "#";
    std::string reddit;
    int wins = 0;
    int losses = 0;
    int ties = 0;
    int tournaments_won = 0;
    int games_played = 0;
    int times_participated = 0;

    void update(const std::vector<match>& matches) {
        if(ranking.is_participating()) {
            ++times_participated;
        }
        // the win/loss/tie/games_played statistic is updated while retrieving these matches
        ranking.update(matches);
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
        { "rating", u.ranking.rating<int>() },
        { "rd", u.ranking.rd() },
        { "volatility", u.ranking.volatility() },
        // 95% confidence interval
        { "bottom_interval", static_cast<int>(u.ranking.rating() - 2 * u.ranking.rd()) },
        { "top_interval", static_cast<int>(u.ranking.rating() + 2 * u.ranking.rd()) }
    };
}

inline user user_from_json(const json::value& v) {
    if(!v.is<json::object>()) {
        return {};
    }
    auto&& obj = v.as<json::object>();
    user result;
    result.ranking.rd(obj["rd"].as<double>(350));
    result.ranking.rating(obj["rating"].as<int>(1500));
    result.ranking.volatility(obj["volatility"].as<double>(0.06));
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
