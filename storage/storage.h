#pragma once

#include <filesystem>
#include <iostream>
#include <map>

#include "fileid.h"
#include "config.h"

namespace fs = std::filesystem;

const int DEFAULT_PAGE_SIZE = 8192;

class Storage {
private:
    fs::path _root;
    StorageConfig _config;
    std::map<int, int> _openFiles;

    fs::path file_path(FileId id);

public:
    Storage(fs::path root): _root(root), _config({DEFAULT_PAGE_SIZE}), _openFiles{} {}
    Storage(fs::path root, StorageConfig config): _root(root), _config(config), _openFiles{} {}

    bool is_present();
    bool can_initialize();
    void initialize();

    void reset(FileId id);
    void close(FileId id);

    int read(std::byte* data, PageId id);
    PageId create_page(FileId id);
    int write(std::byte* data, PageId id);

    int page_size();
};

