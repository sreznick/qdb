#ifndef QDB_BTREE_H
#define QDB_BTREE_H

#include "stdio.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "pagecache/pagecache.h"

struct Record {
    std::int32_t value;
    std::int32_t pointer;
};

class BTreeNode {
private:
    int const MAX_RECORDS_COUNT = 2001; // approx. 8kb / 4
    int const META_SIZE = 4; // 4 for records counter
    int const RECORD_SIZE = 8; // 4 for key, 4 for pointer

    std::byte* _page;
    int _pageId;
    int _recordsCount;

    void _init_state();
    void _assert_not_full();
public:
    BTreeNode(std::byte* page, int pageId): _page(page), _pageId(pageId) {
        _init_state();
    }

    Record at(int index);
    void insert(Record record);
    bool is_full();
    void reassign_records(BTreeNode* node, BTreeNode* parent, BTreeNode* sibling);

    std::byte* page() {
        return _page;
    }

    int page_id() {
        return _pageId;
    }

    int records_count() {
        return _recordsCount;
    }
};

#endif //QDB_BTREE_H

class BTree {
private:
    int const FAKE_RECORD_VALUE = -1000000;

    std::byte* _metaPage;
    std::byte* _rootPage;

    FileId _fileId;

    int _metaPageId;
    int _rootPageId;

    std::int32_t _depth;
    std::int32_t _recordsCount = 0;

    std::shared_ptr<PageCache> _pageCachePtr;

    void _init_state(std::shared_ptr<PageCache> pageCachePtr) {
        std::memcpy(&_depth, _metaPage, 4);
        std::memcpy(&_rootPageId, _metaPage + 4, 4);
        std::memcpy(&_recordsCount, _rootPage, 4);
        _metaPageId = 0;

        if (_recordsCount == 0) insert_fake_record();
    }

    void insert_fake_record();

    void update_meta_page() {
        memcpy(_metaPage, &_depth, 4);
        _pageCachePtr.get()->write(PageId{{_fileId}, _metaPageId}, _metaPage);
    }

    PageId _create_page() {
        return _pageCachePtr.get()->create_page(_fileId);
    }
    std::byte* _read_new_page(PageId pageId) {
        return _pageCachePtr.get()->read_page(pageId);
    }
public:
    BTree(
            std::byte* metaPage,
            std::byte* rootPage,
            std::shared_ptr<PageCache> pageCachePtr,
            FileId fileId): _metaPage(metaPage), _rootPage(rootPage), _pageCachePtr(pageCachePtr), _fileId(fileId) {
        _init_state(pageCachePtr);
    }

    Record find(int value);
    int insert(Record record);
    void persist(BTreeNode* node);
};

