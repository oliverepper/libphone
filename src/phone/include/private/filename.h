//
// Created by Oliver Epper on 18.04.23.
//

#ifndef PHONE_FILENAME_H
#define PHONE_FILENAME_H

#include <filesystem>

std::string filename(const std::string& filepath) {
    std::filesystem::path path(filepath);
    return path.filename().string();
}

#endif //PHONE_FILENAME_H
