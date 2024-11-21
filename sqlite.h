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
#pragma once

/*------- include files:
-------------------------------------------------------------------*/
#include "types.h"
#include "query.h"
#include "stmt.h"
#include <array>
#include <functional>
#include <sqlite3.h>

class SQLite {
    static inline std::array<u8,16> HEADER = {
        0x53, 0x51, 0x4c, 0x69, 0x74, 0x65, 0x20, 0x66,
        0x6f, 0x72, 0x6d, 0x61, 0x74, 0x20, 0x33, 0x00
    };
    sqlite3 *db_ = nullptr;
public:
    static constexpr i64 INVALID_ROWID = -1;
    static inline Str IN_MEMORY = ":memory:";

    /// Implemented as singleton
    static SQLite& self() noexcept {
        static auto db = SQLite{};
        return db;
    }
    /// No Copy
    SQLite(SQLite const&) = delete;
    SQLite& operator=(SQLite const&) = delete;
    /// No Move
    SQLite(SQLite&&) = delete;
    SQLite& operator=(SQLite&&) = delete;

    static std::string version() noexcept {
        return sqlite3_version;
    }
    bool close() noexcept;
    bool open(std::string const& path, bool expected_success = false, bool read_only = false) noexcept;
    bool create(std::string const&  path, std::function<bool(SQLite const&)> const& fn, bool overwrite = false) noexcept;

    //------- EXEC ----------
    [[nodiscard]] bool exec(Query const& query) const {
       return Stmt(db_).exec(query);
    }
    template<typename... T>
    bool exec(std::string const& query_str, T... args) const {
        return exec(Query{query_str, args...});
    }

    //------- INSERT ----------
    [[nodiscard]] i64 insert(Query const& query) const {
        if (Stmt stmt(db_); stmt.exec(query))
            return sqlite3_last_insert_rowid(db_);;
        return INVALID_ROWID;
    }
    template<typename... T>
    [[nodiscard]] i64 insert(std::string const& query_str, T... args) const {
        return insert(Query{query_str, args...});
    }

    //------- UPDATE ----------
    [[nodiscard]] bool update(Query const& query) const {
        return Stmt(db_).exec(query);
    }
    template<typename... T>
    bool update(std::string const& query_str, T... args ) const {
        return update(Query{query_str, args...});
    }

    //------- SELECT ----------
    [[nodiscard]] std::optional<Result> select(Query const& query) const {
        return Stmt(db_).exec_with_result(query);
    }
    template<typename... T>
    std::optional<Result> select(std::string const& query_str, T... args ) const {
        return select(Query{query_str, args...});
    }

private:
    SQLite() {
        sqlite3_initialize();
    }
};

