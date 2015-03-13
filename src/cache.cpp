#include <error.hpp>
#include <cache.hpp>
#include <fstream>
#include <cstdint>
#include <algorithm>

namespace hypest {
cache_t get_cache() {
    std::ifstream in("db.cache");
    if(not in.good()) {
        in.close();
        return {};
    }

    // file format:
    // header: number of entries
    // entry: id url connection
    // repeat..
    // if a connection is connected to something, e.g. entry 2 is connected to entry 4 then
    // they should denote their relationship together e.g.:
    // 2 url 4
    // 4 url 2

    uint32_t length;
    if(not (in >> length)) {
        throw fatal_error("invalid cache format: no number of entries found");
    }

    cache_t entries;
    for(uint32_t i = 0; i < length; ++i) {
        cache_entry entry;
        if((in >> entry.id >> entry.url >> entry.link).fail()) {
            throw fatal_error("invalid cache format: incorrect entry (must be 'id url link')");
        }
        entries.push_back(entry);
    }

    std::sort(std::begin(entries), std::end(entries));
    return entries;
}

void update_cache(const cache_t& cache) {
    std::ofstream out("db.cache");
    out << cache.size() << '\n';
    for(auto&& entry : cache) {
        out << entry.id << ' ' << entry.url << ' ' << entry.link << '\n';
    }
}
} // hypest
