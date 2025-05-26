#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <vector>
#include "C:/Users/kos22/CLionProjects/library/databases/genre_repository.h"
#include "C:/Users/kos22/CLionProjects/library/models/genre.h"

class CSVGenreReader {
private:
    GenreRepository& repo_;
    std::string csv_file_;

public:
    CSVGenreReader(const std::string& file, GenreRepository& repo);
    std::vector<Genre> loadFromCSV();
};