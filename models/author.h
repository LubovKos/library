#pragma once
#include <string>
#include <chrono>
#include <stdexcept>
#include <iomanip>
#include <sstream>

struct Author {
    int id;
    std::string full_name;
    std::string biography;
    std::string date_of_birth;
    std::string date_of_death;

    Author(const std::string& fn, const std::string& dob, const std::string& dod, const std::string& bio, int id = -1)
        : full_name(fn), date_of_birth(dob), date_of_death(dod), biography(bio), id(id) {
        validate();
    }

private:
    void validate() {
        // Validate full_name
        if (full_name.empty()) {
            throw std::invalid_argument("Author's name cannot be empty");
        }

        // Helper function to parse date in dd.mm.yyyy format
        auto parse_date = [](const std::string& date_str) -> std::chrono::system_clock::time_point {
            if (date_str.empty()) {
                return std::chrono::system_clock::time_point{}; // Empty date returns epoch
            }
            std::tm tm = {};
            std::istringstream ss(date_str);
            ss >> std::get_time(&tm, "%d.%m.%Y");
            if (ss.fail()) {
                throw std::invalid_argument("Invalid date format: " + date_str);
            }
            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
            };

        // Current time for future checks
        auto now = std::chrono::system_clock::now();

        // Parse dates if not empty
        std::chrono::system_clock::time_point birth_date, death_date;
        bool has_birth_date = !date_of_birth.empty();
        bool has_death_date = !date_of_death.empty();

        if (has_birth_date) {
            birth_date = parse_date(date_of_birth);
            // Check if birth date is in the future
            if (birth_date > now) {
                throw std::invalid_argument("Date of birth cannot be in the future");
            }
        }

        if (has_death_date) {
            death_date = parse_date(date_of_death);
            // Check if death date is in the future
            if (death_date > now) {
                throw std::invalid_argument("Date of death cannot be in the future");
            }
        }

        // Check if birth date is later than death date
        if (has_birth_date && has_death_date && birth_date > death_date) {
            throw std::invalid_argument("Date of death cannot be earlier than date of birth");
        }
    }
};