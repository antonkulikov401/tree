#include <filesystem>
#include <fstream>
#include <random>
#include <limits>
#include <algorithm>
#include <iostream>
#include <string_view>
#include <cstdio>
#include <array>
#include <stdlib.h>

namespace fs = std::filesystem;

fs::path generate_path(fs::path path, std::mt19937& gen, size_t len = 20) {
    std::string name(len, 0);
    constexpr char alphabet[] = 
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::generate_n(name.begin(), len, [&gen, &alphabet]() { 
        return alphabet[gen() % (sizeof(alphabet) - 1)]; });
    return path / name;
}

std::string exec(std::string_view cmd) {
    std::string result;
    std::array<char, 128> buff;
    FILE* pipe = popen(cmd.data(), "r");
    while (fgets(buff.data(), buff.size(), pipe) != nullptr)
        result += buff.data();
    pclose(pipe);
    return result;
}

void generate_directory(fs::path path, size_t depth,
    std::mt19937& gen, size_t max_files = 5, size_t max_dirs = 5) {

    const size_t file_num = static_cast<size_t>(gen() % max_files);
    const size_t dir_num = static_cast<size_t>(gen() % max_dirs);
    fs::create_directory(path);
    if (gen() % 10) exec("chmod +t " + path.string());
    if (gen() % 10) exec("chmod o+w " + path.string());
    for (size_t i = 0; i < file_num; ++i) {
        auto file_path = generate_path(path, gen);
        std::ofstream file(file_path);
        file.close();
        if (gen() % 10 == 0) {
            auto symlink_path = file_path.string() + "_symlink";
            fs::create_symlink(file_path.filename(), symlink_path);
        }
    }
    if (depth > 0) {
        for (size_t i = 0; i < dir_num; ++i) {
            auto dir_path = generate_path(path, gen);
            generate_directory(dir_path, depth - 1, gen);
        }
    }
}

void random_tests(const std::string& path, size_t depth,
    size_t test_num, int seed = 0) {

    std::mt19937 gen(seed);
    for (size_t i = 0; i < test_num; ++i) {
        auto dir_name = generate_path(fs::path(""), gen);
        generate_directory(dir_name, depth, gen, 20, 20);
        std::string correct_output = exec("tree -a -C " + dir_name.string());
        std::string test_output = exec(path + " " + dir_name.string());
        fs::remove_all(dir_name);
        if (correct_output != test_output) {
            std::cout << "\033[01;31mBAD:\033[00m Failed on test #";
            std::cout << (i + 1) << std::endl << "Test output:\n";
            std::cout << test_output << "\nCorrect output:\n";
            std::cout << correct_output << std::endl;
            return;
        }
    }
    std::cout << "\033[01;32mOK:\033[00m Passed " << test_num
        << " tests!" << std::endl;
}

int main(int argc, char* argv[]) {
    random_tests(argv[1], atoi(argv[2]), atoi(argv[3]));
    return 0;
}
