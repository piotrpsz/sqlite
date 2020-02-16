/*------- include files:
-------------------------------------------------------------------*/
#include <iostream>
#include <sstream>
#include <iterator>
#include "Field.h"

/*------- namespaces:
-------------------------------------------------------------------*/
namespace beesoft {
namespace sqlite {
using namespace std;


auto deleter = [](void* ptr) {
    delete[] static_cast<char*>(ptr);
};

/********************************************************************
*                                                                   *
*                     C O N S T R U C T O R S                       *
*                                                                   *
********************************************************************/

/**
 * @brief Field::Field(name)
 * Konstruktor pola typu NULL.
 *
 * @param name - nazwa pola/kolumny
 */
Field::Field(const string& name)
    : name_(name)
    , type_(Type::Null)
    , value_(nullptr, deleter)
{}

/**
 * @brief Field::Field(name, i64)
 * Konstruktor pola typu INT.
 *
 * @param name - nazwa pola/kolumny
 * @param v - wartość pola typu i64
 */
Field::Field(const string& name, const i64 v)
    : name_(name)
    , type_(Type::Int)
    , nbytes_(sizeof(i64))
    , value_(new byte[nbytes_], deleter)
{
    memcpy(value_.get(), &v, nbytes_);
}

Field::Field(const string& name, const bool v)
    : Field(name, v ? i64(1) : i64(0))
{}

Field::Field(const string& name, const f64 v)
    : name_(name)
    , type_(Type::Float)
    , nbytes_(sizeof(f64))
    , value_(new byte[nbytes_], deleter)
{
    memcpy(value_.get(), &v, nbytes_);
}

Field::Field(const std::string& name, const text& v)
    : name_(name)
    , type_(Type::Text)
    , nbytes_(v.size())
    , value_(new byte[nbytes_], deleter)
{
    memcpy(value_.get(), v.c_str(), nbytes_);
}

Field::Field(const std::string& name, const void* const ptr, const int nbytes)
    : name_(name)
    , type_(Type::Blob)
    , nbytes_(nbytes)
    , value_(new byte[nbytes_], deleter)
{
    memcpy(value_.get(), ptr, nbytes_);
}

Field::Field(const std::string& name, const std::vector<char>& value)
    : Field(name, value.data(), value.size())
{}

/********************************************************************
*                                                                   *
*                       S E T T E R S                               *
*                                                                   *
********************************************************************/

void Field::value(const i64 v) {
    type_ = Type::Int;
    nbytes_ = sizeof(i64);
    value_.reset(new byte[nbytes_], deleter);
    memcpy(value_.get(), &v, nbytes_);
}

void Field::value(const bool v) {
    value(v ? i64(1) : i64(0));
}

void Field::value(const f64 v) {
    type_ = Type::Float;
    nbytes_ = sizeof(f64);
    value_.reset(new byte[nbytes_], deleter);
    memcpy(value_.get(), &v, nbytes_);
}

void Field::value(const text& v) {
    type_ = Type::Text;
    nbytes_ = v.size();
    value_.reset(new byte[nbytes_], deleter);
    memcpy(value_.get(), v.c_str(), nbytes_);
}

void Field::value(const void* const ptr, const int nbytes) {
    type_ = Type::Blob;
    nbytes_ = nbytes;
    value_.reset(new byte[nbytes_], deleter);
    memcpy(value_.get(), ptr, nbytes_);
}

void Field::value(const vec& v) {
    value(v.data(), v.size());
}


/********************************************************************
*                                                                   *
*                       G E T T E R S                               *
*                                                                   *
********************************************************************/

i64 Field::as_i64() const {
    if (Type::Int == type_) {
        return *static_cast<i64*>(value_.get());
    }
    const auto errstr = error_string("i64");
    cerr << errstr << endl;
    throw errstr;
}
bool Field::as_bool() const {
    return (as_i64() == 0) ? false : true;
}

f64 Field::as_f64() const {
    if (Type::Float == type_) {
        return *static_cast<f64*>(value_.get());
    }
    const auto errstr = error_string("f64");
    cerr << errstr << endl;
    throw errstr;
}

text Field::as_text() const {
    if (Type::Text == type_) {
        return string(static_cast<char*>(value_.get()), nbytes_);
    }
    const auto errstr = error_string("text");
    cerr << errstr << endl;
    throw errstr;
}

std::vector<char> Field::as_vector() const {
    std::vector<char> data(nbytes_);
    memcpy(&data[0], static_cast<char*>(value_.get()), nbytes_);
    return data;
}

/********************************************************************
*                                                                   *
*                          H E L P E R S                            *
*                                                                   *
********************************************************************/

std::string Field::type_as_string() const {
    switch (type_) {
    case Type::Null:
        return "NULL";
    case Type::Int:
        return "INT";
    case Type::Float:
        return "FLOAT";
    case Type::Text:
        return "TEXT";
    case Type::Blob:
        return "BLOB";
    }
    return "Unknown";
}

std::ostream& operator<<(std::ostream& s, const Field& f) {
    stringstream ss;

    ss << "(";
    ss << "type: " << f.type_as_string() << ", ";
    ss << "name: " << f.name();

    if (f.type() != Type::Null) {
        ss << ", ";
        switch (f.type()) {
        case Type::Int:
            ss << "value: " << f.as_i64();
            break;
        case Type::Float:
            ss << "value: " << f.as_f64();
            break;
        case Type::Text:
            ss << "value: " << f.as_text();
            break;
        default:
            break;
        }
    }
    ss << " [" << f.value_.use_count() << "]";
    ss << ")";

    s << ss.str();
    return s;
}

}}
