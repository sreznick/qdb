#include "pagecache.h"

PageId PageCache::create_page(FileId fileId) {
    PageId id = storage.create_page(fileId);
    
    if (id.id < 0) { 
        return PageId{{-1}, -1}; 
    }

    storage.write(0, id);
    if (localData.connection.size() == config.pageCount) {
        PageId to_del = find_victim();

        if (localData.store.contains(to_del) && localData.store.at(to_del).dirty) {
            storage.write(cdata[localData.store.at(to_del).position], to_del);
        }

        localData.data[to_del] = 0;
        localData.store[id].position = localData.store.at(to_del).position;
        localData.store[id].dirty = false;
        cdata[localData.store.at(to_del).position] = nullptr;
        localData.connection[localData.store.at(to_del).position] = id;
    } else {
        localData.store[id].position = localData.connection.size();
        localData.store[id].dirty = false;
        cdata[localData.connection.size()] = nullptr;
        localData.connection[localData.connection.size()] = id;
    }

    return id;
}

PageId PageCache::find_victim() {
    bool flag = true;
    int nominant = 0;
    PageId ret;

    while(flag) {
        for (int i = nominant; i < config.pageCount; i++) {
            if (localData.connection.find(i) != localData.connection.end()) {
                int tmp = 0;

                if (localData.store.find(localData.connection.at(i)) != localData.store.end()) 
                    tmp = localData.data.at(localData.connection.at(i));

                if (tmp == 0) {
                    nominant = (i + 1) % config.pageCount;
                    flag = false;
                    ret = localData.connection.at(i);
                }
                
                localData.data[localData.connection.at(i)] = tmp - 1;
            }
        }
        nominant = 0;
    }

    return ret;
}

std::byte* PageCache::read_page(PageId id) {
    if (id.id < 0) {
        return nullptr;
    }

    if (localData.store.find(id) == localData.store.end()) {
        std::byte* elem;
        storage.read(elem, id);

        PageId to_del = find_victim();

        if (localData.store.contains(to_del) && localData.store.at(to_del).dirty) {
            storage.write(cdata[localData.store.at(to_del).position], to_del);
        }

        localData.data[to_del] = 0;
        cdata[localData.store.at(to_del).position] = elem;
        localData.data[to_del] = 0;
        localData.store[id].position = localData.store.at(to_del).position;
        localData.store[id].dirty = false;
        cdata[localData.store.at(to_del).position] = nullptr;
        localData.connection[localData.store.at(to_del).position] = id;

        return elem;
    }

    if (localData.data.find(id) != localData.data.end()) {
        localData.data[id] = std::min(localData.data.at(id) + 1, config.countLimit);
    } else {
        localData.data[id] = std::min(1, config.countLimit);
    }

    return cdata[localData.store.at(id).position];
}

int PageCache::write(PageId id) {
    if (localData.store.find(id) != localData.store.end()) {
        localData.store[id].dirty = true;
    }

    return 0;
}

int PageCache::sync() {
    for (auto iterator = localData.store.begin(); iterator != localData.store.end(); iterator++) {
        if (localData.store[iterator->first].dirty) {
            storage.write(cdata[ localData.store.at(iterator->first).position], iterator->first);
            localData.store[iterator->first].dirty = false;
        }
    }

    return 0;
}

std::pair<std::map<PageId, State>*, std::map<PageId, int>*> PageCache::get_local_info() {
    return std::make_pair(&localData.store, &localData.data);
}
