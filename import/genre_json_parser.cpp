#include "genre_json_parser.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fstream>
#include <set>
#include <algorithm>

JSONGenreReader::JSONGenreReader(const std::string& file, GenreRepository& repo)
    : repo_(repo), json_file_(file) {
    spdlog::info("JSONGenreReader initialized with file: {}", json_file_);
}

std::vector<Genre> JSONGenreReader::loadFromJSON() {
    spdlog::info("Loading JSON from file: {}", json_file_);
    std::vector<Genre> genres;
    try {
        std::ifstream file(json_file_);
        if (!file.is_open()) {
            spdlog::error("Failed to open JSON file: {}", json_file_);
            return genres;
        }

        nlohmann::json json_data;
        file >> json_data;
        file.close();

        // Check if json_data is an array
        if (!json_data.is_array()) {
            spdlog::error("JSON is not an array");
            return genres;
        }

        std::set<std::string> required_fields = { "Name", "Description" };
        int row_number = 1;

        for (const auto& item : json_data) {
            spdlog::debug("Processing row: {}", row_number);

            // Check for required fields
            std::set<std::string> item_keys;
            for (auto it = item.begin(); it != item.end(); ++it) {
                item_keys.insert(it.key());
            }

            std::set<std::string> missing_fields;
            std::set_difference(
                required_fields.begin(), required_fields.end(),
                item_keys.begin(), item_keys.end(),
                std::inserter(missing_fields, missing_fields.begin())
            );
            if (!missing_fields.empty()) {
                std::string missing;
                for (const auto& field : missing_fields) {
                    missing += field + ", ";
                }
                if (!missing.empty()) missing = missing.substr(0, missing.size() - 2);
                spdlog::warn("Missing fields in row {}: {}", row_number, missing);
                throw std::runtime_error("JSON does not contain required headers");
            }

            try {
                Genre genre(
                    item["Name"].get<std::string>(),
                    item["Description"].get<std::string>()
                );
                if (repo_.save(genre) != -1) {
                    genres.push_back(genre);
                } else {
                    spdlog::warn("Genre already exists in row {}: {}", row_number, item["Name"].get<std::string>());
                }
            }
            catch (const std::exception& e) {
                spdlog::warn("Error parsing row {}: {}", row_number, e.what());
            }
            ++row_number;
        }

        spdlog::info("Loaded {} genres from JSON", genres.size());
        return genres;
    }
    catch (const std::exception& e) {
        spdlog::error("Error reading JSON: {}", e.what());
        return genres;
    }
}