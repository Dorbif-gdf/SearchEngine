cmake_minimum_required(VERSION 3.25)

project(SearchEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(libpqxx CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS locale)
find_package(Threads REQUIRED)
find_package(Catch2 3 CONFIG REQUIRED)

# Индексатор документов
add_executable(Indexer
    src/IndexerMain.cpp
)

target_include_directories(Indexer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Indexer PRIVATE
    libpqxx::pqxx
    Boost::locale
    Threads::Threads
)

# Поисковик документов
add_executable(Searcher
    src/SearcherMain.cpp
)

target_include_directories(Searcher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Searcher PRIVATE
    libpqxx::pqxx
    Boost::locale
)

# Модульные тесты
add_executable(SearchEngineTests
    tests/TestQueryProcessor.cpp
    tests/TestConfig.cpp
    tests/TestTextProcessor.cpp
    tests/TestFileScanner.cpp
)

target_include_directories(SearchEngineTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SearchEngineTests PRIVATE
    Catch2::Catch2WithMain
    Boost::locale
)

# Автоматическое обнаружение тестов Catch2
include(CTest)
include(Catch)

catch_discover_tests(SearchEngineTests)

if(MSVC)
    target_compile_options(Indexer PRIVATE /utf-8)
    target_compile_options(Searcher PRIVATE /utf-8)
    target_compile_options(SearchEngineTests PRIVATE /utf-8)
endif()