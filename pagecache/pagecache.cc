#include "pagecache.h"

PageId PageCache::find_victim() {
    for (int epoch = 0;;epoch++) {
        for (int i = lastVictimPosition; i < _config.pageCount; i++) {
            if (_pageIdByPosition.find(i) == _pageIdByPosition.end()) { continue; }

            PageId pageId = _pageIdByPosition.at(i);
            int pageValue = (_values.find(pageId) != _values.end()) ? _values.at(pageId) : 0;

            if (pageValue <= 0) {
                lastVictimPosition = (i + 1) % _config.pageCount;
                return pageId;
            } else {
                _values[pageId] = pageValue - 1;
            }
        }
        lastVictimPosition = 0;
    }
}

void PageCache::incrementCounter(PageId pageId) {
    int nextValue = (_values.find(pageId) != _values.end()) ? _values.at(pageId) + 1 : 1;
    _values[pageId] = std::min(nextValue, _config.countLimit);
}

void PageCache::insertPage(PageId pageId, int position) {
    _cachePosition[pageId] = position;
    _pageIdByPosition[position] = pageId;
    _isDirty[pageId] = false;
    auto data = new std::byte[DEFAULT_PAGE_SIZE];
    _cache[position] = data;
}

void PageCache::removePage(PageId pageId) {
    if (_isDirty.contains(pageId) && _isDirty.at(pageId)) { // flush dirty page on disk
        _storage.write(_cache[_cachePosition.at(pageId)], pageId);
    }
    _cachePosition.erase(pageId);
    _values[pageId] = 0;
}

PageId PageCache::create_page(FileId fileId) {
    PageId pageId = _storage.create_page(fileId);
    if (pageId.id < 0) { return PageId{0, -1}; }

    std::byte nullByte = static_cast<std::byte>(0);
    _storage.write(&nullByte, pageId);

    if (capacity == _config.pageCount) {
        PageId victimPageId = find_victim();
        int victimPagePosition = _cachePosition.at(victimPageId);

        removePage(victimPageId);
        insertPage(pageId, victimPagePosition);
    } else {
        insertPage(pageId, capacity);

        capacity++;
    }

    return pageId;
}

std::byte* PageCache::read_page(PageId pageId) {
    if (pageId.id < 0) {
        return nullptr;
    }

    if (_cachePosition.find(pageId) != _cachePosition.end()) {
        incrementCounter(pageId);
        return _cache[_cachePosition.at(pageId)];
    } else {
        std::byte* data = new std::byte[DEFAULT_PAGE_SIZE];
        int res = _storage.read(data, pageId);

        if (res < 0) {
            std::cerr << "ERROR: PageCache#read_page" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (capacity == _config.pageCount) {
            PageId victimPageId = find_victim();
            int victimCachePosition = _cachePosition.at(victimPageId);
            removePage(victimPageId);

            insertPage(pageId, victimCachePosition);
            _cache[victimCachePosition] = data;
        } else {
            insertPage(pageId, capacity);
            _cache[capacity] = data;
            capacity++;
        }

        return data;
    }
}

int PageCache::write(PageId pageId, std::byte* data) {
    if (pageId.id < 0) { return -2; }

    if (_cachePosition.find(pageId) != _cachePosition.end()) {
        _isDirty[pageId] = true;
        _cache[_cachePosition.at(pageId)] = data;
        return 0;
    } else {
        return -1;
    }
}

int PageCache::sync() {
    std::map<PageId, bool>::iterator iterator;

    for (iterator = _isDirty.begin(); iterator != _isDirty.end(); iterator++) {
        if (!_isDirty[iterator->first]) { continue; }

        if (_cachePosition.find(iterator->first) != _cachePosition.end()) {
            int pagePosition = _cachePosition.at(iterator->first);
            int res = _storage.write(_cache[pagePosition], iterator->first);
        }
        _isDirty[iterator->first] = false;
    }

    return 0;
}

// NOTE: only for test purposes
// encapsulation is good, but we don't have any other way
// to verify algorithm correctness but check internal structures.
std::map<PageId, int, pageIdComparator>* PageCache::_getValues() {
    return &_values;
}

std::map<PageId, bool, pageIdComparator>* PageCache::_getIsDirty() {
    return &_isDirty;
}