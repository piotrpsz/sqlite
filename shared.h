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
#include <span>
#include <string>
#include <optional>
#include <charconv>
#include <format>
#include <iostream>
#include <range/v3/all.hpp>

namespace shared {
    namespace rng = ranges;
    namespace vws = rng::views;

    /// Join strings with 'delimiter'.
    /// \param data - source span of strings,
    /// \param delimiter - a character inserted between strings
    /// \param spacer - a character added after delimiter (space as default)
    /// \return connection result as string
    static inline std::string join(std::span<const std::string> const data, char const delimiter = ',', std::optional<char> const spacer = {}) noexcept {
        if (data.empty())
            return {};

        auto n = std::accumulate(
                data.begin(),
                data.end(),
                uint{0},
                [](uint const count, std::string_view str) {
                    return count + str.size();
                }
        );

        n += ((data.size() - 1) * 2);   // 2 := delimiter + one space after
        std::string buffer{};
        buffer.reserve(n);

        for (auto it = data.begin(); it != std::prev(data.end()); ++it) {
            buffer.append(*it);
            buffer.push_back(delimiter);
            if (spacer)
                buffer.push_back(*spacer);
        }
        buffer += *std::prev(data.end());
        return std::move(buffer);
    }

    /// Generate hex string representation of vector of bytes.
    /// \param data - span with bytes
    /// \return string representation of bytes.
    static inline std::string hex_bytes_as_str(std::span<const u8> const data) noexcept {
        auto const components = data
                      | vws::transform([](u8 const c) { return std::format("0x{:02x}", c); })
                      | rng::to<std::vector>();
        return join(components, ',');
    }

    static inline std::string hex_bytes_as_str(std::span<const char> const data) noexcept {
        auto const components = data
                      | vws::transform([](u8 const c) { return std::format("0x{:02x}", c); })
                      | rng::to<std::vector>();
        return join(components, ',');
    }

    static inline std::optional<int> to_int(std::string_view sv, int base = 10) {
        int value{};
        auto [_, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value, base);

        // Konwersja się udała.
        if (ec == std::errc{})
            return value;

        // Coś poszło nie tak.
        if (ec == std::errc::invalid_argument)
            std::cerr << std::format("This is not a number ({}).\n", sv);
        else if (ec == std::errc::result_out_of_range)
            std::cerr << std::format("The number is to big ({}).\n", sv);
        return {};
    }

    static inline std::string to_string(std::integral auto v) {
        return std::to_string(static_cast<i64>(v));
    }
    static inline std::string to_string(std::floating_point auto v) {
        return std::to_string(static_cast<f64>(v));
    }

    template<std::integral T>
    std::optional<T> from(std::span<const char> const span) noexcept {
        if (span.size() >= sizeof(T))
            return *reinterpret_cast<T const*>(span.first(sizeof(T)).data());
        return {};
    }
    template<std::integral T>
    std::optional<T> from(std::span<char> const span) noexcept {
        if (span.size() >= sizeof(T))
            return *reinterpret_cast<T*>(span.first(sizeof(T)).data());
        return {};
    }
}
