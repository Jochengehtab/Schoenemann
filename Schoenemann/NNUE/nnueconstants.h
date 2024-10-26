#pragma once

#include <iostream>

constexpr std::uint16_t inputSize = 768;
constexpr std::uint16_t hiddenSize = 512;
constexpr std::uint16_t outputSize = 1;
constexpr std::uint16_t scale = 200;

constexpr std::uint8_t QA = 255;
constexpr std::uint8_t QB = 64;
constexpr std::uint32_t inputHidden = inputSize * hiddenSize;
constexpr bool activate = true;
constexpr bool deactivate = false;

constexpr std::uint16_t blackSqures = 64 * 6;
constexpr std::uint8_t whiteSquares = 64;