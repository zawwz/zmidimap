#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <ztd/filedat.hpp>

ztd::chunkdat mimtochk(const std::string& mim);

std::string file_strimport(const std::string& path);

bool is_mim(const std::string& str);

#endif //FORMAT_HPP
