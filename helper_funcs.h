#ifndef HELPER_FUNCS_H
#define HELPER_FUNCS_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <filesystem>
#include <cctype>

#include <qstring.h>

void remove_duplicate_lines_from_file(const std::string& filePath);
std::string get_fullpath_by_filename(const std::list<std::string>& v_full_paths, const QString& filename);
QString get_env(const std::string &env_var);

#endif // HELPER_FUNCS_H
