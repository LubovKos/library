#pragma once
#include <string>
#include <chrono>
#include <ctime> 
#include <stdexcept>

struct Book {
    std::string title;
    int author_id;
    std::string description;
    int year;
    int genre_id;
    int publisher_id;
    int pages;
    int id;

    Book(const std::string& t, const int& a, const std::string& desc, int y,
        const int& g, const int& p, int pg, const int& id = -1)
        : title(t), author_id(a), description(desc), year(y), genre_id(g), publisher_id(p), pages(pg), id(id) {
        validate();
    }

private:
    void validate() {
        if (title.empty()) {
            throw std::invalid_argument("Book title must not be empty");
        }
        if (std::to_string(author_id).empty()) {
            throw std::invalid_argument("Author field must not be empty");
        }

        auto now = std::chrono::system_clock::now();
        auto now_t = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm = {};
        errno_t err = localtime_s(&now_tm, &now_t);

        if (err != 0) {
            throw std::runtime_error("Failed to get local time");
        }

        int current_year = now_tm.tm_year + 1900;
        if (year > current_year) {
            throw std::invalid_argument("The year of publication cannot be in the future");
        }

    }
};