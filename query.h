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

    // 1. 'Q'
    // 2. u32 := total size of the query
    // 3. u16 := command size
    // 4. u16 := number of value
    // 5. u32 := total size of values | values bytes.... |
    [[nodiscard]] std::vector<u8> serialize() const {
        std::vector<std::vector<u8>> serialized_values{};   // Vector with serialized value bytes.
        serialized_values.reserve(values_.size());
        size_t values_size = 0;                             // Total size of all serialized values.
        std::ranges::for_each(values_, [&](auto const& v) {
            auto sv = v.serialize();
            values_size += sv.size();
            serialized_values.push_back(std::move(sv));
        });

        size_t cmd_size = cmd_.size();
        size_t values_count = values_.size();
        const u32 total_size
            = sizeof(u8)    // (1b) marker 'Q'
            + sizeof(u32)   // (4b) informaation about the query total size
            + sizeof(u16)   // (2b) information about the command size
            + sizeof(u16)   // (2b) information about number of values
            + sizeof(u32)   // (4b) information about bytes count of all serialized values.
            * cmd_.size()   // command content
            + values_size;  // values content

        fmt::print("-------------------------------------------------------------------\n");

        std::vector<u8> buffer(total_size);
        u8* ptr = buffer.data();
        {   // marker 'Q'
            constexpr u8 marker{'Q'};
            memcpy(ptr, &marker, sizeof(marker));
            ptr += sizeof(u8);
        }
        {   // information about the query total size (without Q)
            auto const nbytes = static_cast<u32>(total_size);
            fmt::print("total size: {}\n", nbytes);
            memcpy(ptr, &nbytes, sizeof(u32));
            ptr += sizeof(u32);
        }
        {   // information about the command size
            auto const nbytes = static_cast<u16>(cmd_.size());
            fmt::print("cmd size: {}\n", nbytes);
            memcpy(ptr, &nbytes, sizeof(u16));
            ptr += sizeof(u16);
        }
        {   // information about number of values.
            auto const nbytes = static_cast<u16>(values_.size());
            fmt::print("number of values: {}\n", nbytes);
            memcpy(ptr, &nbytes, sizeof(u16));
            ptr += sizeof(u16);
        }
        {   // information about bytes count of all serialized values.
            auto const nbytes = static_cast<u32>(values_size);
            fmt::print("serialized values size: {}\n", nbytes);
            memcpy(ptr, &nbytes, sizeof(u32));
            ptr += sizeof(u32);
        }
        {   // command content
            memcpy(ptr, cmd_.data(), cmd_.size());
            ptr += cmd_.size();
        }
        {   // serialized values
            for (auto&& v: serialized_values) {
                memcpy(ptr, v.data(), v.size());
                ptr += v.size();
            }
        }
        return buffer;
    }

    static Query deserialize(std::span<u8> span) {
        fmt::print("=========================================================================\n");
        if (span[0] == 'Q') {
            span = span.subspan(1);
            auto total_size_span = span.subspan(0, sizeof(u32));
            u32 total_size = *reinterpret_cast<u32*>(total_size_span.data());
            fmt::print("total size: {}\n", total_size);
            span = span.subspan(sizeof(u32));
            // --------------------------------------------------------
            auto cmd_size_span = span.subspan(0, sizeof(u16));
            u16 cmd_size = *reinterpret_cast<u16*>(cmd_size_span.data());
            fmt::print("cmd size: {}\n", cmd_size);
            span = span.subspan(sizeof(u16));
            // --------------------------------------------------------
            auto number_value_span = span.subspan(0, sizeof(u16));
            u16 number_value = *reinterpret_cast<u16*>(number_value_span.data());
            fmt::print("number_value: {}\n", number_value);
            // --------------------------------------------------------
            span = span.subspan(sizeof(u16));
            auto serialized_values_span = span.subspan(0, sizeof(u32));
            u32 serialized_value = *reinterpret_cast<u32*>(serialized_values_span.data());
            fmt::print("serialized_value: {}\n", serialized_value);
            span = span.subspan(sizeof(u32));
            // --------------------------------------------------------
            auto cmd = std::string{reinterpret_cast<char const*>(span.data()), cmd_size};
            fmt::print("cmd: {}\n", cmd);
            span = span.subspan(cmd_size);
            // --------------------------------------------------------
            for (int i = 0; i < number_value; ++i) {
                auto [v, nbytes] = Value::deserialize(span);
                fmt::print("value: {}\n", v.to_string());
                span = span.subspan(nbytes);
            }
        }
        return Query{};
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
