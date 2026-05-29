#include "server.hpp"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace nexus;
using namespace std::chrono_literals;

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " [--capacity N] [--cache N] [--ttl N]\n"
              << "  --capacity N   Primary map initial capacity (default: 65536)\n"
              << "  --cache N      Hot-cache capacity in entries (default: 8192)\n"
              << "  --ttl N        Hot-cache TTL in seconds (default: 30)\n";
}

static void run_demo(KvServer& kv) {
    constexpr int kKeys = 500000;

    std::cout << "[nexus] Populating " << kKeys << " keys...\n";
    auto t0 = std::chrono::steady_clock::now();

    for (int i = 0; i < kKeys; ++i) {
        kv.set("key:" + std::to_string(i), "value:" + std::to_string(i));
    }

    auto t1 = std::chrono::steady_clock::now();
    double write_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "[nexus] Writes: " << kKeys << " ops in "
              << std::fixed << std::setprecision(2) << write_ms << "ms ("
              << static_cast<int>(kKeys / (write_ms / 1000.0)) << " ops/s)\n";

    std::cout << "[nexus] Reading " << kKeys << " keys (sequential)...\n";
    t0 = std::chrono::steady_clock::now();
    int hits = 0;
    for (int i = 0; i < kKeys; ++i) {
        if (kv.get("key:" + std::to_string(i))) ++hits;
    }
    t1 = std::chrono::steady_clock::now();
    double read_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "[nexus] Reads:  " << hits << "/" << kKeys << " hits in "
              << std::fixed << std::setprecision(2) << read_ms << "ms ("
              << static_cast<int>(kKeys / (read_ms / 1000.0)) << " ops/s)\n";

    auto cs = kv.cache_stats();
    std::cout << "[nexus] Hot cache — hits=" << cs.hits
              << " misses=" << cs.misses
              << " evictions=" << cs.evictions
              << std::fixed << std::setprecision(1)
              << " hit_rate=" << cs.hit_rate() * 100.0 << "%\n";

    std::cout << "[nexus] Store size: " << kv.size() << " entries\n";
}

int main(int argc, char** argv) {
    ServerConfig cfg;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--capacity" || arg == "-c") && i + 1 < argc) {
            cfg.map_initial_capacity = static_cast<std::size_t>(std::atoll(argv[++i]));
        } else if ((arg == "--cache") && i + 1 < argc) {
            cfg.hot_cache_capacity = static_cast<std::size_t>(std::atoll(argv[++i]));
        } else if ((arg == "--ttl") && i + 1 < argc) {
            cfg.hot_cache_ttl = std::chrono::seconds(std::atoll(argv[++i]));
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    std::cout << "[nexus] Starting — map_capacity=" << cfg.map_initial_capacity
              << " cache=" << cfg.hot_cache_capacity
              << " ttl=" << cfg.hot_cache_ttl.count() << "s\n";

    KvServer kv(cfg);
    run_demo(kv);

    return 0;
}
