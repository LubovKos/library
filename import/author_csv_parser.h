#pragma once
#include <string>
#include <vector>
#include "C:/Users/kos22/CLionProjects/library/databases/author_repository.h"
#include "C:/Users/kos22/CLionProjects/library/models/author.h"

class CSVAuthorReader {
private:
    AuthorRepository& repo_;
    std::string csv_file_;

public:
    CSVAuthorReader(const std::string& file, AuthorRepository& repo);
    std::vector<Author> loadFromCSV();
};