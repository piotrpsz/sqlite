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
#include <string>
#include <utility>
#include <optional>
#include <unordered_map>
#include "field.h"

class Row {
    std::unordered_map<std::string,Field> data_;
public:
    Row() = default;
    ~Row() = default;
    Row(Row const&) = default;
    Row& operator=(Row const&) = default;
    Row(Row&&) = default;
    Row& operator=(Row&&) = default;

    Row(std::string name, Value value) {
        const auto f = Field{std::move(name), std::move(value)};
        data_[f.name()] = f;
    }

    bool empty() const {
        return data_.empty();
    }
    std::optional<Field> operator[](std::string const& name) {
        if (data_.contains(name)) return data_[name];
        return {};
    }
    auto size() const {
        return data_.size();
    }

    Row& add(Field const& f) noexcept {
        data_[f.name()] = f;
        return *this;
    }
    Row& add(std::string name, Value value) noexcept {
        Field const f{std::move(name), std::move(value)};
        return add(f);
    }
    Row& add(std::string name) noexcept {
        Field f{std::move(name), {}};
        return add(f);
    }
    template<typename T>
    Row& add(std::string name, std::optional<T> value) noexcept {
        return value
               ? add(std::move(name), std::move(*value))
               : add(std::move(name));
    }

    auto operator==(Row const& rhs) const -> bool;
    auto operator!=(Row const& rhs) const -> bool {
        return !(*this == rhs);
    }

    /// Serialization. Converting a Field to bytes.
    auto to_bytes() const -> std::vector<char>;

    /// Deserialization. Recreate Field from bytes.
    static auto from_bytes(std::span<const char> span) -> std::pair<Row,size_t>;

    /// Serialized data info. Generally for debug.
    static auto serialized_data(std::span<char> span) -> std::string;

    auto to_string() -> std::string;

    /****************************************************************
    *                                                               *
    *                      I T E R A T O R S                        *
    *                                                               *
    ****************************************************************/

    using iterator = std::unordered_map<std::string,Field>::iterator;
    using const_iterator = std::unordered_map<std::string,Field>::const_iterator;
    iterator begin() noexcept { return data_.begin(); }
    iterator end() noexcept { return data_.end(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    const_iterator cend() const noexcept { return data_.cend(); }

    /// We store pairs (name, field) in a map, we separate the data into two vectors.
    /// Separate names, separate fields.
    std::pair<std::vector<std::string>, std::vector<Value>> split() const noexcept {
        std::vector<std::string> names{};
        std::vector<Value> values{};

        if (!data_.empty()) {
            auto const n = data_.size();
            names.reserve(n);
            values.reserve(n);
            for (auto const& [name, field] : data_) {
                names.emplace_back(name);
                values.emplace_back(field.value());
            }
        }
        return {names, values};
    }
};
