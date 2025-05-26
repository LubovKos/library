#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <vector>
#include "C:/Users/kos22/CLionProjects/library/databases/book_repository.h"
#include "C:/Users/kos22/CLionProjects/library/models/book.h"

class CSVBookReader {
private:
    BookRepository& repo_;
    std::string csv_file_;

public:
    CSVBookReader(const std::string& file, BookRepository& repo);
    std::vector<Book> loadFromCSV();
};