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
// Created by Piotr Pszczółkwski on 17.10.2024 (piotr@beesoft.pl).
//

/*------- include files:
-------------------------------------------------------------------*/
#include "result.h"
#include "gzip.h"

/********************************************************************
*                                                                   *
*                        T O   S T R I N G                          *
*                                                                   *
********************************************************************/

auto Result
::to_string() const
-> std::string {
    std::vector<std::string> rows{};
    rows.reserve(data_.size());
    std::ranges::for_each(data_, [&rows](auto row) {
        rows.push_back(row.to_string());
    });
    return shared::join(rows, '\n');
}


/********************************************************************
*                                                                   *
*                         T O   B Y T E S                           *
*                                                                   *
********************************************************************/

auto Result::
to_bytes() const
-> std::vector<char> {
    std::vector<std::vector<char>> serialized_rows{};
    const u16 rows_count = data_.size();
    serialized_rows.reserve(rows_count);
    size_t rows_size = 0;

    for (const auto& row : data_) {
        auto sr = row.to_bytes();
        rows_size += sr.size();
        serialized_rows.push_back(std::move(sr));
    }

    // Chunk size describes everything that is behind it.
    u32 const chunk_size
        = sizeof(u16)   // rows number
        + rows_size;    // bytes number of sll rows

    std::vector<char> buffer{};
    buffer.reserve(sizeof(char) + sizeof(u32) + chunk_size);

    buffer.push_back(RESULT_MARKER);
    std::copy_n(reinterpret_cast<char const*>(&chunk_size), sizeof(u32), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<char const*>(&rows_count), sizeof(u16), std::back_inserter(buffer));
    std::ranges::for_each(serialized_rows, [&buffer](auto sf) {
        std::copy_n(sf.begin(), sf.size(), std::back_inserter(buffer));
    });

    return std::move(buffer);
}

/********************************************************************
*                                                                   *
*                       F R O M   B Y T E S                         *
*                                                                   *
********************************************************************/

auto Result::
from_bytes(std::span<const char> span) ->
std::pair<Result,size_t> {
    if (span.empty())
        return {};

    if (auto const marker = span.front(); (marker & 0b1000'0000) == 0b1000'0000)
        return from_gzip_bytes(span);

    if (!span.empty() && span.front() == RESULT_MARKER) {
        span = span.subspan(1);
        if (auto const nbytes = shared::from<u32>(span)) {
            span = span.subspan(sizeof(u32));
            if (span.size() >= *nbytes) {
                span = span.first(*nbytes);
                if (auto const rows_count = shared::from<u16>(span)) {
                    span = span.subspan(sizeof(u16));
                    Result result{};
                    for (int i = 0; i < rows_count; ++i) {
                        auto [row, nbytes] = Row::from_bytes(span);
                        result.add(std::move(row));
                        span = span.subspan(nbytes);
                    }
                    return {std::move(result), sizeof(char) + sizeof(u32) + *nbytes};
                }
            }
        }
    }

    return {};
}


auto Result::
to_gzip_bytes() const
-> std::vector<char> {
    std::vector<std::vector<char>> serialized_rows{};
    const u16 rows_count = data_.size();
    serialized_rows.reserve(rows_count);
    size_t rows_size = 0;

    for (const auto& row : data_) {
        auto sr = row.to_bytes();
        rows_size += sr.size();
        serialized_rows.push_back(std::move(sr));
    }

    std::vector<char> buffer{};
    buffer.reserve(sizeof(u16) + rows_size);
    std::copy_n(reinterpret_cast<char const*>(&rows_count), sizeof(u16), std::back_inserter(buffer));
    std::ranges::for_each(serialized_rows, [&buffer](auto sf) {
        std::copy_n(sf.begin(), sf.size(), std::back_inserter(buffer));
    });

    auto const compressed = gzip::compress(buffer);
    u32 const nbytes = compressed.size();
    std::vector<char> result{};
    result.reserve(sizeof(char) + sizeof(u32) + compressed.size());
    result.push_back(static_cast<char>(RESULT_MARKER | 0b1000'0000));
    std::copy_n(reinterpret_cast<char const*>(&nbytes), sizeof(u32), std::back_inserter(result));
    std::copy_n(compressed.begin(), compressed.size(), std::back_inserter(result));
    return std::move(result);
}

auto Result::
from_gzip_bytes(std::span<const char> span) ->
std::pair<Result,size_t> {
    if (span.empty())
        return {};

    if (auto const marker = span.front(); (marker & 0b1000'0000) == 0b1000'0000) {
        if (static_cast<char>(marker & ~0b1000'0000) == RESULT_MARKER) {
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

                    // Number of rows.
                    if (auto const rows_count = shared::from<u16>(span)) {
                        span = span.subspan(sizeof(u16));
                        Result result{};
                        for (int i = 0; i < rows_count; ++i) {
                            auto [row, nbytes] = Row::from_bytes(span);
                            result.add(std::move(row));
                            span = span.subspan(nbytes);
                        }
                        return {std::move(result), sizeof(char) + sizeof(u32) + *nbytes};
                    }
                }
            }
        }
    }
    return {};
}

/********************************************************************
*                                                                   *
*                      O P E R A T O R   ==                         *
*                                                                   *
********************************************************************/

auto Result::
operator==(Result const& rhs) const noexcept
-> bool {
    for (auto i = 0; i < data_.size(); ++i)
        if (data_[i] != rhs.data_[i])
            return false;
    return true;
}
