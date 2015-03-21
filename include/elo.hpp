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

#ifndef HYPEST_ELO_HPP
#define HYPEST_ELO_HPP

#include <cmath>
#include <cstddef>
#include <numeric>

namespace hypest {
constexpr double initial_rating = 1200.0;

struct elo {
public:
    elo() noexcept = default;
    elo(double r): r(r) {}

    int rating() const noexcept {
        return static_cast<int>(std::round(r + participation_bonus()));
    }

    void rating(double d) noexcept {
        r = d - participation_bonus();
    }

    int tournament_miss_streak() const noexcept {
        return t;
    }

    void tournament_miss_streak(int arg) noexcept {
        t = arg;
    }

    template<typename Matches>
    void update(const Matches& matches) noexcept {
        if(matches.empty()) {
            ++t;
            return;
        }
        t = 1;
        for(auto&& m : matches) {
            r += k_factor() * (m.score() - expected_win_rate(m.opponent.r));
        }
    }
private:
    // the decay constant specifies how much decay occurs if missing a rating period
    static constexpr double decay = 15.0;

    int k_factor() const noexcept {
        auto floored_rating = static_cast<int>(r);
        if(floored_rating < 2100) {
            return 32;
        }
        else if(floored_rating >= 2100 && floored_rating <= 2400) {
            return 24;
        }
        else {
            return 16;
        }
    }

    double participation_bonus() const noexcept {
        return t == 1 ? decay : - t * decay;
    }

    double expected_win_rate(double opponent_rating) const noexcept {
        return 1 / (1 + std::pow(10, (opponent_rating - r) / 400.0));
    }

    double r = initial_rating;
    int t = 1;
    bool participating = false;
};

struct match {
    enum : int {
        win, loss, tie
    };

    elo opponent;
    int result;

    match(elo opponent, int result) noexcept: opponent(opponent), result(result) {}

    double score() const noexcept {
        switch(result) {
        case match::win:
            return 1.0;
        default:
            return 0.0;
        }
    }
};
} // hypest

#endif // HYPEST_ELO_HPP
