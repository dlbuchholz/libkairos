include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.11.0
)

FetchContent_MakeAvailable(googletest)

add_executable(libkairos_tests
    test_time.cpp
)

target_link_libraries(libkairos_tests
    PRIVATE
        libkairos
        Qt${QT_VERSION_MAJOR}::Core
        gtest_main
)

add_test(NAME libkairos_tests COMMAND libkairos_tests)
