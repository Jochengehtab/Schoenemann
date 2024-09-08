#pragma once

#include <iostream>
#include <fstream>
#include <ios>
#include <array>
#include <cstdint>
#include <sstream>

template <std::ios_base::openmode O>
class DataStream
{
    using FileStream = std::fstream;

protected:
    FileStream Stream;

public:
    explicit DataStream(const std::string &path)
    {
        Stream.open(path, O);
    }

    virtual ~DataStream()
    {
        Stream.close();
    }
};

class BinaryFileStream : public DataStream<std::ios::binary | std::ios::in>
{
private:
    std::string Path;
    std::stringstream MemoryStream;

public:
    BinaryFileStream(const unsigned char *data, size_t length) : DataStream(""), Path("")
    {
        MemoryStream.write(reinterpret_cast<const char *>(data), length);
        MemoryStream.seekg(0);
    }

    template <typename T, size_t Size>
    void ReadArray(std::array<T, Size> &array)
    {
        MemoryStream.read(reinterpret_cast<char *>(&array), sizeof(array));
    }
};
