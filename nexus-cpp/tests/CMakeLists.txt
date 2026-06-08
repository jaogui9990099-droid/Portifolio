add_executable(nexus-tests
    test_concurrent_map.cpp
    test_lru_cache.cpp
    test_memory_pool.cpp
)

target_link_libraries(nexus-tests PRIVATE nexus::nexus GTest::gtest_main Threads::Threads)

include(GoogleTest)
gtest_discover_tests(nexus-tests)
