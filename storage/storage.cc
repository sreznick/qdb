#include "storage.h"

#include <fstream>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int write_single(int fd, std::byte* data, int size) {
    int nWritten = 0;
    while (nWritten < size) {
        int n = write(fd, data + nWritten, size - nWritten);
        if (n < 0) {
            return -200;
        }
        nWritten += n;
    }
    return 0; 
}

int read_single(int fd, std::byte* data, int size) {
    int nRead = 0;

    while (nRead < size) {
        int n = read(fd, data + nRead, size - nRead);
        if (n < 0) {
            return -200;
        }
        nRead += n;
    }
    return 0;
}


bool Storage::is_present() {
    if (!fs::is_directory(_root)) return false;

    fs::path qdb = _root / ".qdb";
    if (!fs::is_regular_file(qdb)) return false;

    return true;
}

bool Storage::can_initialize() {
    if (!fs::exists(_root)) {
       return true;
    }

    if (!fs::is_directory(_root)) return false;

    auto it = fs::directory_iterator(_root);
    if (it != fs::end(it)) return false;

    return true;
}

void Storage::initialize() {
    if (!fs::exists(_root)) {
       fs::create_directory(_root);
    }

    std::ofstream output(_root / ".qdb");
}

void Storage::reset(FileId id) {
    std::string name = std::to_string(id.id);
   
    std::ofstream output(_root / name);
}

void Storage::close(FileId id) {
    if (_openFiles.contains(id.id)) {
        int fid = _openFiles[id.id];
        ::close(fid);
    }
}
    
int Storage::read(std::byte* data, PageId pageId) {
    // PATCH START
    if (!_openFiles.contains(pageId.fileId.id)) {
        const char* name = file_path(pageId.fileId).c_str();
        //std::cout << "going to open file " << name << std::endl;

        int fd = open(name, O_CREAT | O_RDWR, 0644);
        //std::cout << "fd: " << fd << std::endl;
        if (fd < 0) { return -1; }

        _openFiles[pageId.fileId.id] = fd;
    }
    // PATCH END

    int fd = _openFiles[pageId.fileId.id];

    int pos = lseek(fd, pageId.id * _config.pageSize, SEEK_SET);
    if (pos < 0) {
        return pos;
    }
    if (pos != pageId.id * _config.pageSize) {
        return -100;
    }

    int nCopied = read_single(fd, data, _config.pageSize);
    if (nCopied < 0) {
        return -200;
    }

    return pageId.id;
}

int Storage::write(std::byte* data, PageId pageId) {
    if (_openFiles.contains(pageId.fileId.id)) {
        int fd = _openFiles[pageId.fileId.id];

        int pos = lseek(fd, pageId.id * _config.pageSize, SEEK_SET);
        if (pos < 0) {
            return pos;
        }
        if (pos != pageId.id * _config.pageSize) {
            return -100;
        }

        int nCopied = write_single(fd, data, _config.pageSize);
        if (nCopied < 0) {
            return -200;
        }
    }

    return pageId.id;
}

fs::path Storage::file_path(FileId id) {
    std::string name = std::to_string(id.id);
    return _root / name;
}

PageId Storage::create_page(FileId id) {
    if (!_openFiles.contains(id.id)) {
        const char* name = file_path(id).c_str();
        //std::cout << "going to open file " << name << std::endl;

        int fd = open(name, O_CREAT | O_RDWR, 0644);
        //std::cout << "fd " << fd << std::endl;
        if (fd < 0) {
            return {id, -1};
        }

        _openFiles[id.id] = fd;
    }

    int fd = _openFiles[id.id];
    int pos = lseek(fd, 0, SEEK_END);
    if (pos < 0) {
        return {id, -1};
    }

    //std::cout << pos << std::endl;

    std::byte data[_config.pageSize];
    memset(data, 0, _config.pageSize);
    int nCopied = write_single(fd, data, _config.pageSize);

    if (nCopied < 0) {
        return {id, -1};
    }

    return {id, pos / _config.pageSize};
}

