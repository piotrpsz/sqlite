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
to_bytes(bool const compress) const
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

    // Chunk size does not include the marker and itself.
    // Total size describes everything that is behind it.
    u32 const chunk_size
        = sizeof(u16)       // information about the command size
        + sizeof(u16)       // information about number of values
        + cmd_size          // command content bytes
        + values_size;      // all values content bytes

    std::vector<char> buffer{};
    buffer.reserve(sizeof(u32) + chunk_size);

    std::copy_n(reinterpret_cast<char const*>(&chunk_size), sizeof(u32), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<char const*>(&cmd_size), sizeof(u16), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<char const*>(&values_count), sizeof(u16), std::back_inserter(buffer));
    std::copy_n(cmd_.begin(), cmd_size, std::back_inserter(buffer));
    std::ranges::for_each(serialized_values, [&buffer](auto sf) {
        std::copy_n(sf.begin(), sf.size(), std::back_inserter(buffer));
    });

    if (compress) {
        auto const compressed = gzip::compress(buffer);
        std::vector<char> result(1 + compressed.size());
        result[0] = QUERY_MARKER;
        std::memcpy(result.data() + 1, compressed.data(), compressed.size());
        return std::move(result);
    }

    std::vector<char> result(1 + buffer.size());
    result[0] = QUERY_MARKER;
    std::memcpy(result.data() + 1, buffer.data(), buffer.size());
    return std::move(result);
}

/********************************************************************
*                                                                   *
*                       F R O M   B Y T E S                         *
*                                                                   *
********************************************************************/

auto Query::
from_bytes(std::span<char> span, bool const compress)
-> std::pair<Query,size_t> {
    if (!span.empty() && span.front() == QUERY_MARKER) {
        span = span.subspan(1);
        if (auto const chunk_size = shared::from<u32>(span)) {
            size_t consumed_bytes = 1;
            span = span.subspan(sizeof(u32));
            consumed_bytes += sizeof(u32);
            if (span.size() >= *chunk_size) {
                span = span.first(*chunk_size);
                if (auto const cmd_size = shared::from<u16>(span)) {
                    span = span.subspan(sizeof(u16));
                    consumed_bytes += sizeof(u16);
                    if (auto const values_count = shared::from<u16>(span)) {
                        span = span.subspan(sizeof(u16));
                        consumed_bytes += sizeof(u16);
                        auto const cmd = std::string{reinterpret_cast<char const*>(span.data()), *cmd_size};
                        span = span.subspan(*cmd_size);
                        consumed_bytes += *cmd_size;
                        Query query{cmd};

                        for (int i = 0; i < values_count; ++i) {
                            auto [v, nbytes] = Value::from_bytes(span);
                            query.add_arg(std::move(v));
                            span = span.subspan(nbytes);
                            consumed_bytes += nbytes;
                        }
                        return {query, consumed_bytes};
                    }
                }
            }
        }
    }

    return {{}, 0};
}
