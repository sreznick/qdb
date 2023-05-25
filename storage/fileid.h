#pragma once


struct FileId {
    int id;
};

struct PageId {
    FileId fileId;
    int id;

    friend bool operator<(const PageId &fst, const PageId &snd) {
        return fst.fileId.id < snd.fileId.id || (fst.fileId.id == snd.fileId.id && fst.id < snd.id);
    }
};

