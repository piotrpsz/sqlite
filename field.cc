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
#include "field.h"

/********************************************************************
*                                                                   *
*                        T O   S T R I N G                          *
*                                                                   *
********************************************************************/

auto Field::
to_string() const -> std::string {
    auto const [name, value] = data_;
    return fmt::format("{}:[{}]", name, value.to_string());
}

/********************************************************************
*                                                                   *
*                         T O   B Y T E S                           *
*                                                                   *
********************************************************************/

auto Field::
to_bytes() const
-> std::vector<char> {
    auto const& [name, value] = data_;
    auto const value_bytes = value.to_bytes();

    // Chunk size does not include the marker and itself.
    // Total size describes everything that is behind it.
    u16 const name_size = name.size();
    u32 const chunk_size
        = sizeof(u16)           // name size
        + name.size()           // name bytes
        + value_bytes.size();   // value bytes

    std::vector<char> buffer{};
    buffer.reserve(sizeof(char) + sizeof(u32) + chunk_size);

    buffer.push_back('F');
    std::copy_n(reinterpret_cast<u8 const*>(&chunk_size), sizeof(u32), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<u8 const*>(&name_size), sizeof(u16), std::back_inserter(buffer));
    std::copy_n(name.begin(), name_size, std::back_inserter(buffer));
    std::copy_n(value_bytes.begin(), value_bytes.size(), std::back_inserter(buffer));
    buffer.shrink_to_fit();

    return buffer;
}

/********************************************************************
*                                                                   *
*                       F R O M   B Y T E S                         *
*                                                                   *
********************************************************************/

auto Field::
from_bytes(std::span<const char> span)
-> std::pair<Field,size_t> {
    size_t consumed_bytes{0};

    if (!span.empty() && span.front() == 'F') {
        span = span.subspan(1);
        consumed_bytes = 1;

        if (const auto chunk_size = shared::from<u32>(span)) {
            span = span.subspan(sizeof(u32));
            consumed_bytes += sizeof(u32);

            if (span.size() >= *chunk_size) {
                span = span.first(*chunk_size);

                if (const auto name_size = shared::from<u16>(span)) {
                    span = span.subspan(sizeof(u16));
                    consumed_bytes += sizeof(u16);

                    // get the name bytes
                    auto name = std::string{reinterpret_cast<char const*>(span.data()), *name_size};
                    span = span.subspan(*name_size);
                    consumed_bytes += *name_size;

                    // get bytes of value and create the Value
                    if (auto const value_size = *chunk_size - sizeof(u16) - *name_size; span.size() >= value_size) {
                        auto&& [value, nbytes] = Value::from_bytes(span);
                        consumed_bytes += nbytes;

                        return { Field(std::move(name), std::move(value)), consumed_bytes};
                    }
                }
            }
        }
    }
    return {{}, 0};
}

auto Field::
serialized_data(std::span<char> span) -> std::string {
    if (!span.empty() && span.front() == 'F') {
        std::string buffer{};
        buffer.append(fmt::format("0x{:02x} [{}]\n", span[0], static_cast<char>(span[0])));
        span = span.subspan(1);

        if (auto chunk_size = shared::from<u32>(span)) {
            auto const subspan = span.subspan(0, sizeof(u32));
            buffer.append(fmt::format("{} [{}]\n", shared::hex_bytes_as_str(subspan), *chunk_size));
            span = span.subspan(sizeof(u32));

            if (auto name_size = shared::from<u16>(span)) {
                buffer.append(fmt::format("{} [{}]\n", shared::hex_bytes_as_str(span.subspan(0, *name_size)), *name_size));
                span = span.subspan(sizeof(u16));
                if (span.size() >= name_size) {
                    auto const span_name = span.subspan(0, *name_size);
                    auto name = std::string{reinterpret_cast<char const*>(span_name.data()), *name_size};
                    buffer.append(fmt::format("{} [{}]\n", shared::hex_bytes_as_str(span_name), name));
                    span = span.subspan(*name_size);
                    if (span.size() >= (*chunk_size - *name_size - sizeof(u16))) {
                        auto const span_value = span.subspan(0, *chunk_size - *name_size - sizeof(u16));
                        auto&& [value, nbytes] = Value::from_bytes(span_value);
                        buffer.append(fmt::format("{} [{}]\n", shared::hex_bytes_as_str(span_value), value.to_string()));
                        return buffer;
                    }
                }
            }
        }
    }
    return "?";
}
