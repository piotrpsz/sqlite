// MIT License
//
// Copyright (c) 2024 Piotr Pszczółkowski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Created by Piotr Pszczółkwski on 09.10.2024 (piotr@beesoft.pl).
//

/*------- include files:
-------------------------------------------------------------------*/
#include "sqlite.h"
#include "logger.h"
#include "value.h"
#include "query.h"

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
bool SQLite::open(std::string const& path, bool const expected_success, bool const read_only) noexcept {
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
    if (expected_success) LOG_ERROR(db_);
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
