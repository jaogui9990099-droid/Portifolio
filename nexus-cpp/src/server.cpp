#include "server.hpp"

namespace nexus {

KvServer::KvServer(const ServerConfig& cfg)
    : store_(cfg.map_initial_capacity)
    , hot_(cfg.hot_cache_capacity,
           std::chrono::duration<double>(cfg.hot_cache_ttl))
{}

std::optional<std::string> KvServer::get(const std::string& key) {
    if (auto cached = hot_.get(key)) return cached;
    if (auto val = store_.get(key)) {
        hot_.put(key, *val);
        return val;
    }
    return std::nullopt;
}

void KvServer::set(const std::string& key, std::string value) {
    hot_.erase(key);
    store_.put(key, std::move(value));
}

bool KvServer::del(const std::string& key) {
    hot_.erase(key);
    return store_.remove(key);
}

std::size_t KvServer::size() const {
    return store_.size();
}

LruCache<std::string, std::string>::Stats KvServer::cache_stats() const {
    return hot_.stats();
}

} // namespace nexus
