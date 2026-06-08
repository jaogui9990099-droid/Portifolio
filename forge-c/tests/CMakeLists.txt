add_executable(forge-tests
    test_arena.c
    test_pool.c
    test_slab.c
    unity.c
)

target_link_libraries(forge-tests PRIVATE forge::forge)
add_test(NAME forge-tests COMMAND forge-tests)
