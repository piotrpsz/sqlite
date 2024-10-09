//
// Created by piotr on 09.10.24.
//

#pragma once

#include <sqlite3.h>
#include <source_location>
#include <fmt/core.h>

static inline void LOG_ERROR(sqlite3 *const db, std::source_location const sl = std::source_location::current()) noexcept {
    if (auto const err = sqlite3_errcode(db); err != SQLITE_OK)
        fmt::print(stderr, "SQLite Error: {} ({}) => fn::{}().{} [{}]\n",
                   sqlite3_errmsg(db),
                   sqlite3_errcode(db),
                   sl.function_name(),
                   sl.line(),
                   sl.file_name());
}
