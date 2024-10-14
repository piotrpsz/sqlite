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
    auto& operator()() const& {
        return data_;
    }
    [[nodiscard]] std::string name() const {
        return data_.first;
    }
    [[nodiscard]] Value value() const {
        return data_.second;
    }
    [[nodiscard]] std::string to_string() const {
        auto [name, value] = data_;
        return fmt::format("{}:[{}]", name, value.to_string());
    }

    /// Serialization. Converting a Field to bytes.
    [[nodiscard]] std::vector<u8> to_bytes() const {
        auto const& [name, value] = data_;
        auto const name_size = name.size();
        auto const value_bytes = value.to_bytes();
        auto const total_size = name_size + value_bytes.size();

        std::vector<u8> buffer(total_size + 1); // We take into account the marker 'F' byte (+ 1).
        u8* ptr = buffer.data();
        {   // marker 'F'
            constexpr u8 marker{'F'};
            memcpy(ptr, &marker, sizeof(u8));
            ptr += sizeof(u8);
        }
        {   // total size
            auto const v = static_cast<u32>(total_size);
            memcpy(ptr, &v, sizeof(u32));
            ptr += sizeof(u32);
        }
        {   // field name size
            auto const v = static_cast<u16>(name_size);
            memcpy(ptr, &v, sizeof(u16));
            ptr += sizeof(u16);
        }
        {   // field name bytes
            memcpy(ptr, name.data(), name_size);
            ptr += name_size;
        }
        {   // field value serialized bytes
            memcpy(ptr, value_bytes.data(), value_bytes.size());
        }
        return buffer;
    }

    /// Deserialization. Recreate Field from bytes.
    static std::pair<Field,size_t> from_bytes(std::span<u8> span) {
        size_t consumed_bytes{0};

        if (!span.empty() && span.front() == 'F') {
            span = span.subspan(1);
            consumed_bytes = 1;

            if (span.size() >= sizeof(u32)) {
                auto const total_size_span = span.subspan(0, sizeof(u32));
                size_t const total_size = *reinterpret_cast<u32*>(total_size_span.data());

                if (span.size() >= total_size) {
                    span = span.subspan(sizeof(u32));
                    consumed_bytes += sizeof(u32);

                    // get size (bytes count) of name
                    auto const name_size_span = span.subspan(0, sizeof(u16));
                    size_t const name_size = *reinterpret_cast<u16*>(name_size_span.data());
                    span = span.subspan(sizeof(u16));
                    consumed_bytes += sizeof(u16);

                    // get the name
                    auto name = std::string{reinterpret_cast<char const*>(span.data()), name_size};
                    span = span.subspan(name_size);
                    consumed_bytes += name_size;

                    // get bytes of value and create the Value
                    auto&& [value, nbytes] = Value::from_bytes(span.subspan(0, total_size - name_size));
                    consumed_bytes += nbytes;

                    return { Field(std::move(name), std::move(value)), consumed_bytes};
                }
            }
        }
        return {{}, 0};
    }
    bool operator==(Field const& rhs) const {
        return data_ == rhs.data_;
    }
};
