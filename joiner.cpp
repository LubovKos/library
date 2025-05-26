#include "joiner.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iomanip>
#include <iostream>
#include <algorithm>

namespace {
    // Calculate the maximum width for each column
    std::vector<size_t> calculateColumnWidths(const std::vector<std::vector<std::string>>& data,
        const std::vector<std::string>& headers) {
        std::vector<size_t> widths(headers.size(), 0);
        for (size_t i = 0; i < headers.size(); ++i) {
            widths[i] = headers[i].size();
        }
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                widths[i] = std::max(widths[i], row[i].size());
            }
        }
        return widths;
    }

    // Print a table with grid formatting
    void printTable(const std::vector<std::vector<std::string>>& data,
        const std::vector<std::string>& headers) {
        if (data.empty()) {
            std::cout << "No data to display.\n";
            return;
        }

        auto widths = calculateColumnWidths(data, headers);

        // Print top border
        std::cout << "\n" << std::string(100, '=') << "\n";

        // Print headers
        std::cout << "|";
        for (size_t i = 0; i < headers.size(); ++i) {
            std::cout << " " << std::left << std::setw(widths[i]) << headers[i] << " |";
        }
        std::cout << "\n";

        // Print header separator
        std::cout << "|";
        for (size_t i = 0; i < headers.size(); ++i) {
            std::cout << std::string(widths[i] + 2, '-') << "|";
        }
        std::cout << "\n";

        // Print data rows
        for (const auto& row : data) {
            std::cout << "|";
            for (size_t i = 0; i < row.size(); ++i) {
                std::cout << " " << std::left << std::setw(widths[i]) << row[i] << " |";
            }
            std::cout << "\n";
        }

        // Print bottom border
        std::cout << std::string(100, '=') << "\n\n";
    }
}

Joiner::Joiner(const std::string& db_path) : db_path_(db_path) {
    spdlog::info("Joiner initialized with database: {}", db_path_);
}

int Joiner::join(const std::string& table_title) {
    spdlog::info("Executing JOIN query for table: {}", table_title);
    try {
        SQLite::Database db(db_path_, SQLite::OPEN_READONLY);
        SQLite::Statement query(db, "");
        std::vector<std::string> headers;
        std::vector<std::vector<std::string>> table_data;

        if (table_title == "author") {
            query = SQLite::Statement(db,
                "SELECT book.title, book.year, book.genre_id, book.pages, book.publisher_id, "
                "author.full_name, author.date_of_birth, author.date_of_death "
                "FROM book JOIN author ON book.author_id = author.id");
            headers = { "title", "year", "genre", "pages", "publisher",
                       "author", "date_of_birth", "date_of_death" };
        }
        else if (table_title == "publisher") {
            query = SQLite::Statement(db,
                "SELECT book.title, book.author_id, book.year, book.genre_id, book.pages, "
                "publisher.name, publisher.address, publisher.phone, publisher.mail "
                "FROM book JOIN publisher ON book.publisher_id = publisher.id");
            headers = { "title", "author", "year", "genre", "pages",
                       "publisher", "address", "phone", "mail" };
        }
        else {
            query = SQLite::Statement(db,
                "SELECT book.title, book.author_id, book.year, book.pages, book.publisher_id, "
                "genre.title, genre.description "
                "FROM book JOIN genre ON book.genre_id = genre.id");
            headers = { "title", "author", "year", "pages", "publisher",
                       "genre", "description" };
        }

        while (query.executeStep()) {
            std::vector<std::string> row;
            for (int i = 0; i < query.getColumnCount(); ++i) {
                std::string value = query.getColumn(i).isNull() ? "" : query.getColumn(i).getString();
                row.push_back(value);
            }
            table_data.push_back(row);
        }

        printTable(table_data, headers);
        spdlog::info("Displayed {} rows for {} JOIN", table_data.size(), table_title);

        return static_cast<int>(table_data.size());
    }
    catch (const SQLite::Exception& e) {
        spdlog::error("SQLite error in JOIN query for {}: {}", table_title, e.what());
        throw;
    }
    catch (const std::exception& e) {
        spdlog::error("Error in JOIN query for {}: {}", table_title, e.what());
        throw;
    }
}