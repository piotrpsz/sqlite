//
// Created by piotr on 09.10.24.
//

#include "sqlite.h"
#include "logger.h"
#include "value.h"
#include "query.h"
#include "row.h"

// Close database (if needed and possible).
bool SQLite::close() noexcept {
    if (db_) {
        if (sqlite3_close_v2(db_) != SQLITE_OK) {
            LOG_ERROR(db_);
            return {};
        }
        db_ = nullptr;
    }
    return true;
}

// Open database with given path.
bool SQLite::open(std::string const& path, bool const read_only) noexcept {
    if (db_) {
        fmt::print(stderr, "Database is already opened!\n");
        return false;
    }
    if (path == IN_MEMORY) {
        fmt::print(stderr, "Database in memory can't be opened (use create).\n");
        return false;
    }

    auto const flags = read_only ? SQLITE_READONLY : SQLITE_OPEN_READWRITE;
    if (SQLITE_OK == sqlite3_open_v2(path.c_str(), &db_, flags, nullptr)) {
        fmt::print("database opened: {}\n", path);
        return true;
    }
    LOG_ERROR(db_);
    db_ = nullptr;
    return {};
}

// Create a new database file.
bool SQLite::create(std::string const& path, std::function<bool(SQLite const&)> const& fn, bool overwrite) noexcept {
    if (db_) {
        fmt::print(stderr, "Database is already opened\n");
        return {};
    }
    if (!fn) {
        fmt::print(stderr, "Operations to be performed on created database were not specified\n");
        return {};
    }

    std::error_code err{};
    // If it is not an in-memory database, create a new database file.
    // If the file already exists, delete it first if the user so desires.
    if (path != IN_MEMORY) {
        if (fs::exists(path, err)) {
            if (overwrite) { // is this what the user wants?
                if (!fs::remove(path)) {
                    fmt::print(stderr, "database file could not be deleted\n");
                    return false;
                }
            }
        }
        else if (err)
            fmt::print(stderr, "database already exist {}\n", err.message());
    }

    constexpr auto flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
    if (SQLITE_OK == sqlite3_open_v2(path.c_str(), &db_, flags, nullptr)) {
        if (!fn(*this))
            return false;
        fmt::print("The database created successfully: {}\n", path);
        return true;
    }
    LOG_ERROR(db_);
    return {};
}
