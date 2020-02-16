
#include <sstream>
#include "SQLite/SQLite.h"
#include "Account.h"

using namespace std;
namespace bs = beesoft::sqlite;

Account::Account(const int id, const std::string& name, const bool is_default)
    : id_(id)
    , name_(name)
    , is_default_(is_default)
{}

Account::Account(const string& name, const bool is_default)
    : Account(0, name, is_default)
{}

Account::Account(const vector<bs::Field>& data) {
    const int n = data.size();
    if (n) {
        for (int i = 0; i < n; i++) {
            const auto& f = data[i];
            const auto& name = f.name();
            if (name == "id") {
                id_ = f.as_i64();
                continue;
            }
            if (name == "name") {
                name_ = f.as_text();
                continue;
            }
            if (name == "is_default") {
                is_default_ = f.as_bool();
            }
        }
    }
}


bool Account::save() {
    auto const fs = fields();
    return (id_ == 0) ? insert(fs) : update(fs);
}

bool Account::insert(const vector<bs::Field>& fs) {
    if (const int retv = bs::SQLite::shared().insert("account", fs); retv != -1) {
        id_ = retv;
        return true;
    }
    return false;
}

bool Account::update(const vector<bs::Field>& fs) {
    return bs::SQLite::shared().update("account", fs);
}




std::vector<bs::Field> Account::fields() const {
    vector<bs::Field> v;
    v.reserve(id_ ? 3 : 2);

    if (id_) {
        v.emplace_back("id", bs::i64(id_));
    }
    v.emplace_back("name", name_);
    v.emplace_back("is_default", is_default_);

    return v;
}

std::ostream& operator<<(std::ostream& s, const Account& account) {
    stringstream ss;

    ss << "(";
    ss << "id: " << account.id_ << ", ";
    ss << "name: " << account.name_ << ", ";
    ss << "is_default: " << (account.is_default_ ? "true" : "false");
    ss << ")";

    s << ss.str();
    return s;
}
