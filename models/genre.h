#pragma once
#include <string>
#include <stdexcept>

struct Genre {
    std::string title;
    std::string description;
    int id;

    Genre(const std::string& t, const std::string& desc, const int& id = -1)
        : title(t), description(desc), id(id) {
        validate();
    }

private:
    void validate() {
        if (title.empty()) {
            throw std::invalid_argument("Genre name must not be empty");
        }
    }
};