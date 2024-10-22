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
// Created by Piotr Pszczółkwski on 18.10.2024 (piotr@beesoft.pl).
//

/*------- include files:
-------------------------------------------------------------------*/
#include "query.h"
#include "gzip.h"

/********************************************************************
*                                                                   *
*                         T O   B Y T E S                           *
*                                                                   *
********************************************************************/

auto Query::
to_bytes() const
-> std::vector<char> {
    // In the 'serialized_values' vector we will store the bytes from ALL serialized values.
    std::vector<std::vector<char>> serialized_values{};
    serialized_values.reserve(values_.size());
    // Total number of bytes of ALL serialized values.
    auto values_size = 0;

    // All values to bytes
    std::ranges::for_each(values_, [&values_size, &serialized_values](auto const& v) {
        auto sv = v.to_bytes();
        values_size += sv.size();
        serialized_values.push_back(std::move(sv));
    });

    u16 const cmd_size = cmd_.size();
    u16 const values_count = values_.size();

    // Chunk size describes everything that is behind it.
    u32 const chunk_size
        = sizeof(u16)       // information about the command size
        + sizeof(u16)       // information about number of values
        + cmd_size          // command content bytes
        + values_size;      // all values content bytes

    std::vector<char> buffer{};
    buffer.reserve(sizeof(char) + sizeof(u32) + chunk_size);

    buffer.push_back(QUERY_MARKER);
    std::copy_n(reinterpret_cast<char const*>(&chunk_size), sizeof(u32), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<char const*>(&cmd_size), sizeof(u16), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<char const*>(&values_count), sizeof(u16), std::back_inserter(buffer));
    std::copy_n(cmd_.begin(), cmd_size, std::back_inserter(buffer));
    std::ranges::for_each(serialized_values, [&buffer](auto sf) {
        std::copy_n(sf.begin(), sf.size(), std::back_inserter(buffer));
    });

    return std::move(buffer);
}

auto Query::
to_gzip_bytes() const
-> std::vector<char> {
    // In the 'serialized_values' vector we will store the bytes from ALL serialized values.
    std::vector<std::vector<char>> serialized_values{};
    serialized_values.reserve(values_.size());
    // Total number of bytes of ALL serialized values.
    auto values_size = 0;

    // All values to bytes
    std::ranges::for_each(values_, [&values_size, &serialized_values](auto const& v) {
        auto sv = v.to_bytes();
        values_size += sv.size();
        serialized_values.push_back(std::move(sv));
    });

    u16 const cmd_size = cmd_.size();
    u16 const values_count = values_.size();
    std::vector<char> buffer{};
    buffer.reserve(sizeof(u16) + sizeof(u16) + cmd_size + values_size);

    std::copy_n(reinterpret_cast<char const*>(&cmd_size), sizeof(u16), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<char const*>(&values_count), sizeof(u16), std::back_inserter(buffer));
    std::copy_n(cmd_.begin(), cmd_size, std::back_inserter(buffer));
    std::ranges::for_each(serialized_values, [&buffer](auto sf) {
        std::copy_n(sf.begin(), sf.size(), std::back_inserter(buffer));
    });

    // The compressed serialization result ultimately consists of three components:
    // 1. marker 'Q',
    // 2. size of compressed data (u32)
    // 3. compressed data
    auto const compressed = gzip::compress(buffer);
    u32 const nbytes = compressed.size();
    std::vector<char> result{};
    result.reserve(sizeof(u8) + sizeof(u32) + nbytes);
    result.push_back(static_cast<char>(QUERY_MARKER | 0b1000'0000));
    std::copy_n(reinterpret_cast<char const*>(&nbytes), sizeof(u32), std::back_inserter(result));
    std::copy_n(compressed.data(), compressed.size(), std::back_inserter(result));
    return std::move(result);
}

/********************************************************************
*                                                                   *
*                       F R O M   B Y T E S                         *
*                                                                   *
********************************************************************/

auto Query::
from_bytes(std::span<char> span)
-> std::pair<Query,size_t> {
    if (span.empty())
        return {};

    // It is possible that the data is packed
    if (auto const marker = span.front(); (marker & 0b1000'0000) == 0b1000'0000)
        return from_gzip_bytes(span);

    if (span.front() == QUERY_MARKER) {
        span = span.subspan(1);
        if (auto const nbytes = shared::from<u32>(span)) {
            span = span.subspan(sizeof(u32));
            if (span.size() >= *nbytes) {
                span = span.first(*nbytes);
                if (auto const cmd_size = shared::from<u16>(span)) {
                    span = span.subspan(sizeof(u16));
                    if (auto const values_count = shared::from<u16>(span)) {
                        span = span.subspan(sizeof(u16));
                        auto const cmd = std::string{reinterpret_cast<char const*>(span.data()), *cmd_size};
                        span = span.subspan(*cmd_size);
                        Query query{cmd};
                        for (int i = 0; i < values_count; ++i) {
                            auto [v, nbytes] = Value::from_bytes(span);
                            query.add_arg(std::move(v));
                            span = span.subspan(nbytes);
                        }
                        return {query, sizeof(char) + sizeof(u32) + *nbytes};
                    }
                }
            }
        }
    }
    return {{}, 0};
}

auto Query::
from_gzip_bytes(std::span<const char> span)
-> std::pair<Query,size_t> {
    if (span.empty())
        return {};

    if (auto const marker = span.front(); (marker & 0b1000'0000) == 0b1000'0000) {
        if (static_cast<char>(marker & ~0b1000'0000) == QUERY_MARKER) {
            span = span.subspan(1);
            if (auto const nbytes = shared::from<u32>(span)) {
                span = span.subspan(sizeof(u32));
                if (span.size() >= *nbytes) {
                    // After the marker and size, there are already compressed data,
                    // the number of bytes of which is equal to the designated size.
                    // And only they are of interest to us.
                    span = span.first(*nbytes);
                    auto unpacked_data = gzip::decompress(span);
                    span = std::span(unpacked_data.data(), unpacked_data.size());
                    // From now on we are working on unpacked data

                    // Command text size.
                    if (auto const cmd_size = shared::from<u16>(span)) {
                        span = span.subspan(sizeof(u16));
                        // Number of values.
                        if (auto const values_count = shared::from<u16>(span)) {
                            span = span.subspan(sizeof(u16));
                            // Command.
                            auto const cmd = std::string{reinterpret_cast<char const*>(span.data()), *cmd_size};
                            span = span.subspan(*cmd_size);
                            Query query{cmd};
                            // Values.
                            for (int i = 0; i < values_count; ++i) {
                                auto [v, nbytes] = Value::from_bytes(span);
                                query.add_arg(std::move(v));
                                span = span.subspan(nbytes);
                            }
                            return {std::move(query),  sizeof(char) + sizeof(u32) + *nbytes};
                        }
                    }
                }
            }
        }
    }
    return {{}, 0};
}
