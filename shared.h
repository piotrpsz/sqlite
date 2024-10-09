//
// Created by piotr on 09.10.24.
//

#pragma once
#include "types.h"
#include <string>
#include <optional>
#include <span>
#include <fmt/core.h>
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
                      | vws::transform([](u8 const c) { return fmt::format("0x{:02x}", c); })
                      | rng::to<std::vector>();
        return join(components, ',');
    }
}
