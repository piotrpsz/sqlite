//
// Created by piotr on 09.10.24.
//

#pragma once
#include <vector>
#include "row.h"

class Result {
    std::vector<Row> data_;
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
    void push_back(Row r) {
        data_.push_back(std::move(r));
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
