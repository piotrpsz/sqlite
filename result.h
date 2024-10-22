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
#include <vector>
#include "row.h"

class Result {
    std::vector<Row> data_;
    static constexpr char RESULT_MARKER{'T'};
public:
    Result() = default;
    ~Result() = default;
    Result(const Result&) = default;
    Result(Result&&) = default;
    Result& operator=(const Result&) = default;
    Result& operator=(Result&&) = default;

    [[nodiscard]] auto empty() const {
        return data_.empty();
    }
    [[nodiscard]] auto size() const {
        return data_.size();
    }
    auto operator[](int const i) const {
        return data_[i];
    }
    [[nodiscard]] auto at(int const i) const {
        return data_.at(i);
    }
    Result& add(Row&& r) {
        data_.push_back(std::move(r));
        return *this;
    }
    Result& add(Row const& r) {
        data_.push_back(r);
        return *this;
    }


    /// Serialization. Converting a Field to bytes.
    [[nodiscard]] auto to_bytes() const -> std::vector<char>;
    [[nodiscard]] auto to_gzip_bytes() const -> std::vector<char>;

    /// Deserialization. Recreate Field from bytes.
    static auto from_bytes(std::span<const char> span) -> std::pair<Result,size_t>;
    static auto from_gzip_bytes(std::span<const char> span) -> std::pair<Result,size_t>;

    auto to_string() const -> std::string;

    /// Serialized data info. Generally for debug.
    // static auto serialized_data(std::span<u8> span) -> std::string;

    auto operator==(Result const& rhs) const noexcept -> bool;
    auto operator!=(Result const& rhs) const noexcept -> bool {
        return !(*this == rhs);
    }

    /****************************************************************
    *                                                               *
    *                      I T E R A T O R S                        *
    *                                                               *
    ****************************************************************/

    using iterator = std::vector<Row>::iterator;
    using const_iterator = std::vector<Row>::const_iterator;
    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }
    [[nodiscard]] const_iterator cbegin() const { return data_.cbegin(); }
    [[nodiscard]] const_iterator cend() const { return data_.cend(); }
};
