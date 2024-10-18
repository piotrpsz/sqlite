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
#include "types.h"
#include "stmt.h"
#include "logger.h"
#include "row.h"
#include "value.h"

/*------- forward declarations:
-------------------------------------------------------------------*/
Row fetch_row_data(sqlite3_stmt* stmt, int column_count) noexcept;
bool bind2stmt(sqlite3_stmt* stmt, std::vector<Value> const& args) noexcept;
bool bind_at(sqlite3_stmt* stmt, int idx, Value const& v) noexcept;

Stmt::~Stmt() {
    if (stmt_) {
        if (SQLITE_OK == sqlite3_finalize(stmt_)) {
            stmt_ = nullptr;
            return;
        }
        LOG_ERROR(db_);
    }
}

bool Stmt::exec(Query const &query) {
    if (query.valid()) {
        if (SQLITE_OK == sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt_, nullptr)) {
            if (bind2stmt(stmt_, query.values())) {
                if (SQLITE_DONE == sqlite3_step(stmt_)) {
                    if (SQLITE_OK == sqlite3_finalize(stmt_)) {
                        stmt_ = nullptr;
                        return true;
                    }
                }
            }
        }
    }
    LOG_ERROR(db_);
    return {};
}

std::optional<Result> Stmt::exec_with_result(Query const& query) {
    if (!query.valid()) {
        return {};
    }

    Result result{};
    if (SQLITE_OK == sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt_, nullptr)) {
        if (bind2stmt(stmt_, query.values())) {
            if (auto n = sqlite3_column_count(stmt_)) {
                while (SQLITE_ROW == sqlite3_step(stmt_)) {
                    if (auto row = fetch_row_data(stmt_, n); !row.empty()) {
                        result.add(std::move(row));
                    }
                }
            }
        }
    }

    if (SQLITE_DONE == sqlite3_errcode(db_)) {
        if (SQLITE_OK == sqlite3_finalize(stmt_)) {
            stmt_ = nullptr;
            return std::move(result);
        }
    }

    LOG_ERROR(db_);
    return {};
}


//*******************************************************************
//*                                                                 *
//*              H E L P E R   C   F U N C T I O N S                *
//*                                                                 *
//*******************************************************************

Row fetch_row_data(sqlite3_stmt* const stmt, int const column_count) noexcept {
    Row row{};

    for (auto i = 0; i < column_count; ++i) {
        std::string name = sqlite3_column_name(stmt, i);
        switch (sqlite3_column_type(stmt, i)) {
            case SQLITE_NULL:
                row.add(std::move(name));
                break;
            case SQLITE_INTEGER: {
                Value v{sqlite3_column_int64(stmt, i)};
                row.add(std::move(name), std::move(v));
                break;
            }
            case SQLITE_FLOAT: {
                Value v{sqlite3_column_double(stmt, i)};
                row.add(std::move(name), std::move(v));
                break;
            }
            case SQLITE_TEXT: {
                std::string text{reinterpret_cast<const char *>(sqlite3_column_text(stmt, i))};
                Value v{std::move(text)};
                row.add(std::move(name), std::move(v));
                break;
            }
            case SQLITE_BLOB: {
                auto const ptr { static_cast<u8 const*>(sqlite3_column_blob(stmt, i))};
                auto const size{ sqlite3_column_bytes(stmt, i)};
                Value v{std::move(std::vector<u8>{ptr, ptr + size})};
                row.add(std::move(name), std::move(v));
            }
            default: ;
        }
    }
    return row;
}

bool bind2stmt(sqlite3_stmt* const stmt, std::vector<Value> const& args) noexcept {
    if (!args.empty()) {
        auto idx = 0;
        for (auto const& v: args)
            if (!bind_at(stmt, ++idx, v))
                return {};
    }
    return true;
}

bool bind_at(sqlite3_stmt* const stmt, int const idx, Value const& v) noexcept {
    switch (v.index()) {
        case Value::MONOSTATE:
            return SQLITE_OK == sqlite3_bind_null(stmt, idx);
        case Value::INTEGER:
            return SQLITE_OK == sqlite3_bind_int64(stmt, idx, static_cast<sqlite3_int64>(v.value<i64>()));
        case Value::DOUBLE:
            return SQLITE_OK == sqlite3_bind_double(stmt, idx, v.value<f64>());
        case Value::STRING: {
            auto const text = v.value<std::string>();
            return SQLITE_OK == sqlite3_bind_text(stmt, idx, text.c_str(), -1, SQLITE_TRANSIENT); }
        case Value::VECTOR: {
            auto const vec = v.value<std::vector<u8>>();
            auto const n{ static_cast<int>(vec.size())};
            return SQLITE_OK == sqlite3_bind_blob(stmt, idx, vec.data(), n, SQLITE_TRANSIENT); }
        default:
            return false;
    }
}
