//
// Created by piotr on 09.10.24.
//

#pragma once
#include <string>
#include <utility>
#include <optional>
#include <unordered_map>
#include "field.h"

class Row {
    std::unordered_map<std::string,Field> data_;
public:
    Row() = default;
    ~Row() = default;
    Row(Row const&) = default;
    Row& operator=(Row const&) = default;
    Row(Row&&) = default;
    Row& operator=(Row&&) = default;

    Row(std::string name, Value value) {
        const auto f = Field{std::move(name), std::move(value)};
        data_[f.name()] = f;
    }

    bool empty() const {
        return data_.empty();
    }
    std::optional<Field> operator[](std::string const& name) {
        if (data_.contains(name)) return data_[name];
        return {};
    }
    auto size() const {
        return data_.size();
    }

    Row& add(Field const& f) noexcept {
        data_[f.name()] = f;
        return *this;
    }
    Row& add(std::string name, Value value) noexcept {
        Field const f{std::move(name), std::move(value)};
        return add(f);
    }
    Row& add(std::string name) noexcept {
        Field f{std::move(name), {}};
        return add(f);
    }
    template<typename T>
    Row& add(std::string name, std::optional<T> value) noexcept {
        return value
               ? add(std::move(name), std::move(*value))
               : add(std::move(name));
    }

    /****************************************************************
    *                                                               *
    *                      I T E R A T O R S                        *
    *                                                               *
    ****************************************************************/

    using iterator = std::unordered_map<std::string,Field>::iterator;
    using const_iterator = std::unordered_map<std::string,Field>::const_iterator;
    iterator begin() noexcept { return data_.begin(); }
    iterator end() noexcept { return data_.end(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    const_iterator cend() const noexcept { return data_.cend(); }

    /// We store pairs (name, field) in a map, we separate the data into two vectors.
    /// Separate names, separate fields.
    std::pair<std::vector<std::string>, std::vector<Value>> split() const noexcept {
        std::vector<std::string> names{};
        std::vector<Value> values{};

        if (!data_.empty()) {
            auto const n = data_.size();
            names.reserve(n);
            values.reserve(n);
            for (auto const& [name, field] : data_) {
                names.emplace_back(name);
                values.emplace_back(field.value());
            }
        }
        return {names, values};
    }

    std::string to_string() const noexcept {
        std::vector<std::string> buffer{};
        buffer.reserve(data_.size());

        for (const auto&[_, f] : data_) {
            buffer.emplace_back(f.to_string());
        }
        return shared::join(buffer);
    }
};
