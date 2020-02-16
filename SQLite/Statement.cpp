#include <iostream>
#include <sstream>
#include "Statement.h"
#include "Field.h"

/*------- namespaces:
-------------------------------------------------------------------*/
namespace beesoft {
namespace sqlite {
using namespace std;



vector<vector<Field>> Statement::select(const string& query) {
    Result result;

    if (sqlite3_prepare_v2(sqlite_.db, query.c_str(), query.size(), &stmt_, nullptr) == SQLITE_OK) {
        if (const int column_count = sqlite3_column_count(stmt_); column_count > 0) {
            while (SQLITE_ROW == sqlite3_step(stmt_)) {
                Row row;
                for (int i = 0; i < column_count; i++) {
                    const string column_name = sqlite3_column_name(stmt_, i);
                    switch (sqlite3_column_type(stmt_, i)) {
                    case SQLITE_INTEGER:
                        row.emplace_back(column_name,
                                         i64(sqlite3_column_int64(stmt_, i)));
                        break;
                    case SQLITE_FLOAT:
                        row.emplace_back(column_name,
                                         f64(sqlite3_column_double(stmt_, i)));
                        break;
                    case SQLITE3_TEXT:
                        row.emplace_back(column_name,
                                         text(reinterpret_cast<const char*>(sqlite3_column_text(stmt_, i)),
                                              sqlite3_column_bytes(stmt_, i)));

                        break;
                    case SQLITE_BLOB:
                        row.emplace_back(column_name,
                                         sqlite3_column_blob(stmt_, i),
                                         sqlite3_column_bytes(stmt_, i));
                        break;
                    case SQLITE_NULL:
                        row.emplace_back(column_name);
                        break;
                    }
                }
                if (row.size()) {
                    result.push_back(row);
                }
            }
        }
    }
    if (SQLITE_OK == sqlite3_finalize(stmt_)) {
        return result;
    }
    sqlite_.log_error();
    return Result();
}

bool Statement::update(const string& table, const vector<Field>& fields) {
    if (const int n = fields.size(); n > 1) { // co najmniej 2 pola: id + coś
        const int last = fields.size() - 1;

        stringstream ss_query;
        ss_query << "UPDATE " << table << " SET ";
        for (int i = 1; i < last; i++) {
            ss_query << fields[i].name() << "=" << fields[i].bind_name() << ",";
        }
        ss_query << fields[last].name() << "=" << fields[last].bind_name()
                 << " WHERE " << fields[0].name() << "=" << fields[0].as_i64();
        const string query = ss_query.str();

        if (sqlite3_prepare_v2(sqlite_.db, query.c_str(), query.size(), &stmt_, nullptr) == SQLITE_OK) {
            for (int i = 0; i < n; i++) {
                const auto& f = fields[i];
                if (const int idx = sqlite3_bind_parameter_index(stmt_, f.bind_name().c_str()); idx != 0) {
                    switch (f.type()) {
                    case Type::Null:
                        sqlite3_bind_null(stmt_, idx);
                        break;
                    case Type::Int:
                        sqlite3_bind_int64(stmt_, idx, f.as_i64());
                        break;
                    case Type::Float:
                        sqlite3_bind_double(stmt_, idx, f.as_f64());
                        break;
                    case Type::Text: {
                        const auto text = f.as_text();
                        sqlite3_bind_text(stmt_, idx, text.c_str(), text.size(), SQLITE_TRANSIENT); }
                        break;
                    case Type::Blob: {
                        const auto vector = f.as_vector();
                        sqlite3_bind_blob(stmt_, idx, vector.data(), vector.size(), nullptr); }
                        break;
                    }
                }
            }
            if (sqlite3_step(stmt_) == SQLITE_DONE) {
                if (sqlite3_finalize(stmt_) == SQLITE_OK) {
                    return true;
                }
            }
            sqlite_.log_error();
            return false;
        }
    }
    return false;
}

int Statement::insert(const string& table, const vector<Field>& fields) {
   if (const int n = fields.size() - 1; n > 0) {
        string names, binds;
        for (int i = 0; i < n; i++) {
            const Field& f = fields[i];
            names += (f.name() + ",");
            binds += (f.bind_name() + ",");
        }
        const Field& f = fields[n];
        names += (f.name());
        binds += (f.bind_name());

        string query("INSERT INTO ");
        query.append(table);
        query.append(" (");
        query.append(names);
        query.append(") VALUES (");
        query.append(binds);
        query.append(")");

        if (sqlite3_prepare_v2(sqlite_.db, query.c_str(), query.size(), &stmt_, nullptr) == SQLITE_OK) {
            for (auto i = 0; i < int(fields.size()); i++) {
                const auto& f = fields[i];
                if (const int idx = sqlite3_bind_parameter_index(stmt_, f.bind_name().c_str()); idx != 0) {
                    switch (f.type()) {
                    case Type::Null:
                        sqlite3_bind_null(stmt_, idx);
                        break;
                    case Type::Int:
                        sqlite3_bind_int64(stmt_, idx, f.as_i64());
                        break;
                    case Type::Float:
                        sqlite3_bind_double(stmt_, idx, f.as_f64());
                        break;
                    case Type::Text: {
                        const auto text = f.as_text();
                        sqlite3_bind_text(stmt_, idx, text.c_str(), text.size(), SQLITE_TRANSIENT); }
                        break;
                    case Type::Blob: {
                        const auto vector = f.as_vector();
                        sqlite3_bind_blob(stmt_, idx, vector.data(), vector.size(), nullptr); }
                        break;
                    }
                }
            }
            if (sqlite3_step(stmt_) == SQLITE_DONE) {
                if (sqlite3_finalize(stmt_) == SQLITE_OK) {
                    return sqlite3_last_insert_rowid(sqlite_.db);
                }
            }
            sqlite_.log_error();
            return -1;
        }
    }
    return -1;
}



}} // namespaces end
