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
#include "row.h"

auto Row::
to_string()
-> std::string {
    if (data_.empty())
        return {};

    // extracting keys and sorting them
    std::vector<std::string> keys{};
    keys.reserve(data_.size());
    std::ranges::for_each(data_, [&keys](auto p) {
        keys.emplace_back(p.first);
    });
    std::ranges::sort(keys);

    // creating a vector of values sorted by their names
    std::vector<std::string> buffer{};
    buffer.reserve(keys.size());
    for (const auto& key : keys) {
        buffer.emplace_back(data_[key].to_string());
    }

    // combining values into one string
    return shared::join(buffer);
}

/********************************************************************
*                                                                   *
*                         T O   B Y T E S                           *
*                                                                   *
********************************************************************/

auto Row::
to_bytes() const ->
std::vector<char> {
    std::vector<std::vector<char>> serialized_fields{};
    const u16 fields_count = data_.size();
    serialized_fields.reserve(fields_count);
    size_t values_size = 0;

    for (auto const& [_, field] : data_) {
        auto sf = field.to_bytes();
        values_size += sf.size();
        serialized_fields.push_back(sf);
    }

    u32 const chunk_size
        = sizeof(u16)   // fields number
        + values_size;  // bytes number of all fields

    std::vector<char> buffer{};
    buffer.reserve(sizeof(char) + sizeof(u32) + chunk_size);

    buffer.push_back('R');
    std::copy_n(reinterpret_cast<u8 const*>(&chunk_size), sizeof(u32), std::back_inserter(buffer));
    std::copy_n(reinterpret_cast<u8 const*>(&fields_count), sizeof(u16), std::back_inserter(buffer));
    std::ranges::for_each(serialized_fields, [&buffer](auto sf) {
        std::copy_n(sf.begin(), sf.size(), std::back_inserter(buffer));
    });

    return buffer;
}

/********************************************************************
*                                                                   *
*                       F R O M   B Y T E S                         *
*                                                                   *
********************************************************************/

auto Row::
from_bytes(std::span<const char> span) ->
std::pair<Row,size_t> {
    size_t consumed_bytes = 0;

    if (!span.empty() && span.front() == 'R') {
        span = span.subspan(1);
        consumed_bytes += 1;
        if (auto const chunk_size = shared::from<u32>(span)) {
            span = span.subspan(sizeof(u32));
            consumed_bytes += sizeof(u32);
            if (span.size() >= *chunk_size) {
                span = span.first(*chunk_size);
                if (auto const field_count = shared::from<u16>(span)) {;
                    span = span.subspan(sizeof(u16));
                    consumed_bytes += sizeof(u16);

                    Row row{};
                    for (int i = 0; i < field_count; ++i) {
                        auto [f, n] = Field::from_bytes(span);
                        row.add(f);
                        span = span.subspan(n);
                        consumed_bytes += n;
                    }
                    return {std::move(row), consumed_bytes};
                }
            }
        }
    }
    return {{}, consumed_bytes};
}

auto Row::
serialized_data(std::span<char> span)
-> std::string {
    if (span.size() >= sizeof(u8)) {
        std::string buffer{};
        buffer.append(fmt::format("0x{:02x} [{}]\n", span[0], static_cast<char>(span[0])));
        span = span.subspan(1);
        if (span.size() >= sizeof(u32)) {
            // total size
            auto span_total_size = span.subspan(0, sizeof(u32));
            size_t const total_size = *reinterpret_cast<u16*>(span_total_size.data());
            buffer.append(fmt::format("{} [{}]\n", shared::hex_bytes_as_str(span_total_size), total_size));
            span = span.subspan(sizeof(u32));
            if (span.size() >= sizeof(u16)) {
                // field count
                auto span_values_count = span.subspan(0, sizeof(u16));
                size_t const values_count = *reinterpret_cast<u16*>(span_values_count.data());
                buffer.append(fmt::format("{} [{}]\n", shared::hex_bytes_as_str(span_values_count), values_count));
                span = span.subspan(sizeof(u16));
                fmt::print("---------\n");
                for (size_t i = 0; i < values_count; ++i) {
                    // we take bytes describing the size of the field
                    auto span_size = span.subspan(0, sizeof(u32));
                    auto const size = *reinterpret_cast<u32*>(span_size.data());
                    span = span.subspan(sizeof(u32));

                    auto span_field = span.subspan(0, size);
                    auto [f, n] = Field::from_bytes(span_field);
                    buffer.append(fmt::format("{} [{}]\n", Field::serialized_data(span.subspan(0, n)), n));
                    span = span.subspan(n);
                }
            }
            return buffer;
        }
    }
    return {};
}

/********************************************************************
*                                                                   *
*                      O P E R A T O R   ==                         *
*                                                                   *
********************************************************************/

auto Row::
operator==(Row const& rhs) const
-> bool {
    auto rhs_data = rhs.data_;
    auto this_data = data_;

    // extracting keys and sorting them
    std::vector<std::string> keys{};
    keys.reserve(data_.size());
    std::ranges::for_each(data_, [&keys](auto p) {
        keys.emplace_back(p.first);
    });

    for (auto const& key : keys)
        if (this_data[key] != rhs_data[key])
            return false;
    return true;
}
