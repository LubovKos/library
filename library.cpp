#include "library.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include <string>
#include <cstdio>


inline bool file_exist(const std::string& name) {
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, name.c_str(), "r");
    if (err == 0 && file != nullptr) {
        fclose(file);
        return true;
    }
    return false;
}

Library::Library(const std::string& db_path, const std::string& data_path)
    : book_repo_(db_path), author_repo_(db_path), publisher_repo_(db_path),
    genre_repo_(db_path), joiner_(db_path), data_path_(data_path) {
    if (!author_repo_.initialize() || !genre_repo_.initialize() || !publisher_repo_.initialize() ||
         !book_repo_.initialize()) {
        spdlog::error("Failed to initialize repositories");
        throw std::runtime_error("Repository initialization failed");
    }
    spdlog::info("Library initialized with data path: {}", data_path_);
}

bool Library::load(const std::string& path, const std::string& choice) {
    std::string full_path = data_path_ + path;
    spdlog::info("Loading file: {}", full_path);
    try {
        std::string file_path(full_path);
        if (!file_exist(file_path)) {
            spdlog::error("File not found: {}", full_path);
            std::cout << "File '" << full_path << "' not found\n";
            return false;
        }

        if (file_path.find(".json") != std::string::npos) {
            if (choice == "1") {
                JSONBookReader reader(full_path, book_repo_);
                auto data = reader.loadFromJSON();
                spdlog::info("Imported {} books from JSON", data.size());
                std::cout << "Imported " << data.size() << " books\n";
                return !data.empty();
            }
            else if (choice == "2") {
                JSONAuthorReader reader(full_path, author_repo_);
                auto data = reader.loadFromJSON();
                spdlog::info("Imported {} authors from JSON", data.size());
                std::cout << "Imported " << data.size() << " authors\n";
                return !data.empty();
            }
            else if (choice == "3") {
                JSONPublisherReader reader(full_path, publisher_repo_);
                auto data = reader.loadFromJSON();
                spdlog::info("Imported {} publishers from JSON", data.size());
                std::cout << "Imported " << data.size() << " publishers\n";
                return !data.empty();
            }
            else if (choice == "4") {
                JSONGenreReader reader(full_path, genre_repo_);
                auto data = reader.loadFromJSON();
                spdlog::info("Imported {} genres from JSON", data.size());
                std::cout << "Imported " << data.size() << " genres\n";
                return !data.empty();
            }
        }
        else if (file_path.find(".csv") != std::string::npos) {
            if (choice == "1") {
                CSVBookReader reader(full_path, book_repo_);
                auto data = reader.loadFromCSV();
                spdlog::info("Imported {} books from CSV", data.size());
                std::cout << "Imported " << data.size() << " books\n";
                return !data.empty();
            }
            else if (choice == "2") {
                CSVAuthorReader reader(full_path, author_repo_);
                auto data = reader.loadFromCSV();
                spdlog::info("Imported {} authors from CSV", data.size());
                std::cout << "Imported " << data.size() << " authors\n";
                return !data.empty();
            }
            else if (choice == "3") {
                CSVPublisherReader reader(full_path, publisher_repo_);
                auto data = reader.loadFromCSV();
                spdlog::info("Imported {} publishers from CSV", data.size());
                std::cout << "Imported " << data.size() << " publishers\n";
                return !data.empty();
            } else if (choice == "4") {
                CSVGenreReader reader(full_path, genre_repo_);
                auto data = reader.loadFromCSV();
                spdlog::info("Imported {} genres from CSV", data.size());
                std::cout << "Imported " << data.size() << " genres\n";
                return !data.empty();
            }
        }

        spdlog::error("Unsupported file format: {}", full_path);
        std::cout << "Unsupported file format\n";
        return false;
    }
    catch (const std::exception& e) {
        spdlog::error("Error loading file {}: {}", full_path, e.what());
        std::cout << "Error loading file: " << e.what() << "\n";
        return false;
    }
}

void Library::filter(const std::string& choice, const std::string& field, const std::string& direction) {
    spdlog::info("Filtering choice: {}, field: {}, direction: {}", choice, field, direction);
    try {
        if (choice == "1") {
            book_repo_.filter(field, direction);
        }
        else if (choice == "2") {
            author_repo_.filter(field, direction);
        }
        else if (choice == "3") {
            publisher_repo_.filter(field, direction);
        }
        else if (choice == "4") {
            genre_repo_.filter(field, direction);
        }
        else {
            spdlog::warn("Invalid filter choice: {}", choice);
            std::cout << "Invalid entity choice\n";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error filtering: {}", e.what());
        std::cout << "Error filtering: " << e.what() << "\n";
    }
}

int Library::search(const std::string& choice, const std::string& field, const std::string& value) {
    spdlog::info("Searching choice: {}, field: {}, value: {}", choice, field, value);
    try {
        int result = 0;
        if (choice == "1") {
            result = book_repo_.find(field, value);
        }
        else if (choice == "2") {
            result = author_repo_.find(field, value);
        }
        else if (choice == "3") {
            result = publisher_repo_.find(field, value);
        }
        else if (choice == "4") {
            result = genre_repo_.find(field, value);
        }
        else {
            spdlog::warn("Invalid search choice: {}", choice);
            std::cout << "Invalid entity choice\n";
            return 0;
        }
        if (result == 0) {
            std::cout << "No results\n";
        }
        spdlog::info("Found {} results", result);
        return result;
    }
    catch (const std::exception& e) {
        spdlog::error("Error searching: {}", e.what());
        std::cout << "Error searching: " << e.what() << "\n";
        return 0;
    }
}

int Library::addRecord(const std::string& choice, const std::map<std::string, std::string>& record) {
    spdlog::info("Adding record for choice: {}", choice);
    try {
        if (choice == "1") {
            Book book(
                record.at("title"),
                std::stoi(record.at("author_id")),
                record.at("description"),
                std::stoi(record.at("year")),
                std::stoi(record.at("genre_id")),
                std::stoi(record.at("publisher_id")),
                std::stoi(record.at("pages"))
            );
            int id = book_repo_.save(book);
            spdlog::info("Added book: {}", record.at("title"));
            return id;
        }
        else if (choice == "2") {
            Author author(
                record.at("full_name"),
                record.at("date_of_birth"),
                record.at("date_of_death"),
                record.at("biography")
            );
            int id = author_repo_.save(author);
            spdlog::info("Added author: {}", record.at("full_name"));
            return id;
        }
        else if (choice == "3") {
            Publisher publisher(
                record.at("name"),
                record.at("address"),
                record.at("phone"),
                record.at("mail")
            );
            int id = publisher_repo_.save(publisher);
            spdlog::info("Added publisher: {}", record.at("name"));
            return id;
        }
        else if (choice == "4") {
            Genre genre(
                record.at("title"),
                record.at("description")
            );
            int id = genre_repo_.save(genre);
            spdlog::info("Added genre: {}", record.at("title"));
            return id;
        }
        spdlog::warn("Invalid add record choice: {}", choice);
        return -1;
    }
    catch (const std::exception& e) {
        spdlog::error("Error adding record: {}", e.what());
        return -1;
    }
}

bool Library::updateRecord(const std::string& choice, const std::string& field, const std::string& new_val,
    const int& id) {
    spdlog::info("Updating choice: {}, field: {}, new_val: {}, id: {}",
        choice, field, new_val, id);
    try {
        if (choice == "1") {
            return book_repo_.update(field, id, new_val);
        }
        else if (choice == "2") {
            return author_repo_.update(field, id, new_val);
        }
        else if (choice == "3") {
            return publisher_repo_.update(field, id, new_val);
        }
        else if (choice == "4") {
            return genre_repo_.update(field, id, new_val);
        }
        spdlog::warn("Invalid update choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return false;
    }
    catch (const std::exception& e) {
        spdlog::error("Error updating record: {}", e.what());
        std::cout << "Error updating: " << e.what() << "\n";
        return false;
    }
}

bool Library::deleteRecord(const std::string& choice, const std::string& field, const std::string& value) {
    spdlog::info("Deleting choice: {}, field: {}, value: {}", choice, field, value);
    try {
        if (choice == "1") {
            return book_repo_.del(field, value);
        }
        else if (choice == "2") {
            return author_repo_.del(field, value);
        }
        else if (choice == "3") {
            return publisher_repo_.del(field, value);
        }
        else if (choice == "4") {
            return genre_repo_.del(field, value);
        }
        spdlog::warn("Invalid delete choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return false;
    }
    catch (const std::exception& e) {
        spdlog::error("Error deleting record: {}", e.what());
        std::cout << "Error deleting: " << e.what() << "\n";
        return false;
    }
}

void Library::displayAll(const std::string& choice) {
    spdlog::info("Displaying all records for choice: {}", choice);
    try {
        if (choice == "1") {
            book_repo_.showAll();
        }
        else if (choice == "2") {
            author_repo_.showAll();
        }
        else if (choice == "3") {
            publisher_repo_.showAll();
        }
        else if (choice == "4") {
            genre_repo_.showAll();
        }
        else {
            spdlog::warn("Invalid display choice: {}", choice);
            std::cout << "Invalid entity choice\n";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error displaying records: {}", e.what());
        std::cout << "Error displaying: " << e.what() << "\n";
    }
}

void Library::join(const std::string& choice) {
    spdlog::info("Joining for choice: {}", choice);
    try {
        if (choice == "1") {
            joiner_.join("author");
        }
        else if (choice == "2") {
            joiner_.join("publisher");
        }
        else if (choice == "3") {
            joiner_.join("genre");
        }
        else {
            spdlog::warn("Invalid join choice: {}", choice);
            std::cout << "Invalid entity choice\n";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error joining: {}", e.what());
        std::cout << "Error joining: " << e.what() << "\n";
    }
}

void Library::exportData(const std::string& choice, const std::string& format) {
    spdlog::info("Exporting data for choice: {}, format: {}", choice, format);
    try {
        if (choice == "1") {
            book_repo_.exportData(format);
        }
        else if (choice == "2") {
            author_repo_.exportData(format);
        }
        else if (choice == "3") {
            publisher_repo_.exportData(format);
        }
        else if (choice == "4") {
            genre_repo_.exportData(format);
        }
        else {
            spdlog::warn("Invalid export choice: {}", choice);
            std::cout << "Invalid entity choice\n";
        }
    }
    catch (const std::exception& e) {
        spdlog::error("Error exporting data: {}", e.what());
        std::cout << "Error exporting: " << e.what() << "\n";
    }
}


// CLI Functions
void searchMenu(Library& library) {
    spdlog::info("Starting search menu");
    std::map<std::string, std::string> entity_types = {
        {"1", "book"}, {"2", "author"}, {"3", "publisher"}, {"4", "genre"}
    };
    std::map<std::string, std::map<std::string, std::string>> field_options = {
        {"book", {{"1", "title"}, {"2", "author_id"}, {"3", "year"}, {"4", "genre_id"}, {"5", "pages"}, {"6", "publisher_id"}, {"7", "id"}}},
        {"author", {{"1", "full_name"}, {"2", "date_of_birth"}, {"3", "date_of_death"}, {"4", "biography"}, {"5", "id"}}},
        {"publisher", {{"1", "name"}, {"2", "address"}, {"3", "phone"}, {"4", "mail"}, {"5", "id"}}},
        {"genre", {{"1", "title"}, {"2", "description"}, {"3", "id"}}}
    };

    std::cout << "\nSearch by Entity:\n"
        << "1. book\n2. author\n3. publisher\n4. genre\n0. back\n"
        << "Select entity: ";
    std::string choice;
    std::getline(std::cin, choice);
    spdlog::debug("User selected entity: {}", choice);

    if (choice == "0") {
        spdlog::info("Returning to main menu");
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    std::string entity = entity_types[choice];
    std::cout << "\nSearch " << entity << " by:\n";


    for (const auto& pair : field_options[entity]) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }

    std::cout << "0. back\nSelect field: ";
    std::string field_choice;
    std::getline(std::cin, field_choice);

    if (field_choice == "0") {
        return;
    }
    if (field_options[entity].find(field_choice) == field_options[entity].end()) {
        spdlog::warn("Invalid field choice: {}", field_choice);
        std::cout << "Invalid field choice\n";
        return;
    }

    std::string field = field_options[entity][field_choice];
    std::cout << "Enter " << field << ": ";
    std::string query;
    std::getline(std::cin, query);
    spdlog::info("Searching {} by {}: {}", entity, field, query);

    library.search(choice, field, query);
}


void importData(Library& library) {
    spdlog::info("Starting data import");
    std::map<std::string, std::string> entity_types = {
        {"1", "book"}, {"2", "author"}, {"3", "publisher"}, {"4", "genre"}
    };

    std::cout << "\nImport Data for:\n"
        << "1. Books\n2. Authors\n3. Publishers\n4. Genres\n0. back\n"
        << "Select entity: ";
    std::string choice;
    std::getline(std::cin, choice);
    spdlog::debug("User selected entity for import: {}", choice);

    if (choice == "0") {
        spdlog::info("Returning to main menu");
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    std::cout << "Enter path to CSV/JSON file: ";
    std::string path;
    std::getline(std::cin, path);
    if (path.empty()) {
        spdlog::warn("Path not provided");
        std::cout << "Path not provided\n";
        return;
    }

    library.load(path, choice);
}

void addRecordMenu(Library& library) {
    spdlog::info("Starting add record menu");
    std::map<std::string, std::string> entity_types = {
        {"1", "book"}, {"2", "author"}, {"3", "publisher"}, {"4", "genre"}
    };
    std::map<std::string, std::vector<std::string>> field_options = {
        {"book", {"title", "author_id", "year", "genre_id", "pages", "publisher_id", "description"}},
        {"author", {"full_name", "date_of_birth", "date_of_death", "biography"}},
        {"publisher", {"name", "address", "phone", "mail"}},
        {"genre", {"title", "description"}}
    };

    std::cout << "\nAdd Record for:\n";
    for (const auto& pair : entity_types) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }
    std::cout << "0. back\nSelect entity: ";
    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "0") {
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    std::string entity = entity_types[choice];
    std::cout << "\nAdding new " << entity << "\n";
    std::map<std::string, std::string> record;
    for (const auto& field : field_options[entity]) {
        std::cout << "Enter " << field << ": ";
        std::string value;
        std::getline(std::cin, value);
        record[field] = value;
    }

    if (library.addRecord(choice, record) >= 0) {
        std::cout << entity << " added successfully\n";
        spdlog::info("{} added successfully", entity);
    }
    else {
        std::cout << "Error adding the " << entity << "\n";
        spdlog::error("Error adding {}", entity);
    }
}

void updateRecordMenu(Library& library) {
    spdlog::info("Starting update record menu");
    std::map<std::string, std::string> entity_types = {
        {"1", "book"}, {"2", "author"}, {"3", "publisher"}, {"4", "genre"}
    };
    std::map<std::string, std::vector<std::string>> field_options = {
        {"book", {"title", "author_id", "year", "genre_id", "pages", "publisher_id", "description"}},
        {"author", {"full_name", "date_of_birth", "date_of_death", "biography"}},
        {"publisher", {"name", "address", "phone", "mail"}},
        {"genre", {"title", "description"}}
    };

    std::cout << "\nUpdate Record for:\n";
    for (const auto& pair : entity_types) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }
    std::cout << "0. back\nSelect entity: ";
    std::string choice;
    std::getline(std::cin, choice);
    spdlog::info("Updating {}", entity_types[choice]);

    if (choice == "0") {
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    std::string entity = entity_types[choice];
    std::cout << "\nAvailable fields for " << entity << ": ";
    for (const auto& field : field_options[entity]) {
        std::cout << field << ", ";
    }
    std::cout << "\nEnter the field to update: ";
    std::string field;
    std::getline(std::cin, field);

    if (std::find(field_options[entity].begin(), field_options[entity].end(), field) == field_options[entity].end()) {
        spdlog::warn("Invalid field: {}", field);
        std::cout << "Invalid field\n";
        return;
    }

    std::cout << "Enter the new value: ";
    std::string new_val;
    std::getline(std::cin, new_val);

    std::string id;
    if (choice == "1") {
        std::cout << "Enter the id of the book: ";
        std::getline(std::cin, id);
    }
    else {
        std::cout << "Enter the id: ";
        std::getline(std::cin, id);
    }

    bool result = library.updateRecord(choice, field, new_val, std::stoi(id));
    if (result) {
        std::cout << "Successfully updated!\n";
        spdlog::info("Successfully updated {}: {} = {}", entity, field, new_val);
    }
    else {
        std::cout << "No records found to update or error occurred.\n";
        spdlog::info("No {} found or error updating with field = {}", entity, field);
    }
}

void showFullInfo(Library& library) {
    spdlog::info("Starting show full info");
    std::map<std::string, std::string> entity_types = {
        {"1", "author"}, {"2", "publisher"}, {"3", "genre"}
    };

    std::cout << "\nYou want to know more information about:\n";
    for (const auto& pair : entity_types) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }
    std::cout << "0. back\nSelect entity: ";
    std::string choice;
    std::getline(std::cin, choice);
    spdlog::info("Joining {}", entity_types[choice]);

    if (choice == "0") {
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    library.join(choice);
}

void deleteRecordMenu(Library& library) {
    spdlog::info("Starting delete record menu");
    std::map<std::string, std::pair<std::string, std::vector<std::string>>> entity_types = {
        {"1", {"book", {"title", "author_id", "year", "genre_id", "pages", "publisher_id", "id"}}},
        {"2", {"author", {"full_name", "date_of_birth", "date_of_death", "biography", "id"}}},
        {"3", {"publisher", {"name", "address", "phone", "mail", "id"}}},
        {"4", {"genre", {"title", "description", "id"}}}
    };

    std::cout << "\nDelete Record for:\n";
    for (const auto& pair : entity_types) {
        std::cout << pair.first << ". " << pair.second.first << "\n";
    }
    std::cout << "0. back\nSelect entity: ";
    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "0") {
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    auto pair = entity_types[choice];
    auto entity_name = pair.first;
    auto fields = pair.second;
    std::cout << "\nDeleting " << entity_name << " by field\nAvailable fields: ";
    for (const auto& field : fields) {
        std::cout << field << ", ";
    }
    std::cout << "\nEnter the field: ";
    std::string field;
    std::getline(std::cin, field);

    if (std::find(fields.begin(), fields.end(), field) == fields.end()) {
        spdlog::warn("Invalid field: {}", field);
        std::cout << "Invalid field!\n";
        return;
    }

    std::cout << "Enter the value of this field: ";
    std::string value;
    std::getline(std::cin, value);

    bool result = library.deleteRecord(choice, field, value);
    if (result) {
        std::cout << "Successfully deleted!\n";
        spdlog::warn("Successfully deleted {} where {} = {}", entity_name, field, value);
    }
    else {
        std::cout << "No records found to delete.\n";
        spdlog::info("No {} found with {} = {}", entity_name, field, value);
    }
}

void filteringMenu(Library& library) {
    spdlog::info("Starting filtering menu");
    std::map<std::string, std::string> entity_types = {
        {"1", "book"}, {"2", "author"}, {"3", "publisher"}, {"4", "genre"}
    };
    std::map<std::string, std::map<std::string, std::string>> field_options = {
        {"book", {{"1", "title"}, {"2", "author_id"}, {"3", "year"}, {"4", "genre_id"}, {"5", "pages"}, {"6", "publisher_id"}}},
        {"author", {{"1", "full_name"}, {"2", "date_of_birth"}, {"3", "date_of_death"}, {"4", "biography"}}},
        {"publisher", {{"1", "name"}, {"2", "address"}, {"3", "phone"}, {"4", "mail"}}},
        {"genre", {{"1", "title"}, {"2", "description"}}}
    };

    std::cout << "\nFilter by Entity:\n"
        << "1. book\n2. author\n3. publisher\n4. genre\n0. back\n"
        << "Select entity: ";
    std::string choice;
    std::getline(std::cin, choice);
    spdlog::debug("User selected entity: {}", choice);

    if (choice == "0") {
        spdlog::info("Returning to main menu");
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    std::string entity = entity_types[choice];
    std::cout << "\nFilter " << entity << " by:\n";
    for (const auto& pair : field_options[entity]) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }
    std::cout << "0. back\nSelect field: ";
    std::string field_choice;
    std::getline(std::cin, field_choice);

    if (field_choice == "0") {
        return;
    }
    if (field_options[entity].find(field_choice) == field_options[entity].end()) {
        spdlog::warn("Invalid field choice: {}", field_choice);
        std::cout << "Invalid field choice\n";
        return;
    }

    std::cout << "Choose direction:\n1. Ascending\n2. Descending\nSelect direction: ";
    std::string dir;
    std::getline(std::cin, dir);
    spdlog::debug("User selected direction: {}", dir);

    if (dir == "1") {
        library.filter(choice, field_options[entity][field_choice], "up");
    }
    else if (dir == "2") {
        library.filter(choice, field_options[entity][field_choice], "down");
    }
    else {
        spdlog::warn("Invalid direction choice: {}", dir);
        std::cout << "Invalid direction choice\n";
    }
}

void displayRecordsMenu(Library& library) {
    spdlog::info("Starting display records menu");
    std::map<std::string, std::string> entity_types = {
        {"1", "book"}, {"2", "author"}, {"3", "publisher"}, {"4", "genre"}
    };

    std::cout << "\nDisplay Records for:\n";
    for (const auto& pair : entity_types) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }
    std::cout << "0. back\nSelect entity: ";
    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "0") {
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    std::string entity = entity_types[choice];
    library.displayAll(choice);
    spdlog::info("Displayed {} records", entity);
}

void exportDataMenu(Library& library) {
    spdlog::info("Starting export data menu");
    std::map<std::string, std::string> entity_types = {
        {"1", "book"}, {"2", "author"}, {"3", "publisher"}, {"4", "genre"}
    };
    std::map<std::string, std::string> file_types = { {"1", "json"}, {"2", "csv"} };

    std::cout << "\nExport data for:\n";
    for (const auto& pair : entity_types) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }
    std::cout << "0. back\nSelect entity: ";
    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "0") {
        return;
    }
    if (entity_types.find(choice) == entity_types.end()) {
        spdlog::warn("Invalid entity choice: {}", choice);
        std::cout << "Invalid entity choice\n";
        return;
    }

    std::string entity = entity_types[choice];
    std::cout << "\nFile format:\n";
    for (const auto& pair : file_types) {
        std::cout << pair.first << ". " << pair.second << "\n";
    }
    std::cout << "0. back\nSelect format: ";
    std::string file_choice;
    std::getline(std::cin, file_choice);

    if (file_choice == "0") {
        return;
    }
    if (file_types.find(file_choice) == file_types.end()) {
        spdlog::warn("Invalid format choice: {}", file_choice);
        std::cout << "Invalid format choice\n";
        return;
    }

    std::string format = file_types[file_choice];
    library.exportData(choice, format);
    spdlog::info("Exported {} data in {}", entity, format);
}

void mainMenu(Library& library) {
    spdlog::info("Starting main menu");
    while (true) {
        std::cout << "\nLibrary Management System:\n"
            << "1. Import data\n2. Display All Records\n3. Add Record\n4. Update Record\n"
            << "5. Delete Record\n6. Search Records\n7. Filter Records\n8. Get more information\n"
            << "9. Export data\n0. Exit\nSelect an option: ";
        std::string choice;
        std::getline(std::cin, choice);
        spdlog::debug("User selected: {}", choice);

        if (choice == "1") {
            importData(library);
        }
        else if (choice == "2") {
            displayRecordsMenu(library);
        }
        else if (choice == "3") {
            addRecordMenu(library);
        }
        else if (choice == "4") {
            updateRecordMenu(library);
        }
        else if (choice == "5") {
            deleteRecordMenu(library);
        }
        else if (choice == "6") {
            searchMenu(library);
        }
        else if (choice == "7") {
            filteringMenu(library);
        }
        else if (choice == "8") {
            showFullInfo(library);
        }
        else if (choice == "9") {
            exportDataMenu(library);
        }
        else if (choice == "0") {
            spdlog::info("User chose to exit");
            std::cout << "Goodbye!\n";
            break;
        }
        else {
            spdlog::warn("Invalid choice: {}", choice);
            std::cout << "Invalid choice\n";
        }
    }
}