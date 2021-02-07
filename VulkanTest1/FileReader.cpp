#include "FileReader.h"
#include <fstream>
std::vector<char> te::FileReader::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

bool te::FileReader::fileExists(const char* filename)
{
    std::ifstream f(filename);
    return !f.fail();
}