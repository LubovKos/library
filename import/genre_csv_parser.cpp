#include "C:/Users/kos22/CLionProjects/library/import/genre_csv_parser.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
#include <map>

namespace {
    // Trim whitespace from both ends of a string
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        size_t last = str.find_last_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        return str.substr(first, last - first + 1);
    }

    // Split a CSV line into fields, handling quoted fields and commas
    std::vector<std::string> splitCSVLine(const std::string& line) {
        std::vector<std::string> fields;
        std::string field;
        bool in_quotes = false;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == '"') {
                in_quotes = !in_quotes;
            }
            else if (c == ',' && !in_quotes) {
                fields.push_back(trim(field));
                field.clear();
            }
            else {
                field += c;
            }
        }
        fields.push_back(trim(field));
        return fields;
    }
}

CSVGenreReader::CSVGenreReader(const std::string& file, GenreRepository& repo)
    : repo_(repo), csv_file_(file) {
    spdlog::info("CSVGenreReader initialized with file: {}", csv_file_);
}

std::vector<Genre> CSVGenreReader::loadFromCSV() {
    spdlog::info("Loading CSV from file: {}", csv_file_);
    std::vector<Genre> genres;
    try {
        std::ifstream file(csv_file_, std::ios::binary);
        if (!file.is_open()) {
            spdlog::error("Failed to open CSV file: {}", csv_file_);
            return genres;
        }

        // Skip UTF-8 BOM if present
        char bom[3];
        file.read(bom, 3);
        if (!(bom[0] == char(0xEF) && bom[1] == char(0xBB) && bom[2] == char(0xBF))) {
            file.seekg(0);
        }

        // Read header row
        std::string header_line;
        if (!std::getline(file, header_line)) {
            spdlog::error("Empty CSV file: {}", csv_file_);
            file.close();
            return genres;
        }

        // Parse header
        std::vector<std::string> headers = splitCSVLine(header_line);
        std::set<std::string> header_set(headers.begin(), headers.end());
        std::set<std::string> required_fields = { "Name", "Description" };

        // Validate headers
        std::set<std::string> missing_fields;
        std::set_difference(
            required_fields.begin(), required_fields.end(),
            header_set.begin(), header_set.end(),
            std::inserter(missing_fields, missing_fields.begin())
        );
        if (!missing_fields.empty()) {
            std::string missing;
            for (const auto& field : missing_fields) {
                missing += field + ", ";
            }
            if (!missing.empty()) missing = missing.substr(0, missing.size() - 2);
            spdlog::error("Missing required fields: {}", missing);
            throw std::runtime_error("CSV does not contain required headers");
        }

        spdlog::debug("Fieldnames CSV: {}", header_line);

        // Read data rows
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            spdlog::debug("Processing line: {}", line);
            std::vector<std::string> fields = splitCSVLine(line);
            if (fields.size() < headers.size()) {
                spdlog::warn("Invalid row, too few fields: {}", line);
                continue;
            }

            try {
                // Map fields to dictionary-like structure
                std::map<std::string, std::string> row;
                for (size_t i = 0; i < headers.size() && i < fields.size(); ++i) {
                    row[headers[i]] = fields[i];
                }

                Genre genre(
                    trim(row["Name"]),
                    trim(row["Description"])
                );
                if (repo_.save(genre) != -1) {
                    genres.push_back(genre);
                }
            }
            catch (const std::exception& e) {
                spdlog::warn("Error parsing row: {}. Error: {}", line, e.what());
            }
        }

        file.close();
        spdlog::info("Loaded {} genres from CSV", genres.size());
        return genres;
    }
    catch (const std::exception& e) {
        spdlog::error("Error reading CSV: {}", e.what());
        return genres;
    }
}