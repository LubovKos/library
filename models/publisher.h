#pragma once
#include <string>
#include <stdexcept>
#include <regex>

struct Publisher {
    std::string name;
    std::string address;
    std::string phone;
    std::string mail;
    int id;

    Publisher(const std::string& n, const std::string& addr, const std::string& ph, const std::string& m, const int& id = -1)
        : name(n), address(addr), phone(ph), mail(m), id(id) {
        validate();
    }

private:
    void validate() {
        if (name.empty()) {
            throw std::invalid_argument("Publisher name must not be empty");
        }
        std::regex email_pattern(R"(^\S+@\S+\.\S+$)");
        if (!std::regex_match(mail, email_pattern)) {
            throw std::invalid_argument("Incorrect mail");
        }
    }
};