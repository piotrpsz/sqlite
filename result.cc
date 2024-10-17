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
-> std::vector<u8> {
    std::vector<std::vector<u8>> serialized_rows{};
    const u16 rows_count = data_.size();
    serialized_rows.reserve(rows_count);
    size_t rows_size = 0;

    for (const auto& row : data_) {
        auto sr = row.to_bytes();
        rows_size += sr.size();
        serialized_rows.push_back(std::move(sr));
    }

    u32 const chunk_size
        = sizeof(u16)   // rows number
        + rows_size;    // bytes number of sll rows

    std::vector<u8> buffer{};
    buffer.reserve(1 + sizeof(u32) + chunk_size);

    buffer.push_back('T');
    std::copy_n(reinterpret_cast<u8 const*>(&chunk_size), sizeof(u32), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<u8 const*>(&rows_count), sizeof(u16), std::back_inserter(buffer));
    std::ranges::for_each(serialized_rows, [&buffer](auto sf) {
        std::copy_n(sf.begin(), sf.size(), std::back_inserter(buffer));
    });
    return buffer;
}

/********************************************************************
*                                                                   *
*                       F R O M   B Y T E S                         *
*                                                                   *
********************************************************************/

auto Result::
from_bytes(std::span<u8> span) ->
std::pair<Result,size_t> {
    size_t consumend_bytes = 0;

    if (!span.empty() && span.front() == 'T') {
        span = span.subspan(1);
        consumend_bytes = 1;
        if (auto const chunk_size = shared::from<u32>(span)) {
            span = span.subspan(sizeof(u32));
            consumend_bytes += sizeof(u32);
            if (span.size() >= *chunk_size) {
                span = span.first(*chunk_size);
                if (auto const rows_count = shared::from<u16>(span)) {
                    span = span.subspan(sizeof(u16));
                    consumend_bytes += sizeof(u16);

                    Result result{};
                    for (int i = 0; i < rows_count; ++i) {
                        auto [row, nbytes] = Row::from_bytes(span);
                        result.push_back(row);
                        span = span.subspan(nbytes);
                        consumend_bytes += nbytes;
                    }
                    return {std::move(result), consumend_bytes};
                }
            }
        }
    }

    return {{}, consumend_bytes};
}

/********************************************************************
*                                                                   *
*                      O P E R A T O R   ==                         *
*                                                                   *
********************************************************************/

auto Result::
operator==(Result const& rhs) const
-> bool {
    for (auto i = 0; i < data_.size(); ++i)
        if (data_[i] != rhs.data_[i])
            return false;
    return true;
}
