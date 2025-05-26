#include "genre_repository.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

GenreRepository::GenreRepository(const std::string& db_path) : db_(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    spdlog::info("GenreRepository initialized with database: {}", db_path);
    initialize();
}

bool GenreRepository::initialize() {
    try {
        db_.exec("CREATE TABLE IF NOT EXISTS genre ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "title TEXT NOT NULL, "
            "description TEXT)");
        spdlog::info("Genre table initialized");
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to initialize genre table: {}", e.what());
        return false;
    }
}

bool GenreRepository::genreExists(const Genre& genre) {
    try {
        SQLite::Statement query(db_, "SELECT 1 FROM genre WHERE title = ?");
        query.bind(1, genre.title);
        bool exists = query.executeStep();
        spdlog::debug("Checked existence of genre '{}': {}", genre.title, exists ? "exists" : "does not exist");
        return exists;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to check genre existence: {}", e.what());
        return false;
    }
}

int GenreRepository::save(Genre& genre) {
    if (genreExists(genre)) {
        spdlog::warn("Genre '{}' already exists", genre.title);
        return -1;
    }
    try {
        SQLite::Statement query(db_, "INSERT INTO genre (title, description) VALUES (?, ?)");
        query.bind(1, genre.title);
        query.bind(2, genre.description);
        query.exec();
        int last_id = static_cast<int>(db_.getLastInsertRowid());
        genre.id = last_id;
        spdlog::info("Saved genre '{}', ID: {}", genre.title, last_id);
        return last_id;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to save genre '{}': {}", genre.title, e.what());
        return -1;
    }
}

void GenreRepository::printTable(const std::vector<Genre>& genres) {
    std::vector<std::string> headers = { "ID", "title", "description" };
    if (genres.empty()) {
        std::cout << "No genres found.\n";
        spdlog::info("No genres found for display");
        return;
    }

    // Calculate column widths
    std::vector<int> widths = { 3, 15, 50 }; // Initial widths
    for (const auto& genre : genres) {
        widths[0] = std::max(widths[0], (int) std::to_string(genre.id).length());
        widths[1] = std::max(widths[1], (int) genre.title.length());
        widths[2] = std::max(widths[2], (int) genre.description.length());
    }

    // Print table
    std::cout << "\n" << std::string(std::accumulate(widths.begin(), widths.end(), 0) + 3 * (widths.size() - 1), '=') << "\n";

    // Print headers
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << std::left << std::setw(widths[i]) << headers[i].substr(0, widths[i]);
        if (i < headers.size() - 1) std::cout << " | ";
    }
    std::cout << "\n" << std::string(std::accumulate(widths.begin(), widths.end(), 0) + 3 * (widths.size() - 1), '-') << "\n";

    // Print rows
    for (const auto& genre : genres) {
        for (size_t i = 0; i < headers.size(); ++i) {
            std::string value;
            switch(i) {
                case 0: value = std::to_string(genre.id); break;
                case 1: value = genre.title; break;
                case 2: value = genre.description; break;
            }
            std::cout << std::left << std::setw(widths[i]) << value.substr(0, widths[i]);
            if (i < headers.size() - 1) std::cout << " | ";
        }
        std::cout << "\n";
    }
    std::cout << std::string(std::accumulate(widths.begin(), widths.end(), 0) + 3 * (widths.size() - 1), '=') << "\n\n";
}

void GenreRepository::showAll() {
    try {
        std::vector<Genre> genres;
        SQLite::Statement query(db_, "SELECT id, title, description FROM genre");
        while (query.executeStep()) {
            genres.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Retrieved {} genres for showAll", genres.size());
        printTable(genres);
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to retrieve genres: {}", e.what());
    }
}

bool GenreRepository::update(const std::string& field, const int& id, const std::string& new_val) {
    try {
        SQLite::Statement check_query(db_, "SELECT 1 FROM genre WHERE id = ?");
        check_query.bind(1, id);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("Genre '{}' not found for update", id);
            return false;
        }
        std::string query_str = "UPDATE genre SET " + field + " = ? WHERE id = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, new_val);
        query.bind(2, id);
        query.exec();
        spdlog::info("Updated field '{}' for genre '{}' to '{}'", field, id, new_val);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to update genre '{}': {}", id, e.what());
        return false;
    }
}

bool GenreRepository::del(const std::string& field, const std::string& value) {
    try {
        std::string check_query_str = "SELECT 1 FROM genre WHERE " + field + " = ?";
        SQLite::Statement check_query(db_, check_query_str);
        check_query.bind(1, value);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("No genre found with {} = '{}'", field, value);
            return false;
        }
        std::string query_str = "DELETE FROM genre WHERE " + field + " = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        query.exec();
        spdlog::info("Deleted genre with {} = '{}'", field, value);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to delete genre with {} = '{}': {}", field, value, e.what());
        return false;
    }
}

void GenreRepository::filter(const std::string& field, const std::string& direction) {
    try {
        std::string query_str;
        if (direction == "up") {
            query_str = "SELECT id, title, description FROM genre ORDER BY " + field + " ASC";
        }
        else if (direction == "down") {
            query_str = "SELECT id, title, description FROM genre ORDER BY " + field + " DESC";
        }
        else {
            spdlog::error("Invalid sort direction: {}", direction);
            throw std::invalid_argument("Invalid sort direction");
        }
        std::vector<Genre> genres;
        SQLite::Statement query(db_, query_str);
        while (query.executeStep()) {
            genres.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Filtered {} genres by {} {}", genres.size(), field, direction);
        printTable(genres);
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to filter genres: {}", e.what());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Filter error: {}", e.what());
    }
}

int GenreRepository::find(const std::string& field, const std::string& value) {
    try {
        std::string query_str = "SELECT id, title, description FROM genre WHERE " + field + " = ?";
        std::vector<Genre> genres;
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        while (query.executeStep()) {
            genres.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Found {} genres with {} = '{}'", genres.size(), field, value);
        printTable(genres);
        return genres.size();
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to find genres with {} = '{}': {}", field, value, e.what());
        return 0;
    }
}

void GenreRepository::exportData(const std::string& format_type) {
    try {
        std::vector<Genre> genres;
        SQLite::Statement query(db_, "SELECT id, title, description FROM genre");
        while (query.executeStep()) {
            genres.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(0)
            );
        }

        if (format_type == "csv") {
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/genre_export.csv", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open CSV file for export");
                throw std::runtime_error("Failed to open CSV file");
            }
            // Write UTF-8 BOM
            file << "\xEF\xBB\xBF";
            // Write headers
            file << "ID,title,description\n";
            // Write data
            for (const auto& genre : genres) {
                auto escape = [](const std::string& s) -> std::string {
                    std::string escaped = s;
                    if (escaped.find(',') != std::string::npos) {
                        escaped = "\"" + escaped + "\"";
                    }
                    return escaped;
                    };
                file << (genre.id) << ","
                    << escape(genre.title) << ","
                    << escape(genre.description) << "\n";
            }
            file.close();
            spdlog::info("Exported {} genres to CSV", genres.size());
        }
        else if (format_type == "json") {
            nlohmann::json json_data = nlohmann::json::array();
            for (const auto& genre : genres) {
                json_data.push_back({
                    {"ID", genre.id},
                    {"title", genre.title},
                    {"description", genre.description}
                    });
            }
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/genre_export.json", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open JSON file for export");
                throw std::runtime_error("Failed to open JSON file");
            }
            file << json_data.dump(4);
            file.close();
            spdlog::info("Exported {} genres to JSON", genres.size());
        }
        else {
            spdlog::error("Invalid export format: {}", format_type);
            throw std::invalid_argument("Invalid export format");
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to export genres: {}", e.what());
    }
}