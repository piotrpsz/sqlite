//
// Created by piotr on 09.10.24.
//

#pragma once
#include <optional>
#include <sqlite3.h>
#include "query.h"
#include "result.h"

class Stmt {
    sqlite3* db_{};
    sqlite3_stmt* stmt_{};
public:
    Stmt() = delete;
    ~Stmt();
    Stmt(Stmt const&) = default;
    Stmt(Stmt&&) = default;
    Stmt& operator=(Stmt const&) = default;
    Stmt& operator=(Stmt&&) = default;

    explicit Stmt(sqlite3* db) : db_(db) {}

    /// Execute query without return data.
    bool exec(Query const& query);

    /// Execute a query that returns the result
    std::optional<Result> exec_with_result(Query const& query);
};
