#ifndef BEESOFT_SQLITE_FIELD_H
#define BEESOFT_SQLITE_FIELD_H

/*------- include files:
-------------------------------------------------------------------*/
#include <cstdint>
#include <string>
#include <vector>
#include <any>
#include <string.h>
#include <memory>

/*------- namespaces:
-------------------------------------------------------------------*/
namespace beesoft {
namespace sqlite {

/*------- types:
-------------------------------------------------------------------*/
using i64  = int64_t;
using f64  = double;
using text = std::string;
using vec  = std::vector<char>;

enum class Type {
    Null = 0,
    Int,
    Float,
    Text,
    Blob
};

class Field{
    std::string name_;
    Type type_;
    int nbytes_;
    std::shared_ptr<void> value_;

public:
    ~Field() = default;

    explicit Field(const std::string&);
    explicit Field(const std::string&, const i64);
    explicit Field(const std::string&, const bool);
    explicit Field(const std::string&, const f64);
    explicit Field(const std::string&, const text&);
    explicit Field(const std::string&, const void* const, const int);
    explicit Field(const std::string&, const vec&);

    Type type() const {
        return type_;
    }
    std::string name() const {
        return name_;
    }
    int size() const {
        return nbytes_;
    }
    std::string bind_name() const {
        return std::string(":") + name_;
    }
    void name(const std::string& name) {
        name_ = name;
    }

    // Setters
    void value(const i64);
    void value(const bool);
    void value(const f64);
    void value(const text&);
    void value(const void* const, const int);
    void value(const vec&);
    // Getters
    i64  as_i64() const;
    bool as_bool() const;
    f64  as_f64() const;
    text as_text() const;
    vec  as_vector() const;

private:
    std::string type_as_string() const;

    std::string error_string(const std::string& marker) const {
        return std::string("Error: SQLite field value conversion to '")
                + marker
                + "' impossible";
    }

    // friends
    friend std::ostream& operator<<(std::ostream&, const Field&);
};

}} // namespace end
#endif // BEESOFT_SQLITE_FIELD_H
