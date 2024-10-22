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
#include <vector>
#include <algorithm>
#include "value.h"

class Query {
    std::string cmd_;
    std::vector<Value> values_;
    static constexpr char QUERY_MARKER{'Q'};

public:
    Query() = default;
    ~Query() = default;
    Query(Query const&) = default;
    Query(Query&&) = default;
    Query& operator=(Query const&) = default;
    Query& operator=(Query&&) = default;

    /// Query for name (without arguments).
    explicit Query(std::string cmd) : cmd_{std::move(cmd)}, values_{} {}

    /// Query for name with arguments.
    Query(std::string cmd, std::vector<Value> data): cmd_{std::move(cmd)}, values_{std::move(data)} {}

    /// Query for name and arguments with fold-expression
    template<typename... T>
    explicit Query(std::string cmd, T... args) : cmd_{std::move(cmd)} {
        (..., values_.push_back(Value(args)));
    }

    void add_arg(Value&& v) {
        values_.push_back(std::move(v));
    }

    /// Serialization. Converting a Query to bytes.
    [[nodiscard]] std::vector<char> to_bytes() const;
    [[nodiscard]] std::vector<char> to_gzip_bytes() const;

    /// Deserialization. Recreate Query from bytes.
    static std::pair<Query,size_t> from_bytes(std::span<char> span);
    static std::pair<Query,size_t> from_gzip_bytes(std::span<const char> span);

    bool operator==(Query const& rhs) const {
        if (cmd_ != rhs.cmd_)
            return false;
        if (values_ != rhs.values_)
            return false;
        return true;
    }

    /// Check if query is valid. \n
    /// The query is valid if the number of placeholders(?) is equal
    /// to the number of query arguments.
    [[nodiscard]] bool valid() const {
        auto placeholder_count = std::ranges::count_if(cmd_, [](char const c) {
            return '?' == c;
        });
        if (std::cmp_not_equal(placeholder_count, values_.size())) {
            fmt::print(stderr, "The number of placeholders and arguments does not match ({}, {})\n", placeholder_count, values_.size());
            return {};
        }
        return true;
    }

    [[nodiscard]] char const* c_str() const noexcept {
        return cmd_.c_str();
    }
    /// Return reference to query itself.
    [[nodiscard]] std::string const& cmd() const {
        return cmd_;
    }
    /// Return refernce to arguments.
    [[nodiscard]] std::vector<Value> const& values() const {
        return values_;
    }

    /// Create text representation of the query.
    [[nodiscard]] std::string to_string() const {
        auto buffer{cmd_};
        for (auto const& value : values_) {
            buffer.append("\n\t");
            buffer.append(value.to_string());
        }
        buffer.shrink_to_fit();
        return buffer;
    }
};
