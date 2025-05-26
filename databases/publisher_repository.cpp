#include "publisher_repository.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

PublisherRepository::PublisherRepository(const std::string& db_path) : db_(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    spdlog::info("PublisherRepository initialized with database: {}", db_path);
    initialize();
}

bool PublisherRepository::initialize() {
    try {
        db_.exec("CREATE TABLE IF NOT EXISTS publisher ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT NOT NULL, "
            "address TEXT, "
            "phone TEXT, "
            "mail TEXT)");
        spdlog::info("Publisher table initialized");
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to initialize publisher table: {}", e.what());
        return false;
    }
}

bool PublisherRepository::publisherExists(const Publisher& publisher) {
    try {
        SQLite::Statement query(db_, "SELECT 1 FROM publisher WHERE name = ?");
        query.bind(1, publisher.name);
        bool exists = query.executeStep();
        spdlog::debug("Checked existence of publisher '{}': {}", publisher.name, exists ? "exists" : "does not exist");
        return exists;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to check publisher existence: {}", e.what());
        return false;
    }
}

int PublisherRepository::save(Publisher& publisher) {
    if (publisherExists(publisher)) {
        spdlog::warn("Publisher '{}' already exists", publisher.name);
        return -1;
    }
    try {
        SQLite::Statement query(db_, "INSERT INTO publisher (name, address, phone, mail) VALUES (?, ?, ?, ?)");
        query.bind(1, publisher.name);
        query.bind(2, publisher.address);
        query.bind(3, publisher.phone);
        query.bind(4, publisher.mail);
        query.exec();
        int last_id = static_cast<int>(db_.getLastInsertRowid());
        publisher.id = last_id;
        spdlog::info("Saved publisher '{}', ID: {}", publisher.name, last_id);
        return last_id;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to save publisher '{}': {}", publisher.name, e.what());
        return -1;
    }
}

void PublisherRepository::printTable(const std::vector<Publisher>& publishers) {
    std::vector<std::string> headers = { "ID", "title", "address", "phone", "mail" };
    if (publishers.empty()) {
        std::cout << "No publishers found.\n";
        spdlog::info("No publishers found for display");
        return;
    }

    // Calculate column widths
    std::vector<int> widths = { 5, 15, 30, 10, 20 }; // Initial widths
    for (const auto& publisher : publishers) {
        widths[0] = std::max(widths[0], (int)std::to_string(publisher.id).length());
        widths[1] = std::max(widths[1], (int)publisher.name.length());
        widths[2] = std::max(widths[2], (int)publisher.address.length());
        widths[3] = std::max(widths[3], (int)publisher.phone.length());
        widths[4] = std::max(widths[4], (int)publisher.mail.length());
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
    for (const auto& publisher : publishers) {
        for (size_t i = 0; i < headers.size(); ++i) {
            std::string value;
            switch(i) {
                case 0: value = std::to_string(publisher.id); break;
                case 1: value = publisher.name; break;
                case 2: value = publisher.address; break;
                case 3: value = publisher.phone; break;
                case 4: value = publisher.mail; break;
            }
            std::cout << std::left << std::setw(widths[i]) << value.substr(0, widths[i]);
            if (i < headers.size() - 1) std::cout << " | ";
        }
        std::cout << "\n";
    }
    std::cout << std::string(std::accumulate(widths.begin(), widths.end(), 0) + 3 * (widths.size() - 1), '=') << "\n\n";
}

void PublisherRepository::showAll() {
    try {
        std::vector<Publisher> publishers;
        SQLite::Statement query(db_, "SELECT id, name, address, phone, mail FROM publisher");
        while (query.executeStep()) {
            publishers.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Retrieved {} publishers for showAll", publishers.size());
        printTable(publishers);
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to retrieve publishers: {}", e.what());
    }
}

bool PublisherRepository::update(const std::string& field, const int& id, const std::string& new_val) {
    try {
        SQLite::Statement check_query(db_, "SELECT 1 FROM publisher WHERE id = ?");
        check_query.bind(1, id);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("Publisher '{}' not found for update", id);
            return false;
        }
        std::string query_str = "UPDATE publisher SET " + field + " = ? WHERE id = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, new_val);
        query.bind(2, id);
        query.exec();
        spdlog::info("Updated field '{}' for publisher '{}' to '{}'", field, id, new_val);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to update publisher '{}': {}", id, e.what());
        return false;
    }
}

bool PublisherRepository::del(const std::string& field, const std::string& value) {
    try {
        std::string check_query_str = "SELECT 1 FROM publisher WHERE " + field + " = ?";
        SQLite::Statement check_query(db_, check_query_str);
        check_query.bind(1, value);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("No publisher found with {} = '{}'", field, value);
            return false;
        }
        std::string query_str = "DELETE FROM publisher WHERE " + field + " = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        query.exec();
        spdlog::info("Deleted publisher with {} = '{}'", field, value);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to delete publisher with {} = '{}': {}", field, value, e.what());
        return false;
    }
}

void PublisherRepository::filter(const std::string& field, const std::string& direction) {
    try {
        std::string query_str;
        if (direction == "up") {
            query_str = "SELECT id, name, address, phone, mail FROM publisher ORDER BY " + field + " ASC";
        }
        else if (direction == "down") {
            query_str = "SELECT id, name, address, phone, mail FROM publisher ORDER BY " + field + " DESC";
        }
        else {
            spdlog::error("Invalid sort direction: {}", direction);
            throw std::invalid_argument("Invalid sort direction");
        }
        std::vector<Publisher> publishers;
        SQLite::Statement query(db_, query_str);
        while (query.executeStep()) {
            publishers.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Filtered {} publishers by {} {}", publishers.size(), field, direction);
        printTable(publishers);
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to filter publishers: {}", e.what());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Filter error: {}", e.what());
    }
}

int PublisherRepository::find(const std::string& field, const std::string& value) {
    try {
        std::string query_str = "SELECT id, name, address, phone, mail FROM publisher WHERE " + field + " = ?";
        std::vector<Publisher> publishers;
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        while (query.executeStep()) {
            publishers.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }
        spdlog::info("Found {} publishers with {} = '{}'", publishers.size(), field, value);
        printTable(publishers);
        return publishers.size();
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to find publishers with {} = '{}': {}", field, value, e.what());
        return 0;
    }
}

void PublisherRepository::exportData(const std::string& format_type) {
    try {
        std::vector<Publisher> publishers;
        SQLite::Statement query(db_, "SELECT id, name, address, phone, mail FROM publisher");
        while (query.executeStep()) {
            publishers.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getString(),
                query.getColumn(3).getString(),
                query.getColumn(4).getString(),
                query.getColumn(0)
            );
        }


        if (format_type == "csv") {
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/publisher_export.csv", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open CSV file for export");
                throw std::runtime_error("Failed to open CSV file");
            }
            // Write UTF-8 BOM
            file << "\xEF\xBB\xBF";
            // Write headers
            file << "ID,title,address,phone,mail\n";
            // Write data
            for (const auto& publisher : publishers) {
                auto escape = [](const std::string& s) -> std::string {
                    std::string escaped = s;
                    if (escaped.find(',') != std::string::npos) {
                        escaped = "\"" + escaped + "\"";
                    }
                    return escaped;
                    };
                file << (publisher.id) << ","
                    << escape(publisher.name) << ","
                    << escape(publisher.address) << ","
                    << escape(publisher.phone) << ","
                    << escape(publisher.mail) << "\n";
            }
            file.close();
            spdlog::info("Exported {} publishers to CSV", publishers.size());
        }
        else if (format_type == "json") {
            nlohmann::json json_data = nlohmann::json::array();
            for (const auto& publisher : publishers) {
                json_data.push_back({
                    {"ID", publisher.id},
                    {"title", publisher.name},
                    {"address", publisher.address},
                    {"phone", publisher.phone},
                    {"mail", publisher.mail}
                    });
            }
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/publisher_export.json", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open JSON file for export");
                throw std::runtime_error("Failed to open JSON file");
            }
            file << json_data.dump(4);
            file.close();
            spdlog::info("Exported {} publishers to JSON", publishers.size());
        }
        else {
            spdlog::error("Invalid export format: {}", format_type);
            throw std::invalid_argument("Invalid export format");
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to export publishers: {}", e.what());
    }
}