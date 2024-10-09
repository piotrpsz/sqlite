//
// Created by piotr on 09.10.24.
//

#pragma once
#include "types.h"
#include "shared.h"
#include <variant>
#include <optional>
#include <fmt/format.h>

class Value {
    std::variant<std::monostate, i64, f64, std::string, std::vector<u8>> data_{};
public:
    enum { MONOSTATE, INTEGER, DOUBLE, STRING, VECTOR };

    Value() = default;
    Value(std::integral auto v) : data_{static_cast<i64>(v)} {}
    Value(std::floating_point auto v) : data_{static_cast<f64>(v)} {}
    Value(std::string v) : data_{std::move(v)} {}
    Value(std::vector<u8> v) : data_{std::move(v)} {}
    ~Value() = default;

    // Special version of constructor for optional values.
    template<typename T>
    explicit Value(std::optional<T> v) noexcept {
        if (v) data_ = v.value();
        else data_ = {};
    }

    // Default Copy and default Move
    Value(Value const&) = default;
    Value& operator=(Value const&) = default;
    Value(Value&&) = default;
    Value& operator=(Value&&) = default;

    auto operator()() && {
        return std::move(data_);
    }
    auto& operator()() const& {
        return data_;
    }

    [[nodiscard]] bool is_null() const noexcept {
        return data_.index() == MONOSTATE;
    }

    [[nodiscard]] uint index() const noexcept {
        return data_.index();
    }

    // Getting values without checking.
    template<typename T>
    [[nodiscard]] T value() const noexcept {
        return std::get<T>(data_);
    }

    template<typename T>
    T value_if() const noexcept {
        if (auto ip = std::get_if<T>(&data_))
            return *ip;
        return T();
    }

    [[nodiscard]] std::string to_string() const noexcept {
        switch (data_.index()) {
            case MONOSTATE:
                return "NULL"s;
            case INTEGER:
                return fmt::format("i64{{{}}}", value<i64>());
            case DOUBLE:
                return fmt::format("f64{{{}}}", value<f64>());
            case STRING:
                return fmt::format("string{{{}}}", value<std::string>());
            case VECTOR: {
                return fmt::format("blob{{{}}}", shared::hex_bytes_as_str(value<std::vector<u8>>()));
            }
            default:
                return "?"s;
        }
    }
};