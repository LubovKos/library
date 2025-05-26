#pragma once
#include <string>
#include <vector>
#include "C:/Users/kos22/CLionProjects/library/databases/publisher_repository.h"
#include "C:/Users/kos22/CLionProjects/library/models/publisher.h"

class JSONPublisherReader {
private:
    PublisherRepository& repo_;
    std::string json_file_;

public:
    JSONPublisherReader(const std::string& file, PublisherRepository& repo);
    std::vector<Publisher> loadFromJSON();
};