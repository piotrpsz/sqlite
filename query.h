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
    std::string query_;
    std::vector<Value> values_;
public:
    Query() = default;
    ~Query() = default;
    Query(Query const&) = default;
    Query(Query&&) = default;
    Query& operator=(Query const&) = default;
    Query& operator=(Query&&) = default;

    /// Query for name (without arguments).
    explicit Query(std::string query) : query_{std::move(query)}, values_{} {}

    /// Query for name with arguments.
    Query(std::string query, std::vector<Value> data): query_{std::move(query)}, values_{std::move(data)} {}

    /// Query for name and arguments with fold-expression
    template<typename... T>
    explicit Query(std::string query, T... args) : query_{std::move(query)} {
        (..., values_.push_back(Value(args)));
    }

    /// Check if query is valid. \n
    /// The query is valid if the number of placeholders(?) is equal
    /// to the number of query arguments.
    [[nodiscard]] bool valid() const {
        auto placeholder_count = std::ranges::count_if(query_, [](char const c) {
            return '?' == c;
        });
        if (std::cmp_not_equal(placeholder_count, values_.size())) {
            fmt::print(stderr, "The number of placeholders and arguments does not match ({}, {})\n", placeholder_count, values_.size());
            return {};
        }
        return true;
    }

    [[nodiscard]] char const* c_str() const noexcept {
        return query_.c_str();
    }
    /// Return reference to query itself.
    [[nodiscard]] std::string const& query() const {
        return query_;
    }
    /// Return refernce to arguments.
    [[nodiscard]] std::vector<Value> const& values() const {
        return values_;
    }
};