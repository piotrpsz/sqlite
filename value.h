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
#include "shared.h"
#include <variant>
#include <optional>
#include <span>
#include <fmt/format.h>
#include <range/v3/all.hpp>
namespace rng = ranges;

class Value {
    std::variant<std::monostate, i64, f64, std::string, std::vector<u8>> data_{};
public:
    enum { MONOSTATE, INTEGER, DOUBLE, STRING, VECTOR };

    Value() = default;
    ~Value() = default;

    // Constructors dedicated to acceptable value types
    explicit Value(std::integral auto v) : data_{static_cast<i64>(v)} {}
    explicit Value(std::floating_point auto v) : data_{static_cast<f64>(v)} {}
    explicit Value(std::string v) : data_{std::move(v)} {}
    explicit Value(std::vector<u8> v) : data_{std::move(v)} {}

    /// Constructor dedicated to optional values
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

    /// Check if the object not contains a value.
    [[nodiscard]] bool is_null() const noexcept {
        return data_.index() == MONOSTATE;
    }

    /// Take the index of the contained value.
    /// i.e. MONOSTATE, INTEGER, DOUBLE, STRING, VECTOR
    [[nodiscard]] uint index() const noexcept {
        return data_.index();
    }

    /// Serialization. Converting a Field to bytes.
    [[nodiscard]] auto to_bytes() const noexcept
    -> std::vector<char>;

    /// Deserialization. Recreate Field from bytes.
    static auto from_bytes(std::span<const char> span) noexcept
    -> std::pair<Value,size_t>;

    /// Serialized data info. Generally for debug.
    static auto serialized_data(std::span<char> span) noexcept
    -> std::string;

    /// Get integral value without checking.
    template<std::integral T>
    [[nodiscard]] T value() const noexcept {
        return static_cast<T>(std::get<i64>(data_));
    }
    /// Get floating point value without checking.
    template<std::floating_point T>
    [[nodiscard]] T value() const noexcept {
        return static_cast<T>(std::get<f64>(data_));
    }

    /// Get value without check.
    template<typename T>
    [[nodiscard]] T value() const noexcept {
        return std::get<T>(data_);
    }

    /// Get optional values.
    template<typename T>
    std::optional<T> value_if() const noexcept {
        if (auto ip = std::get_if<T>(&data_))
            return *ip;
        return {};
    }

    bool operator==(Value const& rhs) const noexcept {
        return (data_.index() == rhs.data_.index()) && (data_ == rhs.data_);
    }
    bool operator!=(Value const& rhs) const noexcept {
        return !operator==(rhs);
    }

    /// Create text representation.
    [[nodiscard]] std::string to_string() const noexcept;

private:
    /// Check if it is valid marker.
    static auto is_marker(char c) noexcept -> bool;

    /// Return marker for current value;
    [[nodiscard]] auto marker() const noexcept -> char;

    /// Return serialize value.
    [[nodiscard]] auto value_to_bytes() const noexcept -> std::vector<char>;
};
