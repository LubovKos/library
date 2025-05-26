#include <iostream>
#include <SQLiteCpp/SQLiteCpp.h>
#include <nlohmann/json.hpp>
#include "models/book.h"
#include "databases/book_repository.h"

#include "library.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


int main() {
    std::cout << "ok";
    try {
        // Initialize spdlog
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("library.log", true);
        std::vector<spdlog::sink_ptr> sinks = { console_sink, file_sink };
        auto logger = std::make_shared<spdlog::logger>("library", sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [%n:%#] %v");

        Library library;
        std::cout << "Welcome to the Library Management System\n";
        spdlog::info("Program started");
        mainMenu(library);
    }
    catch (const std::exception& e) {
        spdlog::error("Program terminated with error: {}", e.what());
        std::cout << "Program terminated with error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        spdlog::error("Program terminated with unknown error");
        std::cout << "Program terminated with unknown error\n";
        return 1;
    }
    return 0;
}
