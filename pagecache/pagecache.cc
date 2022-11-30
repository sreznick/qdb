#include "pagecache.h"

#include <utility>
#include <algorithm>
#include <cassert>

int PageCache::sync() {
    for (auto &page: dirty) {
        storage.write(cache[page2index[page]], page);
    }
    dirty.clear();
    return 0;
}

PageCache::PageCache(Storage storage, PageCacheConfig config) : storage(std::move(storage)), config(config) {
    cache = new std::byte* [config.pageCount];
    for (int i = 0; i < config.pageCount; i++) {
        cache[i] = nullptr;
    }
    access_counters = std::vector<int>(config.pageCount, 0);
    next_free_slot = 0;
    last_checked_position = 0;
}

PageCache::~PageCache() {
    sync();
    for (int i = 0; i < config.pageCount; i++) {
        delete[] cache[i];
    }
    delete[] cache;
}

int PageCache::write(PageId pageId) {
    if (pageId.id < 0) {
        return -2;
    }
    if (page2index.count(pageId) > 0) {
        dirty.insert(pageId);
        return 0;
    }
    return -1;
}

std::byte* PageCache::read_page(PageId pageId) {
    if (pageId.id < 0) {
        return nullptr;
    }
    auto it = page2index.find(pageId);
    if (it != page2index.end()) {
        access_counters[it->second] = std::min(access_counters[it->second] + 1, config.countLimit);
        return cache[it->second];
    }
    PageId victim = find_victim();
    int position = page2index[victim];
    remove_page(victim);
    insert_page_into(pageId, position);
    storage.read(cache[position], pageId);
    return cache[position];
}

PageId PageCache::create_page(FileId fileId) {
    PageId pageId = storage.create_page(fileId);
    if (pageId.id < 0) {
        return PageId {0, -1};
    }
    auto data = static_cast<std::byte>(0);
    storage.write(&data, pageId);
    if (next_free_slot == config.pageCount) {
        PageId victim = find_victim();
        int position = page2index[victim];
        remove_page(victim);
        insert_page_into(pageId, position);
    } else {
        insert_page_into(pageId, next_free_slot++);
    }
    return pageId;
}

PageId PageCache::find_victim() {
    while (true) {
        if (access_counters[last_checked_position] == 0) {
            auto page = index2page[last_checked_position];
            last_checked_position = (last_checked_position + 1) % config.pageCount;
            return page;
        } else {
            --access_counters[last_checked_position];
            last_checked_position = (last_checked_position + 1) % config.pageCount;
        }
    }
}

void PageCache::insert_page_into(PageId pageId, int position) {
    page2index[pageId] = position;
    index2page[position] = pageId;
    if (cache[position] == nullptr) {
        cache[position] = new std::byte[DEFAULT_PAGE_SIZE];
    }
    std::fill(cache[position], cache[position] + DEFAULT_PAGE_SIZE, static_cast<std::byte>(0));
}

void PageCache::remove_page(PageId pageId) {
    auto position = page2index[pageId];
    if (dirty.contains(pageId)) {
        storage.write(cache[position], pageId);
        dirty.erase(pageId);
    }
    index2page.erase(position);
    page2index.erase(pageId);
}