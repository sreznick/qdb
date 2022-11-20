#include "pagecache.h"

#include <utility>
#include <algorithm>
#include <cassert>

int PageCache::sync() {
    for (auto &page: _dirty) {
        _storage.write(_cache[_page2position[page]], page);
    }
    _dirty.clear();
    return 0;
}

int PageCache::write(PageId pageId) {
    if (pageId.id < 0) {
        return -2;
    }
    if (_page2position.count(pageId)) {
        _dirty.insert(pageId);
        return 0;
    }
    return -1;
}

std::byte *PageCache::read_page(PageId pageId) {
    if (pageId.id < 0) {
        return nullptr;
    }
    auto it = _page2position.find(pageId);
    if (it != _page2position.end()) {
        _counter[it->second] = std::min(_counter[it->second] + 1, _config.countLimit);
        return _cache[it->second];
    }
    PageId victim = find_victim();
    int pos = _page2position[victim];
    remove_page(victim);
    insert_page_into(pageId, pos);
    _storage.read(_cache[pos], pageId);
    return _cache[pos];
}

PageId PageCache::create_page(FileId fileId) {
    PageId pageId = _storage.create_page(fileId);
    if (pageId.id < 0) {
        return PageId{0, -1};
    }
    auto data = static_cast<std::byte>(0);
    _storage.write(&data, pageId);
    if (_initializerPosition == _config.pageCount) {
        PageId victim = find_victim();
        int pos = _page2position[victim];
        remove_page(victim);
        insert_page_into(pageId, pos);
    } else {
        insert_page_into(pageId, _initializerPosition++);
    }
    return pageId;
}

PageCache::PageCache(Storage storage, PageCacheConfig config) : _storage(std::move(storage)), _config(config) {
    _initializerPosition = 0;
    _lastPosition = -1;
    _counter = std::vector<int>(_config.pageCount, 0);
    _cache = new std::byte *[_config.pageCount];
    for (int i = 0; i < _config.pageCount; i++) {
        _cache[i] = nullptr;
    }
}

PageId PageCache::find_victim() {
    while (true) {
        _lastPosition = (_lastPosition + 1) % _config.pageCount;
        if (_counter[_lastPosition] == 0) {
            return _position2page[_lastPosition];
        } else {
            --_counter[_lastPosition];
        }
    }
}

void PageCache::insert_page_into(PageId pageId, int position) {
    _page2position[pageId] = position;
    _position2page[position] = pageId;
    if (_cache[position] == nullptr) {
        _cache[position] = new std::byte[DEFAULT_PAGE_SIZE];
    }
    std::fill(_cache[position], _cache[position] + DEFAULT_PAGE_SIZE, static_cast<std::byte>(0));
}

void PageCache::remove_page(PageId pageId) {
    auto position = _page2position[pageId];
    assert(_counter[position] == 0);
    if (_dirty.contains(pageId)) {
        _storage.write(_cache[position], pageId);
        _dirty.erase(pageId);
    }
    _page2position.erase(pageId);
}

PageCache::~PageCache() {
    sync();
    for (int i = 0; i < _config.pageCount; i++) {
        delete[] _cache[i];
    }
    delete[] _cache;
}
