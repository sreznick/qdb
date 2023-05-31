#include "pagecache.h"
#include "pagecache_test.h"

#include <cstring>

int PageCache::find_victim() {
    while (_clock[_clock_position] > 0 || _clock[_clock_position] == 0 && _used < _config.pageCount) {
        _clock[_clock_position] = std::max(0, _clock[_clock_position] - 1);
        if (++_clock_position == _config.pageCount)
            _clock_position = 0;
    }
    return _clock_position;
}

int PageCache::kill_victim(int i) {
    if (_modified[i] && _storage.write(_data[i], _page_ids[i]) < 0)
        return -1;

    _used -= 1;
    _clock[i] = -1;
    _modified[i] = false;
    _reverse_page_ids.erase(_page_ids[i]);
    delete[] _data[i];
    return 0;
}

void PageCache::insert(int i, PageId id, std::byte* new_data) {
    _used += 1;
    _clock[i] = 1;
    _modified[i] = false;
    _page_ids[i] = id;
    _reverse_page_ids[id] = i;
    _data[i] = new_data;
}

PageCache::PageCache(Storage storage, PageCacheConfig config): _storage(storage), _config(config) {
    _clock = std::vector(_config.pageCount, -1);
    _modified = std::vector(_config.pageCount, false);
    _page_ids = std::vector(_config.pageCount, PageId{ -1, -1 });
    _data = std::vector(_config.pageCount, (std::byte*) nullptr);
}

PageCache::~PageCache() {
    for (int i = 0; i < _config.pageCount; ++i) {
        if (_clock[i] == -1)
            continue;
        delete[] _data[i];
    }
}

PageId PageCache::create_page(FileId id) {
    PageId page_id = _storage.create_page(id);
    if (page_id.id == -1)
        return page_id;

    int page_size = _storage.page_size();
    std::byte null_data[page_size] = { std::byte{ 0 } };
    std::byte* new_data = new std::byte[page_size];
    memcpy(new_data, null_data, page_size);

    if (_storage.write(new_data, page_id) < 0) {
        delete[] new_data;
        page_id.id = -1;
        return page_id;
    }

    int i = find_victim();
    if (_clock[i] != -1 && kill_victim(i) < 0) {
        delete[] new_data;
        page_id.id = -1;
        return page_id;
    }
    insert(i, page_id, new_data);

    return page_id;
}

std::byte* PageCache::read_page(PageId id) {
    if (id.id < 0)
        return nullptr;

    auto it = _reverse_page_ids.find(id);
    if (it != _reverse_page_ids.end()) {
        int i = it->second;
        _clock[i] = std::min(_config.countLimit, _clock[i] + 1);
        return _data[i];
    }

    std::byte* new_data = new std::byte[_storage.page_size()];
    if (_storage.read(new_data, id) < 0)
        return nullptr;

    int i = find_victim();
    if (_clock[i] != -1 && kill_victim(i) < 0) {
        delete[] new_data;
        return nullptr;
    }
    insert(i, id, new_data);

    return new_data;
}

int PageCache::write(PageId id) {
    if (id.id < 0)
        return -2;

    auto it = _reverse_page_ids.find(id);
    if (it == _reverse_page_ids.end())
        return -1;

    int i = it->second;
    _modified[i] = true;
    _clock[i] = std::min(_config.countLimit, _clock[i] + 1);
    return 0;
}

int PageCache::sync() {
    for (int i = 0; i < _config.pageCount; ++i) {
        if (!_modified[i])
            continue;
        _modified[i] = false;

        if (_storage.write(_data[i], _page_ids[i]) < 0)
            return -1;
    }
    return 0;
}
