
cmake_minimum_required(VERSION 3.25)

project(SearchEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(libpqxx CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS locale)
find_package(Threads REQUIRED)
find_package(Catch2 3 CONFIG REQUIRED)

# Основная программа: индексатор и поисковик
add_executable(SearchEngine
    src/main.cpp
)

target_include_directories(SearchEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SearchEngine PRIVATE
    libpqxx::pqxx
    Boost::locale
    Threads::Threads
)

# Модульные тесты
add_executable(SearchEngineTests
    tests/TestQueryProcessor.cpp
)

target_include_directories(SearchEngineTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SearchEngineTests PRIVATE
    Catch2::Catch2WithMain
    Boost::locale
)

# Подключение и автоматический поиск тестов Catch2
include(CTest)
include(Catch)

catch_discover_tests(SearchEngineTests)

if(MSVC)
    target_compile_options(SearchEngine PRIVATE /utf-8)
    target_compile_options(SearchEngineTests PRIVATE /utf-8)
endif()

