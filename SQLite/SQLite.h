#ifndef SQLITE_H
#define SQLITE_H

/*------- include files:
-------------------------------------------------------------------*/
#include <sqlite3.h>
#include <string>
#include <functional>

/*------- namespaces:
-------------------------------------------------------------------*/
namespace beesoft {
namespace sqlite {

class Field;

class SQLite {
    static constexpr int HeaderSize = 16;
    static const char ValidHeader[HeaderSize];

    sqlite3 *db;
public:
    static SQLite& shared() {
        static SQLite instance;
        return instance;
    }
private:
    SQLite();
    ~SQLite();
    SQLite(const SQLite&) = delete;
    SQLite& operator=(const SQLite&) = delete;

public:
    bool open(const std::string&);
    bool create(const std::string&, const std::function<bool(SQLite&)>&, const bool = false);
    bool close();
    bool exec(const std::string&);
    int  insert(const std::string&, const std::vector<Field>&);
    bool update(const std::string&, const std::vector<Field>&);
    std::vector<std::vector<Field>> select(const std::string&);

private:
    bool remove_file(const std::string&) const;
    bool file_exists(const std::string&) const;
    bool can_read_from(const std::string&) const;
    bool can_write_to(const std::string&) const;
    bool is_database_file(const std::string&) const;
    void log_error(const std::string& = __BASE_FILE__, const int = __LINE__, const std::string& = __FUNCTION__);

    friend class Statement;
};

}} // namespace end
#endif // SQLITE_H__func__
