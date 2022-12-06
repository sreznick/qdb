#include "pagecache.h"

PageCache::PageCache(Storage storage, PageCacheConfig config): _storage(storage), _config(config) {
    _cap = 0;
    _position_state = 0;
    _cache = new std::byte*[_config.pageCount];
}

PageCache::~PageCache() {}

PageId PageCache::create_page(FileId fileId) {
    PageId pageId = _storage.create_page(fileId);
    if (pageId.id < 0) {
        return PageId{0, -1};
    }

    std::byte nullData = static_cast<std::byte>(0);
    _storage.write(&nullData, pageId);   
    std::cerr << "PC: " << _config.pageCount << "\n";

    if (_cap >= _config.pageCount) {
        PageId victim = find_victim();
        int victimPosition = _positions.at(victim);

        remove(victim);
        insert(pageId, victimPosition);
    } else {
        insert(pageId, _cap);
        _cap++;
    }   

    return pageId;
}

std::byte* PageCache::read_page(PageId pageId) {
    if (pageId.id < 0) {
        return nullptr;
    }

    if (_positions.find(pageId) != _positions.end()) {
        inc(pageId);
        return _cache[_positions.at(pageId)];
    } else {
        std::byte* data;
        _storage.read(data, pageId);

        PageId victim = find_victim();
        int victimPosition = _positions.at(victim);
        
        remove(victim);
        insert(pageId, victimPosition);
        _cache[victimPosition] = data;

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
    for (;;) {
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


// My custom test implementation :)
// To run this, pass "t" parameter to ./qdb run

void PageCache::self_test() {

    PageId ids[5];

    for (int i = 0; i < 4; i++) {
        ids[i] = create_page(FileId{1});
        assert(ids[i].id >= 0 && ids[i].fileId.id == 1);
        
        for (int j = 0; j < (i % 2) + 2; j++) {
            read_page(ids[i]);
        }
    }

    assert(_data.size() == 4);
    assert(_data.at(ids[0]).value == 2);
    assert(_data.at(ids[1]).value == 3);
    assert(_data.at(ids[2]).value == 2);
    assert(_data.at(ids[3]).value == 3);

    ids[4] = create_page(FileId{5});
    assert(ids[4].id >= 0 && ids[4].fileId.id == 5);

    assert(_data.at(ids[0]).value == 0);
    assert(_data.at(ids[1]).value == 1);
    assert(_data.at(ids[2]).value == 0);
    assert(_data.at(ids[3]).value == 1);


    // Test after write
    write(ids[0]);
    write(ids[1]);

    assert(!_data.at(ids[0]).dirty);
    assert(_data.at(ids[1]).dirty);
    assert(!_data.at(ids[2]).dirty);
    assert(!_data.at(ids[3]).dirty);
    assert(!_data.at(ids[4]).dirty);

    // Test after sync no dirty pages
    sync();
    for (int i = 0; i < 5; i++) {
        assert(!_data.at(ids[i]).dirty);
    }
}