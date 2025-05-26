#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <vector>
#include "C:/Users/kos22/CLionProjects/library/databases/genre_repository.h"
#include "C:/Users/kos22/CLionProjects/library/models/genre.h"

class JSONGenreReader {
private:
    GenreRepository& repo_;
    std::string json_file_;

public:
    JSONGenreReader(const std::string& file, GenreRepository& repo);
    std::vector<Genre> loadFromJSON();
};