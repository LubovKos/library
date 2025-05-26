#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <vector>
#include "C:/Users/kos22/CLionProjects/library/databases/publisher_repository.h"
#include "C:/Users/kos22/CLionProjects/library/models/publisher.h"

class CSVPublisherReader {
private:
    PublisherRepository& repo_;
    std::string csv_file_;

public:
    CSVPublisherReader(const std::string& file, PublisherRepository& repo);
    std::vector<Publisher> loadFromCSV();
}; 