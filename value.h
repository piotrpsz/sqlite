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
    template<std::integral T>
    [[nodiscard]] T value() const noexcept {
        return static_cast<T>(std::get<i64>(data_));
    }
    template<std::floating_point T>
    [[nodiscard]] T value() const noexcept {
        return static_cast<T>(std::get<f64>(data_));
    }
    template<typename T>
    [[nodiscard]] T value() const noexcept {
        return std::get<T>(data_);
    }

    template<typename T>
    std::optional<T> value_if() const noexcept {
        if (auto ip = std::get_if<T>(&data_))
            return *ip;
        return {};
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