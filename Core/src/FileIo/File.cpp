#include "File.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace Pulsar::FileIo {
    std::string ReadFile(const std::string &path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file.");
        }

        std::string data;
        std::string buffer;
        while (std::getline(file, buffer)) {
            data += buffer;
        }

        file.close();

        return data;
    }
}