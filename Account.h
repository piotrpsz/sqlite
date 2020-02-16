#ifndef POSTMAN_ACCOUNT_H
#define POSTMAN_ACCOUNT_H

/*------- include files:
-------------------------------------------------------------------*/
#include <string>
#include <iostream>
#include <vector>
#include "SQLite/Field.h"

/*
"CREATE TABLE account ("
"id INTEGER PRIMARY KEY,"
"name TEXT NOT NULL,"
"is_default INTEGER CHECK(is_default=0 OR is_default=1)"
")";
*/

class Account {
    int id_;
    std::string name_;
    bool is_default_;

public:
    Account(const std::string&, const bool = false);
    Account(const int, const std::string&, const bool = false);
    Account(const std::vector<beesoft::sqlite::Field>&);
    ~Account() = default;

    bool save();
    bool insert(const std::vector<beesoft::sqlite::Field>&);
    bool update(const std::vector<beesoft::sqlite::Field>&);

    int id() {
        return id_;
    }
    std::string name() const {
        return name_;
    }
    void name(const std::string& text) {
        name_ = text;
    }
    bool is_default() const {
        return is_default_;
    }
    void is_default(const bool flag) {
        is_default_ = flag;
    }


private:
    std::vector<beesoft::sqlite::Field> fields() const;

    // friends
    friend std::ostream& operator<<(std::ostream&, const Account&);
};

#endif // POSTMAN_ACCOUNT_H
