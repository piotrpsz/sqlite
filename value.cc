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
// Created by Piotr Pszczółkwski on 24.10.2024 (piotr@beesoft.pl).
//

/*------- include files:
-------------------------------------------------------------------*/
#include "value.h"

/********************************************************************
*                                                                   *
*                         T O   B Y T E S                           *
*                                                                   *
********************************************************************/

auto Value::
to_bytes() const noexcept
-> std::vector<char> {
    auto const data = value_to_bytes();
    u32 const chunk_size = static_cast<u32>(data.size());

    std::vector<char> buffer{};
    buffer.reserve(sizeof(char) + sizeof(u32) + chunk_size);

    buffer.push_back(marker());
    std::copy_n(reinterpret_cast<u8 const*>(&chunk_size), sizeof(u32), std::back_inserter(buffer));
    std::copy_n(data.data(), chunk_size, std::back_inserter(buffer));
    buffer.shrink_to_fit();

    return buffer;
}

/********************************************************************
*                                                                   *
*                       F R O M   B Y T E S                         *
*                                                                   *
********************************************************************/

auto Value::
from_bytes(std::span<const char> span) noexcept
-> std::pair<Value,size_t> {
    if (!span.empty() && is_marker(span.front())) {
        size_t consumed_bytes = 0;
        auto const type = span.front();
        span = span.subspan(1);
        consumed_bytes += 1;

        // next step - get the number of bytes that make up the value
        if (const auto chunk_size = shared::from<u32>(span)) {
            span = span.subspan(sizeof(u32));
            consumed_bytes += sizeof(u32);

            if (span.size() >= *chunk_size) {
                span = span.first(*chunk_size);
                consumed_bytes += *chunk_size;

                switch (type) {
                    case 'I': {
                        auto const v = *reinterpret_cast<i64 const*>(span.data());
                        return {Value{v}, consumed_bytes};
                    }
                    case 'D': {
                        auto const v = *reinterpret_cast<f64 const*>(span.data());
                        return { Value{v}, consumed_bytes};
                    }
                    case 'S': {
                        auto const v = std::string{reinterpret_cast<char const*>(span.data()),span.size()};
                        return { Value(v), consumed_bytes};
                    }
                    case 'V': {
                        auto const v = std::vector<u8>{span.begin(), span.end()};
                        return { Value(v), consumed_bytes};
                    }
                    default:
                    {}
                }
            }
        }
    }
    return {{}, 0};
}

/********************************************************************
*                                                                   *
*                S E R I A L I Z E D   D A T A                      *
*                                                                   *
********************************************************************/

std::string Value::serialized_data(std::span<char> span) noexcept {
    std::string buffer{};
    if (!span.empty() && is_marker(span[0])) {
        auto type = span[0];
        buffer.append(fmt::format("0x{:02x}  [{}]\n", type, static_cast<char>(type)));
        span = span.subspan(1);

        if (auto chunk_size = shared::from<u32>(span)) {
            buffer.append(fmt::format("{}  [{}]\n", shared::hex_bytes_as_str(span.first(sizeof(u32))), *chunk_size));
            span = span.subspan(sizeof(u32));

            if (span.size() >= *chunk_size) {
                span = span.first(*chunk_size);
                switch (type) {
                    case 'I': {
                        auto const v = *reinterpret_cast<i64*>(span.data());
                        buffer.append(fmt::format("{}  [{}]\n", shared::hex_bytes_as_str(span), v));
                        break;
                    }
                    case 'D': {
                        auto const v = *reinterpret_cast<f64*>(span.data());
                        buffer.append(fmt::format("{}  [{}]\n", shared::hex_bytes_as_str(span), v));
                        break;
                    }
                    case 'S': {
                        auto const v = std::string{reinterpret_cast<char const*>(span.data()),span.size()};
                        buffer.append(fmt::format("{}  [{}]\n", shared::hex_bytes_as_str(span), v));
                        break;

                    }
                    case 'V': {
                        auto const v = std::vector<u8>{span.begin(), span.end()};
                        buffer.append(fmt::format("{0}  [{0}]\n", shared::hex_bytes_as_str(span)));
                        break;
                    }
                    default:
                    {}
                }
            }

        }
    }
    return buffer;
}

/********************************************************************
*                                                                   *
*                        T O   S T R I N G                          *
*                                                                   *
********************************************************************/

auto Value::
to_string() const noexcept
-> std::string {
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

/********************************************************************
*                                                                   *
*                        I S   M A R K E R                          *
*                                                                   *
********************************************************************/

auto Value::
is_marker(char const c) noexcept
-> bool {
    if (c == 'M') return true;
    if (c == 'I') return true;
    if (c == 'D') return true;
    if (c == 'S') return true;
    if (c == 'V') return true;
    return {};
}

/********************************************************************
*                                                                   *
*                           M A R K E R                             *
*                                                                   *
********************************************************************/

auto Value::
marker() const noexcept
-> char {
    switch (data_.index()) {
        case MONOSTATE: return 'M';
        case INTEGER:   return 'I';
        case DOUBLE:    return 'D';
        case STRING:    return 'S';
        case VECTOR:    return 'V';
        default:        return '?';
    }
}

/********************************************************************
*                                                                   *
*                  V A L U E   T O   B Y T E S                      *
*                                                                   *
********************************************************************/

auto Value::
value_to_bytes() const noexcept
-> std::vector<char> {
    switch (data_.index()) {
        case INTEGER: {
            // i64 = 64 bity = 8 bajtów
            std::vector<char> buffer(sizeof(i64));
            auto const v = value<i64>();
            memcpy(buffer.data(), &v, sizeof(i64));
            return std::move(buffer);
        }
        case DOUBLE: {
            // f64 = 64 bity = 8 bajtów.
            std::vector<char> buffer(sizeof(f64));
            auto const v = value<f64>();
            memcpy(buffer.data(), &v, sizeof(f64));
            return std::move(buffer);
        }
        case STRING: {
            auto const v = value<std::string>();
            std::vector<char> buffer(v.size());
            memcpy(buffer.data(), v.data(), v.size());
            return std::move(buffer);
        }
        case VECTOR: {
            auto const v = value<std::vector<u8>>();
            std::vector<char> buffer(v.size());
            memcpy(buffer.data(), v.data(), v.size());
            return std::move(buffer);
        }
        default:
            return {};
    }
}
