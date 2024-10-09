//
// Created by piotr on 09.10.24.
//

#pragma once
#include <string>
#include <utility>
#include "value.h"


class Field {
    std::pair<std::string, Value> data_;
public:
    Field() = default;
    ~Field() = default;
    Field(Field const&) = default;
    Field(Field&&) = default;
    Field& operator=(Field const&) = default;
    Field& operator=(Field&&) = default;

    Field(std::string name, Value value) : data_{std::move(name), std::move(value)} {}
    explicit Field(std::pair<std::string, Value> data) : data_{std::move(data)} {}

    auto operator()() && {
        return std::move(data_);
    }
    auto& operator()() const& {
        return data_;
    }
    [[nodiscard]] std::string name() const {
        return data_.first;
    }
    [[nodiscard]] Value value() const {
        return data_.second;
    }
    [[nodiscard]] std::string to_string() const {
        auto [name, value] = data_;
        return fmt::format("{}:[{}]", name, value.to_string());
    }
};
