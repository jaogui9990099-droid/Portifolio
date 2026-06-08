#pragma once

#include <nexus/concurrent_map.hpp>
#include <nexus/lru_cache.hpp>

#include <chrono>
#include <optional>
#include <string>

namespace nexus {

struct ServerConfig {
    std::size_t map_initial_capacity = 65536;
    std::size_t hot_cache_capacity   = 8192;
    std::chrono::seconds hot_cache_ttl{30};
};

class KvServer {
public:
    explicit KvServer(const ServerConfig& cfg = {});

    std::optional<std::string> get(const std::string& key);
    void                       set(const std::string& key, std::string value);
    bool                       del(const std::string& key);
    std::size_t                size() const;
    LruCache<std::string, std::string>::Stats cache_stats() const;

private:
    ConcurrentMap<std::string, std::string> store_;
    LruCache<std::string, std::string>      hot_;
};

} // namespace nexus
