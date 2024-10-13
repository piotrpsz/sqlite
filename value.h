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

    /// Check if the object contains a specific value.
    [[nodiscard]] bool is_null() const noexcept {
        return data_.index() == MONOSTATE;
    }

    /// Take the index of the contained value.
    /// i.e. MONOSTATE, INTEGER, DOUBLE, STRING, VECTOR
    [[nodiscard]] uint index() const noexcept {
        return data_.index();
    }

    /// Each field will start with a letter ('marker') specifying the type of the value. \n
    /// 'M' - monostate, 'I' - integer, 'D' - double, 'S' - string, 'V' - vector. \n
    /// Then a number u32 (4 bytes) specifying the size of the value in bytes. \n
    /// The byte representation of the value to the end. \n
    /// |mark|u32 := how many bytes|value bytes|
    /// \return Resulting byte vector.
    [[nodiscard]] std::vector<u8> serialize() const noexcept {
        auto const data = value_to_bytes();
        // Buffer size := 1 byte (marker) + value bytes.
        std::vector<u8> buffer(1 + sizeof(u32) + data.size());
        u32 const nbytes = value_to_bytes().size();

        buffer[0] = marker();
        memcpy(&buffer[1], &nbytes, sizeof(u32));
        memcpy(&buffer[1 + sizeof(u32)], data.data(), data.size());
        return buffer;
    }

    /// Creating a 'Value' object based on the sent bytes.
    /// \param span source bytes,
    /// \return value as object and bytes count consumed on deserialization.
    /// \remark zero returned as consumed bytes count means error.
    /// \remark size of the span we are checking step by step.
    static std::pair<Value,size_t> deserialize(std::span<u8> span) noexcept {
        size_t consumed_bytes = 0;

        if (!span.empty()) {
            auto const value_type = span[0];
            span = span.subspan(1);
            consumed_bytes += 1;

            // next step - get the number of bytes that make up the value
            if (span.size() >= sizeof(u32)) {
                auto const nbytes = *reinterpret_cast<u32*>(span.subspan(0, sizeof(u32)).data());
                span = span.subspan(sizeof(u32));
                consumed_bytes += sizeof(u32);

                // next step - take the bytes that make up the value and convert them to the correct value
                if (span.size() >= nbytes) {
                    std::vector<u8> buffer(nbytes);
                    memcpy(buffer.data(), span.data(), nbytes);
                    consumed_bytes += nbytes;

                    switch (value_type) {
                        case 'I': {
                            auto const v = *reinterpret_cast<i64*>(buffer.data());
                            return {Value{v}, consumed_bytes};
                        }
                        case 'D': {
                            auto const v = *reinterpret_cast<f64*>(buffer.data());
                            return { Value{v}, consumed_bytes};
                        }
                        case 'S': {
                            auto const value = std::string{reinterpret_cast<char const*>(buffer.data()),buffer.size()};
                            return { Value(value), consumed_bytes};
                        }
                        case 'V':
                            return { Value(buffer), consumed_bytes};
                        default:
                        {}
                    }
                }
            }
        }
        return {{}, consumed_bytes};
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

    // Getting optional values.
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
private:
    [[nodiscard]] u8 marker() const noexcept {
        switch (data_.index()) {
            case MONOSTATE: return 'M';
            case INTEGER:   return 'I';
            case DOUBLE:    return 'D';
            case STRING:    return 'S';
            case VECTOR:    return 'V';
            default:        return '?';
        }
    }

    /// Return size value in bytes.
    [[nodiscard]] std::vector<u8> value_to_bytes() const noexcept {
        switch (data_.index()) {
            case INTEGER: {
                // i64 = 64 bity = 8 bajtów
                std::vector<u8> buffer(8);
                auto const v = value<i64>();
                memcpy(buffer.data(), &v, 8);
                return std::move(buffer);
            }
            case DOUBLE: {
                // f64 = 64 bity = 8 bajtów.
                std::vector<u8> buffer(8);
                auto const v = value<f64>();
                memcpy(buffer.data(), &v, 8);
                return std::move(buffer);
            }
            case STRING: {
                auto const v = value<std::string>();
                std::vector<u8> buffer(v.size());
                memcpy(buffer.data(), v.data(), v.size());
                return std::move(buffer);
            }
            case VECTOR: {
                auto const v = value<std::vector<u8>>();
                std::vector<u8> buffer(v.size());
                memcpy(buffer.data(), v.data(), v.size());
                return std::move(buffer);
            }
            default:
                return {};
        }
    }
};
