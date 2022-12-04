#pragma once


struct FileId {
    int id;
};

struct PageId {
    FileId fileId;
    int id;

    friend bool operator<(const PageId &a, const PageId &b) {
         return a.id < b.id || (a.id == b.id && a.fileId.id < b.fileId.id);
     }
};

