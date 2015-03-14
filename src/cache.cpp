#include <error.hpp>
#include <cache.hpp>
#include <fstream>
#include <cstdint>
#include <algorithm>

namespace hypest {
rank_cache get_cache() {
    std::ifstream in("db.cache");
    if(not in.good()) {
        in.close();
        return {};
    }

    // file format:
    // header: number of entries current_rating_period
    // entry: id url rating_period
    // repeat..

    uint32_t length;
    rank_cache result;
    if(not (in >> length >> result.current_rating_period)) {
        throw fatal_error("invalid cache format: no number of entries or current rating period found");
    }

    for(uint32_t i = 0; i < length; ++i) {
        cache_entry entry;
        if((in >> entry.id >> entry.url >> entry.rating_period).fail()) {
            throw fatal_error("invalid cache format: incorrect entry (must be 'id url rating_period')");
        }
        result.entries.push_back(entry);
    }

    std::sort(std::begin(result), std::end(result));
    return result;
}

struct comparator {
    bool operator()(const cache_entry& lhs, const std::string& url) const noexcept {
        return lhs.url < url;
    }

    bool operator()(const std::string& url, const cache_entry& rhs) const noexcept {
        return url < rhs.url;
    }
};

void rank_cache::add(const std::string& url) {
    cache_entry entry;
    entry.url = url;
    entry.rating_period = current_rating_period;
    entry.id = entries.size() + 1;
    entries.push_back(entry);
}

bool rank_cache::in(const std::string& url) {
    return std::binary_search(begin(), end(), url, comparator());
}

void update_cache(const rank_cache& cache) {
    std::ofstream out("db.cache");
    out << cache.size() << ' ' << cache.current_rating_period << '\n';
    for(auto&& entry : cache) {
        out << entry.id << ' ' << entry.url << ' ' << entry.rating_period << '\n';
    }
}
} // hypest
