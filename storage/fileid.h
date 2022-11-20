#pragma once


struct FileId {
    int id;
};

struct PageId {
    FileId fileId;
    int id;

    friend bool operator<(const PageId &lhs, const PageId &rhs) {
        if (lhs.id == rhs.id) {
            return lhs.fileId.id < rhs.fileId.id;
        }
        return lhs.id < rhs.id;
    }
};

