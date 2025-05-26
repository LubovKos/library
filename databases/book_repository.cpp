#include "book_repository.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>
#include <algorithm>   
#include <fstream>
#include <iomanip>
#include <iostream>

BookRepository::BookRepository(const std::string& db_path) : db_(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
     spdlog::info("BookRepository initialized with database: {}", db_path);
    initialize();
}

bool BookRepository::initialize() {
    try {
        db_.exec("CREATE TABLE IF NOT EXISTS book ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "title TEXT NOT NULL, "
            "author_id INTEGER NOT NULL, "
            "year INTEGER, "
            "genre_id INTEGER, "
            "pages INTEGER, "
            "description TEXT, "
            "publisher_id INTEGER NOT NULL, "
            "FOREIGN KEY (author_id) REFERENCES author_id(id), "
            "FOREIGN KEY (genre_id) REFERENCES genre_id(id), "
            "FOREIGN KEY (publisher_id) REFERENCES publisher_id(id))");
        spdlog::info("Book table initialized");
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to initialize book table: {}", e.what());
        return false;
    }
}

bool BookRepository::bookExists(const Book& book) {
    try {
        SQLite::Statement query(db_, "SELECT 1 FROM book WHERE title = ? AND author_id = ? AND year = ? AND genre_id = ? AND pages = ? AND publisher_id = ?");
        query.bind(1, book.title);
        query.bind(2, book.author_id);
        query.bind(3, book.year);
        query.bind(4, book.genre_id);
        query.bind(5, book.pages);
        query.bind(6, book.publisher_id);
        bool exists = query.executeStep();
        spdlog::debug("Checked existence of book '{}': {}", book.title, exists ? "exists" : "does not exist");
        return exists;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to check book existence: {}", e.what());
        return false;
    }
}

int BookRepository::save(Book& book) {
    std::cout << "Saving";
    if (bookExists(book)) {
        spdlog::warn("Book '{}' by '{}' already exists", book.title, book.author_id);
        return -1;
    }
    try {
        SQLite::Statement query(db_, "INSERT INTO book (title, author_id, year, genre_id, pages, description, publisher_id) VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.bind(1, book.title);
        query.bind(2, book.author_id);
        query.bind(3, book.year);
        query.bind(4, book.genre_id);
        query.bind(5, book.pages);
        query.bind(6, book.description);
        query.bind(7, book.publisher_id);
        query.exec();
        int last_id = static_cast<int>(db_.getLastInsertRowid());
        book.id = last_id;
        spdlog::info("Saved book '{}', ID: {}", book.title, last_id);
        return last_id;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to save book '{}': {}", book.title, e.what());
        return -1;
    }
}

void BookRepository::printTable(const std::vector<Book>& books) {
    std::vector<std::string> headers = { "ID", "title", "author", "year", "genre", "pages", "publisher" };
    if (books.empty()) {
        std::cout << "No books found.\n";
        spdlog::info("No books found for display");
        return;
    }
    // Calculate column widths
    std::vector<int> widths = { 5, 20, 6, 7, 5, 5, 7 }; // Initial widths
    for (const auto& book : books) {
        widths[0] = std::max(widths[0], (int)(std::to_string(book.id).length()));
        widths[1] = std::max(widths[1], (int)book.title.length());
        widths[2] = std::max(widths[2], (int)(std::to_string(book.author_id).length()));
        widths[3] = std::max(widths[3], (int)std::to_string(book.year).length());
        widths[4] = std::max(widths[4], (int)(std::to_string(book.genre_id).length()));
        widths[5] = std::max(widths[5], (int)std::to_string(book.pages).length());
        widths[6] = std::max(widths[6], (int)(std::to_string(book.publisher_id).length()));
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
    for (const auto& book : books) {
        for (size_t i = 0; i < headers.size(); ++i) {
            std::string value;
            switch(i) {
                case 0: value = std::to_string(book.id); break;
                case 1: value = book.title; break;
                case 2: value = std::to_string(book.author_id); break;
                case 3: value = std::to_string(book.year); break;
                case 4: value = std::to_string(book.genre_id); break;
                case 5: value = std::to_string(book.pages); break;
                case 6: value = std::to_string(book.publisher_id); break;
            }
            std::cout << std::left << std::setw(widths[i]) << value.substr(0, widths[i]);
            if (i < headers.size() - 1) std::cout << " | ";
        }
        std::cout << "\n";
    }
    std::cout << std::string(std::accumulate(widths.begin(), widths.end(), 0) + 3 * (widths.size() - 1), '=') << "\n\n";
}

void BookRepository::showAll() {
    try {
        std::vector<Book> books;
        SQLite::Statement query(db_, "SELECT id, title, author_id, year, genre_id, pages, description, publisher_id FROM book");

        while (query.executeStep()) {
            books.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getInt(),
                query.getColumn(6).getString(),
                query.getColumn(3).getInt(),
                query.getColumn(4).getInt(),
                query.getColumn(7).getInt(),
                query.getColumn(5).getInt(),
                query.getColumn(0).getInt()
            );
        }
        spdlog::info("Retrieved {} books for showAll", books.size());
        printTable(books);
    }
    catch (const SQLite::Exception& e) {
        std::cout << "e";
        spdlog::error("Failed to retrieve books: {}", e.what());
    }
}

bool BookRepository::update(const std::string& field, const int& id, const std::string& new_val) {
    try {
        SQLite::Statement check_query(db_, "SELECT 1 FROM book WHERE id  = ?");
        check_query.bind(1, id);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("Book '{}' by not found for update", id);
            return false;
        }
        std::string query_str = "UPDATE book SET " + field + " = ? WHERE id = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, new_val);
        query.bind(2, id);
        query.exec();
        spdlog::info("Updated field '{}' for book '{}' to '{}'", field, id, new_val);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to update book '{}': {}", id, e.what());
        return false;
    }
}

bool BookRepository::del(const std::string& field, const std::string& value) {
    try {
        std::string check_query_str = "SELECT 1 FROM book WHERE " + field + " = ?";
        SQLite::Statement check_query(db_, check_query_str);
        check_query.bind(1, value);
        bool exists = check_query.executeStep();
        if (!exists) {
            spdlog::warn("No book found with {} = '{}'", field, value);
            return false;
        }
        std::string query_str = "DELETE FROM book WHERE " + field + " = ?";
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        query.exec();
        spdlog::info("Deleted book with {} = '{}'", field, value);
        return true;
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to delete book with {} = '{}': {}", field, value, e.what());
        return false;
    }
}

void BookRepository::filter(const std::string& field, const std::string& direction) {
    try {
        std::string query_str;
        if (direction == "up") {
            query_str = "SELECT id, title, author_id, year, genre_id, pages, description, publisher_id FROM book ORDER BY " + field + " ASC";
        }
        else if (direction == "down") {
            query_str = "SELECT id, title, author_id, year, genre_id, pages, description, publisher_id FROM book ORDER BY " + field + " DESC";
        }
        else {
            spdlog::error("Invalid sort direction: {}", direction);
            throw std::invalid_argument("Invalid sort direction");
        }
        std::vector<Book> books;
        SQLite::Statement query(db_, query_str);
        while (query.executeStep()) {
            books.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getInt(),
                query.getColumn(6).getString(),
                query.getColumn(3).getInt(),
                query.getColumn(4).getInt(),
                query.getColumn(7).getInt(),
                query.getColumn(5).getInt(),
                query.getColumn(0).getInt()
            );
        }
        spdlog::info("Filtered {} books by {} {}", books.size(), field, direction);
        printTable(books);
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to filter books: {}", e.what());
    }
    catch (const std::invalid_argument& e) {
        spdlog::error("Filter error: {}", e.what());
    }
}

int BookRepository::find(const std::string& field, const std::string& value) {
    try {
        std::string query_str = "SELECT id, title, author_id, year, genre_id, pages, description, publisher_id FROM book WHERE " + field + " = ?";
        std::vector<Book> books;
        SQLite::Statement query(db_, query_str);
        query.bind(1, value);
        while (query.executeStep()) {
            books.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getInt(),
                query.getColumn(6).getString(),
                query.getColumn(3).getInt(),
                query.getColumn(4).getInt(),
                query.getColumn(7).getInt(),
                query.getColumn(5).getInt(),
                query.getColumn(0).getInt()
            );
        }
        spdlog::info("Found {} books with {} = '{}'", books.size(), field, value);
        printTable(books);
        return books.size();
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("Failed to find books with {} = '{}': {}", field, value, e.what());
        return 0;
    }
}

void BookRepository::exportData(const std::string& format_type) {
    try {
        std::vector<Book> books;
        SQLite::Statement query(db_, "SELECT id, title, author_id, year, genre_id, pages, description, publisher_id FROM book");
        while (query.executeStep()) {
            books.emplace_back(
                query.getColumn(1).getString(),
                query.getColumn(2).getInt(),
                query.getColumn(6).getString(),
                query.getColumn(3).getInt(),
                query.getColumn(4).getInt(),
                query.getColumn(7).getInt(),
                query.getColumn(5).getInt(),
                query.getColumn(0).getInt()
            );
        }

        if (format_type == "csv") {
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/book_export.csv", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open CSV file for export");
                throw std::runtime_error("Failed to open CSV file");
            }
            // Write UTF-8 BOM
            file << "\xEF\xBB\xBF";
            // Write headers
            file << "ID,title,author_id,year,genre_id,pages,publisher_id\n";
            // Write data
            for (const auto& book : books) {
                auto escape = [](const std::string& s) -> std::string {
                    std::string escaped = s;
                    if (escaped.find(',') != std::string::npos) {
                        escaped = "\"" + escaped + "\"";
                    }
                    return escaped;
                    };
                file << (book.id) << ","
                    << escape(book.title) << ","
                    << book.author_id << ","
                    << book.year << ","
                    << book.genre_id << ","
                    << book.pages << ","
                    << book.publisher_id << "\n";
            }
            file.close();
            spdlog::info("Exported {} books to CSV", books.size());
        }
        else if (format_type == "json") {
            nlohmann::json json_data = nlohmann::json::array();
            for (const auto& book : books) {
                json_data.push_back({
                    {"ID", book.id},
                    {"title", book.title},
                    {"author_id", book.author_id},
                    {"year", book.year},
                    {"genre_id", book.genre_id},
                    {"pages", book.pages},
                    {"publisher_id", book.publisher_id}
                    });
            }
            std::ofstream file("C:/Users/kos22/CLionProjects/library/export/book_export.json", std::ios::out);
            if (!file.is_open()) {
                spdlog::error("Failed to open JSON file for export");
                throw std::runtime_error("Failed to open JSON file");
            }
            file << json_data.dump(4);
            file.close();
            spdlog::info("Exported {} books to JSON", books.size());
        }
        else {
            spdlog::error("Invalid export format: {}", format_type);
            throw std::invalid_argument("Invalid export format");
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to export books: {}", e.what());
    }
}