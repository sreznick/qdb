#include "pagecache.h"

PageCache::PageCache(Storage storage, PageCacheConfig config): _storage(storage), _config(config) {
    _cap = 0;
    _position_state = 0;
    _cache = new std::byte*[_config.pageCount];
}

std::map<PageId, PageData, CMP>* PageCache::_getData() {
    return &_data;
}

PageId PageCache::create_page(FileId fileId) {
    PageId pageId = _storage.create_page(fileId);
    
    if (pageId.id < 0) {
        return PageId{0, -1};
    }

    std::byte nullByte = static_cast<std::byte>(0);
    _storage.write(&nullByte, pageId);

    if (_cap == _config.pageCount) {
        PageId victimPageId = find_victim();
        int victimPagePosition = _positions.at(victimPageId);

        remove(victimPageId);
        insert(pageId, victimPagePosition);
    } else {
        insert(pageId, _cap);
        _cap++;
    }

    return pageId;
}

std::byte* PageCache::read_page(PageId pageId) {
    std::cerr << "Read here " << pageId.fileId.id << " " << pageId.id << "\n";
    if (pageId.id < 0) {
        return nullptr;
    }

    if (_positions.find(pageId) != _positions.end()) {
        inc(pageId);
        return _cache[_positions.at(pageId)];
    } else {
        std::byte* data;
        _storage.read(data, pageId);

        PageId victimPageId = find_victim();

        int victimCachePosition = _positions.at(victimPageId);
        
        remove(victimPageId);
        insert(pageId, victimCachePosition);
        _cache[victimCachePosition] = data;

        return data;
    }
}

int PageCache::write(PageId pageId) {
    if (pageId.id < 0) {
        return -2;
    } else if (_positions.find(pageId) != _positions.end()) {
        _data[pageId].dirty = true;
        return 0;
    }

    return -1;
}

void PageCache::insert(PageId pageId, int position) {
    _positions[pageId] = position;
    _position_to_id[position] = pageId;
    _data[pageId].dirty = false;
    _cache[position] = nullptr;
}

void PageCache::remove(PageId pageId) {
    if (_data.contains(pageId) && _data.at(pageId).dirty) {
        _storage.write(_cache[_positions.at(pageId)], pageId);
    }

    _positions.erase(pageId);
    _data[pageId].value = 0;
}

void PageCache::inc(PageId pageId) {
    if (_data.find(pageId) != _data.end()) {
        _data[pageId].value = std::min(_data.at(pageId).value + 1, _config.countLimit);
        return;
    }

    _data[pageId].value = 1;
}

int PageCache::sync() {
    for (auto &i : _data) {
        if (!i.second.dirty) {
            continue;
        }

        int pagePosition = _positions.at(i.first);
        _storage.write(_cache[pagePosition], i.first);
        i.second.dirty = false;
    }

    // Seems like 0 means ok, would be "void" here
    return 0;
}

PageId PageCache::find_victim() {
    for (int epoch = 0;;epoch++) {
        for (int i = _position_state; i < _config.pageCount; i++) {
            if (_position_to_id.find(i) == _position_to_id.end()) {
                continue;
            }

            PageId pageId = _position_to_id.at(i);
            int pageValue = (_data.find(pageId) != _data.end()) ? _data.at(pageId).value : 0;

            if (pageValue == 0) {
                _position_state = (i + 1) % _config.pageCount;
                return pageId;
            } else {
                _data[pageId].value = pageValue - 1;
            }
        }
        _position_state = 0;
    }
}