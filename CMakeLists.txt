cmake_minimum_required(VERSION 3.15)
project(library)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/kos22/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")


find_package(SQLiteCpp CONFIG REQUIRED)
message(STATUS "SQLiteCpp include dirs: ${SQLiteCpp_INCLUDE_DIRS}")
message(STATUS "SQLiteCpp libraries: ${SQLiteCpp_LIBRARIES}")


find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(library main.cpp
        library.cpp
        joiner.cpp
        databases/book_repository.cpp
        databases/author_repository.cpp
        databases/publisher_repository.cpp
        databases/genre_repository.cpp
        import/author_csv_parser.cpp
        import/author_json_parser.cpp
        import/genre_csv_parser.cpp
        import/genre_json_parser.cpp
        import/publisher_csv_parser.cpp
        import/publisher_json_parser.cpp
        import/book_csv_parser.cpp
        import/book_json_parser.cpp
)

# Линковка с библиотеками
target_link_libraries(library PRIVATE SQLiteCpp nlohmann_json::nlohmann_json spdlog::spdlog)
