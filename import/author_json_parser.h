#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <vector>
#include "C:/Users/kos22/CLionProjects/library/databases/author_repository.h"
#include "C:/Users/kos22/CLionProjects/library/models/author.h"

class JSONAuthorReader {
private:
    AuthorRepository& repo_;
    std::string json_file_;

public:
    JSONAuthorReader(const std::string& file, AuthorRepository& repo);
    std::vector<Author> loadFromJSON();
};