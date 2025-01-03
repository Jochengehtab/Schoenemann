#pragma once

#include <iostream>
#include <fstream>
#include <array>
#include <cstdint>
#include <sstream>

class memorystream
{
private:
    std::string path;
    std::stringstream memoryStream;

public:
    memorystream(const unsigned char *data, size_t length)
    {
        memoryStream.write(reinterpret_cast<const char *>(data), length);
        memoryStream.seekg(0);
    }

    template <size_t Size>
    void readArray(std::array<std::int16_t, Size> &array)
    {
        memoryStream.read(reinterpret_cast<char *>(&array), sizeof(array));
    }
    template <size_t OuterSize, size_t InnerSize>
void readArray(std::array<std::array<std::int16_t, InnerSize>, OuterSize> &array)
{
    for (auto &innerArray : array)
    {
        memoryStream.read(reinterpret_cast<char *>(innerArray.data()), sizeof(std::int16_t) * InnerSize);
    }
}
};
