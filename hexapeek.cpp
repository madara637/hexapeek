// build: g++ -std=c++17 -O2 hexapeek.cpp -o hexapeek

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

namespace fs = std::filesystem;

#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_CYAN    "\033[36m"

struct Entry {
    std::string path;
    uintmax_t size;
    bool is_dir;
};

int skipped_count = 0;

std::string human_size(uintmax_t bytes) {
    double b = (double)bytes;
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;

    while (b >= 1024.0 && i < 4) {
        b /= 1024.0;
        i++;
    }

    char buf[64];

    if (i == 0) {
        std::snprintf(buf, sizeof(buf), "%.0f %s", b, units[i]);
    } else {
        std::snprintf(buf, sizeof(buf), "%.1f %s", b, units[i]);
    }

    return std::string(buf);
}

uintmax_t dir_size(const fs::path& p) {
    uintmax_t total = 0;

    std::error_code ec;
    fs::directory_iterator it(
        p,
        fs::directory_options::skip_permission_denied,
        ec
    );

    if (ec) {
        skipped_count++;
        return 0;
    }

    for (const auto& entry : it) {
        std::error_code ec2;

        if (fs::is_symlink(entry.path(), ec2)) {
            continue;
        }

        if (entry.is_directory(ec2)) {
            total += dir_size(entry.path());
        } else if (entry.is_regular_file(ec2)) {
            uintmax_t fsize = fs::file_size(entry.path(), ec2);

            if (!ec2) {
                total += fsize;
            } else {
                skipped_count++;
            }
        }
    }

    return total;
}

void print_help(const char* prog) {
    std::cout << "diskpeek - see what's hogging your disk space\n\n";
    std::cout << "usage:\n";
    std::cout << "  " << prog << " <directory> [options]\n\n";
    std::cout << "options:\n";
    std::cout << "  -n <num>    how many results to show (default 20)\n";
    std::cout << "  --help      show this\n\n";
    std::cout << "example:\n";
    std::cout << "  " << prog << " /home/me/Downloads -n 10\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    std::string dir_arg;
    int top_n = 20;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];

        if (a == "--help" || a == "-h") {
            print_help(argv[0]);
            return 0;
        } else if (a == "-n") {
            if (i + 1 < argc) {
                top_n = std::atoi(argv[i + 1]);
                i++;
            }
        } else {
            dir_arg = a;
        }
    }

    if (dir_arg.empty()) {
        std::cerr << "no directory given, try --help\n";
        return 1;
    }

    fs::path root(dir_arg);

    std::error_code ec;

    if (!fs::exists(root, ec) || ec) {
        std::cerr << C_RED << "error: " << dir_arg
                  << " doesn't exist\n" << C_RESET;
        return 1;
    }

    if (!fs::is_directory(root, ec) || ec) {
        std::cerr << C_RED << "error: " << dir_arg
                  << " is not a directory\n" << C_RESET;
        return 1;
    }

    std::cout << "scanning " << C_CYAN << dir_arg << C_RESET << " ...\n";

    std::vector<Entry> entries;

    fs::directory_iterator top_it(
        root,
        fs::directory_options::skip_permission_denied,
        ec
    );

    if (ec) {
        std::cerr << C_RED
                  << "cant open that directory (permissions?)\n"
                  << C_RESET;
        return 1;
    }

    for (const auto& item : top_it) {
        std::error_code e2;

        if (fs::is_symlink(item.path(), e2)) {
            continue;
        }

        if (item.is_directory(e2)) {
            uintmax_t sz = dir_size(item.path());
            entries.push_back({item.path().string(), sz, true});
        } else if (item.is_regular_file(e2)) {
            uintmax_t sz = fs::file_size(item.path(), e2);

            if (e2) {
                skipped_count++;
                continue;
            }

            entries.push_back({item.path().string(), sz, false});
        }
    }

    std::sort(entries.begin(), entries.end(),
        [](const Entry& a, const Entry& b) {
            return a.size > b.size;
        });

    uintmax_t grand_total = 0;

    for (auto& e : entries) {
        grand_total += e.size;
    }

    std::cout << "\n"
              << C_BOLD
              << "top space users in " << dir_arg
              << C_RESET << "\n";

    std::cout << "----------------------------------------\n";

    int count = 0;

    for (auto& e : entries) {
        if (count >= top_n)
            break;

        std::string sizestr = human_size(e.size);

        if (e.is_dir) {
            std::cout << C_YELLOW << sizestr << C_RESET
                      << "\t[dir]  " << e.path << "\n";
        } else {
            std::cout << C_GREEN << sizestr << C_RESET
                      << "\t[file] " << e.path << "\n";
        }

        count++;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "total scanned: "
              << human_size(grand_total) << "\n";

    if (skipped_count > 0) {
        std::cout << C_RED
                  << "(skipped " << skipped_count
                  << " items, probably permission errors)"
                  << C_RESET << "\n";
    }

    return 0;
}