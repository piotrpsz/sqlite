//
// Created by piotr on 09.10.24.
//
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include <string_view>
namespace fs = std::filesystem;
using namespace std::string_literals;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i64 = uint64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
using uint = unsigned int;

template<typename T>
    using Vec = std::vector<T>;
using Str = std::string;
using StrView = std::string_view;

