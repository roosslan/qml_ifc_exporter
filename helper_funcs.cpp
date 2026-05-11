#include "helper_funcs.h"

void remove_duplicate_lines_from_file(const std::string& file_path) {
    std::ifstream input_file(file_path);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open file for reading!" << std::endl;
        return;
    }

    std::vector<std::string> unique_lines;
    std::unordered_set<std::string> seen_lines;
    std::string line;

    // Step 1: Read all unique lines into memory while preserving order
    while (std::getline(input_file, line)) {
        if (seen_lines.insert(line).second) {
            unique_lines.push_back(line);
        }
    }
    input_file.close(); // Essential: Close the file before reopening to write

    // Step 2: Overwrite the original file with the unique lines
    std::ofstream output_file(file_path, std::ios::trunc); // std::ios::trunc clears the file
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return;
    }

    for (const auto& uniqueLine : unique_lines) {
        output_file << uniqueLine << "\n";
    }
    output_file.close();
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}
std::string get_fullpath_by_filename(const std::list<std::string>& v_full_paths, const QString& filename) {
    std::string lowerFilename = to_lower(filename.toStdString());

    auto it = std::find_if(v_full_paths.begin(), v_full_paths.end(),
                           [&lowerFilename](const std::string& fullPath) {
                               return to_lower(std::filesystem::path(fullPath).filename().string())
                               == lowerFilename;
                           });

    return (it != v_full_paths.end()) ? *it : "";
}
