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
#include "value.h"


class Field {
    std::pair<std::string, Value> data_;
public:
    Field() = default;
    ~Field() = default;
    Field(Field const&) = default;
    Field(Field&&) = default;
    Field& operator=(Field const&) = default;
    Field& operator=(Field&&) = default;

    explicit Field(std::string&& name) : data_{name, {}} {}
    Field(std::string&& name, Value&& value) : data_{name, value} {}
    explicit Field(std::pair<std::string, Value> data) : data_{std::move(data)} {}

    auto operator()() && {
        return std::move(data_);
    }
    auto const& operator()() const& {
        return data_;
    }
    [[nodiscard]] auto const& name() const {
        return data_.first;
    }
    [[nodiscard]] auto const& value() const {
        return data_.second;
    }

    [[nodiscard]] auto to_string() const -> std::string;

    /// Serialization. Converting a Field to bytes.
    [[nodiscard]] auto to_bytes() const -> std::vector<char>;

    /// Deserialization. Recreate Field from bytes.
    static std::pair<Field,size_t> from_bytes(std::span<const char> span);

    /// Serialized data info. Generally for debug.
    static auto serialized_data(std::span<char> span) -> std::string;

    bool operator==(Field const& rhs) const {
        return data_ == rhs.data_;
    }
    bool operator!=(Field const& rhs) const {
        return data_ != rhs.data_;
    }

};
