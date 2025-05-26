#pragma once
#include <string>
#include <vector>
#include <map>
#include "C:/Users/kos22/CLionProjects/library/databases/book_repository.h"
#include "C:/Users/kos22/CLionProjects/library/databases/author_repository.h"
#include "C:/Users/kos22/CLionProjects/library/databases/publisher_repository.h"
#include "C:/Users/kos22/CLionProjects/library/databases/genre_repository.h"
#include "C:/Users/kos22/CLionProjects/library/import/book_json_parser.h"
#include "C:/Users/kos22/CLionProjects/library/import/book_csv_parser.h"
#include "C:/Users/kos22/CLionProjects/library/import/author_json_parser.h"
#include "C:/Users/kos22/CLionProjects/library/import/author_csv_parser.h"
#include "C:/Users/kos22/CLionProjects/library/import/publisher_json_parser.h"
#include "C:/Users/kos22/CLionProjects/library/import/publisher_csv_parser.h"
#include "C:/Users/kos22/CLionProjects/library/import/genre_json_parser.h"
#include "C:/Users/kos22/CLionProjects/library/import/genre_csv_parser.h"
#include "joiner.h"

class Library {
private:
    BookRepository book_repo_;
    AuthorRepository author_repo_;
    PublisherRepository publisher_repo_;
    GenreRepository genre_repo_;
    Joiner joiner_;
    std::string data_path_;

public:
    Library(const std::string& db_path = "library.db", const std::string& data_path = "C:/Users/kos22/CLionProjects/library/data/");
    bool load(const std::string& path, const std::string& choice);
    void filter(const std::string& choice, const std::string& field, const std::string& direction);
    int search(const std::string& choice, const std::string& field, const std::string& value);
    int addRecord(const std::string& choice, const std::map<std::string, std::string>& record);
    bool updateRecord(const std::string& choice, const std::string& field, const std::string& new_val,
        const int& id = -1);
    bool deleteRecord(const std::string& choice, const std::string& field, const std::string& value);
    void displayAll(const std::string& choice);
    void join(const std::string& choice);
    void exportData(const std::string& choice, const std::string& format);
  
}; 
// CLI function declarations
void mainMenu(Library& library);
void searchMenu(Library& library);
void importData(Library& library);
void addRecordMenu(Library& library);
void updateRecordMenu(Library& library);
void deleteRecordMenu(Library& library);
void filteringMenu(Library& library);
void displayRecordsMenu(Library& library);
void showFullInfo(Library& library);
void exportDataMenu(Library& library);