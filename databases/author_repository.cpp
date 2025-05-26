#include "author_repository.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

AuthorRepository::AuthorRepository(const std::string& db_path) : db_(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    spdlog::info("AuthorRepository initialized with database: {}", db_path);
    initialize();
}

bool AuthorRepository::initialize() {
    try {
        db_.exec("CREATE TABLE IF NOT EXISTS author ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "full_name TEXT, "
            "date_of_birth TEXT, "
            "date_of_death TEXT, "
            "biography TEXT)");
        spdlog::info("Author table initialized");
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to initialize author table: {}", e.what());
        return false;
    }
}

bool AuthorRepository::authorExists(const Author& author) {
    try {
        SQLite::Statement query(db_, "SELECT 1 FROM author WHERE full_name = ?");
        query.bind(1, author.full_name);
        bool exists = query.executeStep();
        spdlog::debug("Checked existence of author '{}': {}", author.full_name, exists ? "exists" : "does not exist");
        return exists;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to check author existence: {}", e.what());
        return false;
    }
}

int AuthorRepository::save(const Author& author) {
    if (authorExists(author)) {
        spdlog::warn("Author '{}' already exists", author.full_name);
        return -1;
    }
    try {
        SQLite::Statement query(db_, "INSERT INTO author (full_name, date_of_birth, date_of_death, biography) VALUES (?, ?, ?, ?)");
        query.bind(1, author.full_name);
        query.bind(2, author.date_of_birth);
        query.bind(3, author.date_of_death);
        query.bind(4, author.biography);
        query.exec();
        int last_id = static_cast<int>(db_.getLastInsertRowid());
        spdlog::info("Saved author '{}', ID: {}", author.full_name, last_id);
        return last_id;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to save author '{}': {}", author.full_name, e.what());
        return -1;
    }
}

void AuthorRepository::printTable(const std::vector<Author>& authors) {
    std::vector<std::string> headers = { "id", "full_name", "birth", "death", "biography" };
    if (authors.empty()) {
        std::cout << "No authors found.\n";
        spdlog::info("No authors found for display");
        return;
    }

    // Calculate column widths
    std::vector<size_t> widths = {3, 20, 10, 10, 50}; // Initial widths
    for (const auto& author : authors) {
        widths[0] = std::max(widths[0], std::to_string(author.id).length());
        widths[1] = std::max(widths[1], author.full_name.length());
        widths[2] = std::max(widths[2], author.date_of_birth.length());
        widths[3] = std::max(widths[3], author.date_of_death.length());
        widths[4] = std::max(widths[4], author.biography.length());
    }

    // Ensure minimum width for headers
    for (size_t i = 0; i < headers.size(); ++i) {
        widths[i] = std::max(widths[i], headers[i].length());
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
    for (const auto& author : authors) {
        for (size_t i = 0; i < headers.size(); ++i) {
            std::string value;
            switch(i) {
                case 0: value = std::to_string(author.id); break;
                case 1: value = author.full_name; break;
                case 2: value = author.date_of_birth; break;
                case 3: value = author.date_of_death; break;
                case 4: value = author.biography; break;
            }
            std::cout << std::left << std::setw(widths[i]) << value.substr(0, widths[i]);
            if (i < headers.size() - 1) std::cout << " | ";
        }
        std::cout << "\n";
    }
    std::cout << std::string(std::accumulate(widths.begin(), widths.end(), 0) + 3 * (widths.size() - 1), '=') << "\n\n";
}

void AuthorRepository::showAll() {
    try {
        std::vector<Author> authors;
        SQLite::Statement query(db_, "SELECT id, full_name, date_of_birth, date_of_death, biography FROM author");
        while (query.executeStep()) {
            authors.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Retrieved {} authors for showAll", authors.size());
        printTable(authors);
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to retrieve authors: {}", e.what());
    }
}

bool AuthorRepository::update(const std::string& field, const int& id, const std::string& new_val) {
    try {
        SQLite::Statement check_query(db_, "SELECT 1 FROM author WHERE id = ?");
        check_query.bind(1, id);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("Author '{}' not found for update", id);
            return false;
        }
        std::string query_str = "UPDATE author SET " + field + " = ? WHERE id = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, new_val);
        query.bind(2, id);
        query.exec();
        spdlog::info("Updated field '{}' for author '{}' to '{}'", field, id, new_val);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to update author '{}': {}", id, e.what());
        return false;
    }
}

bool AuthorRepository::del(const std::string& field, const std::string& value) {
    try {
        std::string check_query_str = "SELECT 1 FROM author WHERE " + field + " = ?";
        SQLite::Statement check_query(db_, check_query_str);
        check_query.bind(1, value);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("No author found with {} = '{}'", field, value);
            return false;
        }
        std::string query_str = "DELETE FROM author WHERE " + field + " = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        query.exec();
        spdlog::info("Deleted author with {} = '{}'", field, value);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to delete author with {} = '{}': {}", field, value, e.what());
        return false;
    }
}

void AuthorRepository::filter(const std::string& field, const std::string& direction) {
    try {
        std::string query_str;
        if (direction == "up") {
            query_str = "SELECT id, full_name, date_of_birth, date_of_death, biography FROM author ORDER BY " + field + " ASC";
        }
        else if (direction == "down") {
            query_str = "SELECT id, full_name, date_of_birth, date_of_death, biography FROM author ORDER BY " + field + " DESC";
        }
        else {
            spdlog::error("Invalid sort direction: {}", direction);
            throw std::invalid_argument("Invalid sort direction");
        }
        std::vector<Author> authors;
        SQLite::Statement query(db_, query_str);
        while (query.executeStep()) {
            authors.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Filtered {} authors by {} {}", authors.size(), field, direction);
        printTable(authors);
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to filter authors: {}", e.what());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Filter error: {}", e.what());
    }
}

int AuthorRepository::find(const std::string& field, const std::string& value) {
    try {
        std::string query_str = "SELECT id, full_name, date_of_birth, date_of_death, biography FROM author WHERE " + field + " = ?";
        std::vector<Author> authors;
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        while (query.executeStep()) {
            authors.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Found {} authors with {} = '{}'", authors.size(), field, value);
        printTable(authors);
        return authors.size();
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to find authors with {} = '{}': {}", field, value, e.what());
        return 0;
    }
}

void AuthorRepository::exportData(const std::string& format_type) {
    try {
        std::vector<Author> authors;
        SQLite::Statement query(db_, "SELECT id, full_name, date_of_birth, date_of_death, biography FROM author");
        while (query.executeStep()) {
            authors.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }


        if (format_type == "csv") {
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/author_export.csv", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open CSV file for export");
                throw std::runtime_error("Failed to open CSV file");
            }
            // Write UTF-8 BOM for compatibility
            file << "\xEF\xBB\xBF";
            // Write headers
            file << "id,full_name,date_of_birth,date_of_death,biography\n";
            // Write data
            for (const auto& author : authors) {
                // Escape commas in fields
                auto escape = [](const std::string& s) -> std::string {
                    std::string escaped = s;
                    if (escaped.find(',') != std::string::npos) {
                        escaped = "\"" + escaped + "\"";
                    }
                    return escaped;
                    };
                file << std::to_string(author.id) << "," <<
                    escape(author.full_name) << ","
                    << escape(author.date_of_birth) << ","
                    << escape(author.date_of_death) << ","
                    << escape(author.biography) << "\n";
            }
            file.close();
            spdlog::info("Exported {} authors to CSV", authors.size());
        }
        else if (format_type == "json") {
            nlohmann::json json_data = nlohmann::json::array();
            for (const auto& author : authors) {
                json_data.push_back({
                    {"id", author.id},
                    {"full_name", author.full_name},
                    {"date_of_birth", author.date_of_birth},
                    {"date_of_death", author.date_of_death},
                    {"biography", author.biography}
                    });
            }
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/author_export.json", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open JSON file for export");
                throw std::runtime_error("Failed to open JSON file");
            }
            file << json_data.dump(4);
            file.close();
            spdlog::info("Exported {} authors to JSON", authors.size());
        }
        else {
            spdlog::error("Invalid export format: {}", format_type);
            throw std::invalid_argument("Invalid export format");
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to export authors: {}", e.what());
    }
}