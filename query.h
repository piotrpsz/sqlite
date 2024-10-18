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
    static constexpr u8 QUERY_MARKER{'Q'};

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
    [[nodiscard]] std::vector<u8> to_bytes() const;
    // {
    //     // In the 'serialized_values' vector we will store the bytes from all serialized values.
    //     std::vector<std::vector<u8>> serialized_values{};
    //     serialized_values.reserve(values_.size());
    //
    //     // Total number of bytes of ALL serialized values.
    //     size_t values_size = 0;
    //
    //     // Serializing all values.
    //     std::ranges::for_each(values_, [&values_size, &serialized_values](auto const& v) {
    //         auto sv = v.to_bytes();
    //         values_size += sv.size();
    //         serialized_values.push_back(std::move(sv));
    //     });
    //
    //     u32 const total_size    // Total size does not include marker byte !!!
    //         = sizeof(u32)       // (4bytes) information about the query total size
    //         + sizeof(u16)       // (2bytes) information about the command size
    //         + sizeof(u16)       // (2bytes) information about number of values
    //         + sizeof(u32)       // (4bytes) information about bytes count of all serialized values.
    //         * cmd_.size()       // command content size in bytes
    //         + values_size;      // values total content size in bytes
    //
    //     std::vector<u8> buffer(total_size + 1); // We take into account the marker 'Q' byte (+ 1).
    //     u8* ptr = buffer.data();
    //     {   // marker 'Q'
    //         constexpr u8 marker{'Q'};
    //         memcpy(ptr, &marker, sizeof(u8));
    //         ptr += sizeof(u8);
    //     }
    //     {   // information about the query total size (without Q)
    //         auto const nbytes = static_cast<u32>(total_size);
    //         memcpy(ptr, &nbytes, sizeof(u32));
    //         ptr += sizeof(u32);
    //     }
    //     {   // information about the command size
    //         auto const nbytes = static_cast<u16>(cmd_.size());
    //         memcpy(ptr, &nbytes, sizeof(u16));
    //         ptr += sizeof(u16);
    //     }
    //     {   // information about number of values.
    //         auto const nbytes = static_cast<u16>(values_.size());
    //         memcpy(ptr, &nbytes, sizeof(u16));
    //         ptr += sizeof(u16);
    //     }
    //     {   // information about bytes count of all serialized values.
    //         auto const nbytes = static_cast<u32>(values_size);
    //         memcpy(ptr, &nbytes, sizeof(u32));
    //         ptr += sizeof(u32);
    //     }
    //     {   // command content
    //         memcpy(ptr, cmd_.data(), cmd_.size());
    //         ptr += cmd_.size();
    //     }
    //     {   // serialized values
    //         for (auto&& v: serialized_values) {
    //             memcpy(ptr, v.data(), v.size());
    //             ptr += v.size();
    //         }
    //     }
    //     return buffer;
    // }

    /// Deserialization. Recreate Query from bytes.
    static std::pair<Query,size_t> from_bytes(std::span<u8> span);
    // {
    //     size_t consumed_bytes{0};
    //
    //     if (!span.empty() && span.front() == 'Q') {
    //         span = span.subspan(1);
    //         consumed_bytes += 1;
    //
    //         // next step - get total span size
    //         if (span.size() >= sizeof(u32)) {
    //             auto const total_size_span = span.subspan(0, sizeof(u32));
    //             size_t const  total_size = *reinterpret_cast<u32*>(total_size_span.data());
    //
    //             if (span.size() >= total_size) {
    //                 span = span.subspan(sizeof(u32));
    //                 consumed_bytes += sizeof(u32);
    //                 // --------------------------------------------------------
    //                 auto const cmd_size_span = span.subspan(0, sizeof(u16));
    //                 auto const cmd_size = *reinterpret_cast<u16*>(cmd_size_span.data());
    //                 span = span.subspan(sizeof(u16));
    //                 consumed_bytes += sizeof(u16);
    //                 // --------------------------------------------------------
    //                 auto const number_value_span = span.subspan(0, sizeof(u16));
    //                 auto const number_value = *reinterpret_cast<u16*>(number_value_span.data());
    //                 span = span.subspan(sizeof(u16));
    //                 consumed_bytes += sizeof(u16);
    //                 // --------------------------------------------------------
    //                 auto const serialized_values_span = span.subspan(0, sizeof(u32));
    //                 auto const serialized_value = *reinterpret_cast<u32*>(serialized_values_span.data());
    //                 span = span.subspan(sizeof(u32));
    //                 consumed_bytes += sizeof(u32);
    //                 // --------------------------------------------------------
    //                 auto cmd = std::string{reinterpret_cast<char const*>(span.data()), cmd_size};
    //                 Query query{std::move(cmd)};
    //                 span = span.subspan(cmd_size);
    //                 consumed_bytes += cmd_size;
    //                 // --------------------------------------------------------
    //                 for (int i = 0; i < number_value; ++i) {
    //                     auto [v, nbytes] = Value::from_bytes(span);
    //                     query.add_arg(std::move(v));
    //                     span = span.subspan(nbytes);
    //                     consumed_bytes += nbytes;
    //                 }
    //                 return {query, consumed_bytes};
    //             }
    //         }
    //     }
    //     return {{}, consumed_bytes};
    // }

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
