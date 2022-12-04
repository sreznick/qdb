#include "pagecache.h"

PageId PageCache::find_victim() {
    bool flag = true;
    PageId ret;

    while(flag) {
        for (int i = nominant; i < config.pageCount; i++) {
            if (connection.find(i) != connection.end()) {
                int pageValue = 0;

                if (store.find(connection.at(i)) != store.end()) 
                    pageValue = data.at(connection.at(i));

                if (pageValue == 0) {
                    nominant = (i + 1) % config.pageCount;
                    flag = false;
                    ret = connection.at(i);
                }
                
                data[connection.at(i)] = pageValue - 1;
            }
        }
        nominant = 0;
    }

    return ret;
}

void PageCache::add_page(PageId id, int position) {
    store[id].position = position;
    store[id].dirty = false;
    cdata[position] = nullptr;
    connection[position] = id;
}

PageId PageCache::create_page(FileId fileId) {
    PageId id = storage.create_page(fileId);
    
    if (id.id < 0) { 
        return PageId{0, -1}; 
    }

    storage.write(0, id);
    if (capacity == config.pageCount) {
        PageId to_del = find_victim();

        if (store.contains(to_del) && store.at(to_del).dirty) {
            storage.write(cdata[store.at(to_del).position], to_del);
        }

        data[to_del] = 0;
        add_page(id, store.at(to_del).position);
    } else {
        add_page(id, capacity);
        capacity++;
    }

    return id;
}

std::byte* PageCache::read_page(PageId id) {
    if (id.id < 0) {
        return nullptr;
    }

    if (store.find(id) == store.end()) {
        std::byte* elem;
        storage.read(elem, id);

        PageId to_del = find_victim();

        if (store.contains(to_del) && store.at(to_del).dirty) {
            storage.write(cdata[store.at(to_del).position], to_del);
        }

        data[to_del] = 0;
        cdata[store.at(to_del).position] = elem;
        add_page(id, store.at(to_del).position);

        return elem;
    }

    if (data.find(id) != data.end()) {
        data[id] = std::min(data.at(id) + 1, config.countLimit);
    } else {
        data[id] = std::min(1, config.countLimit);
    }

    return cdata[store.at(id).position];
}

int PageCache::write(PageId id) {
    if (id.id < 0) {
         return -2; 
         }

    if (store.find(id) != store.end()) {
        store[id].dirty = true;

        return 0;
    } else {
        return -1;
    }
}

int PageCache::sync() {
    for (auto iterator = store.begin(); iterator != store.end(); iterator++) {
        if (store[iterator->first].dirty) {
            storage.write(cdata[ store.at(iterator->first).position], iterator->first);
            store[iterator->first].dirty = false;
        }
    }

    return 0;
}

std::pair<std::map<PageId, State>*, std::map<PageId, int>*> PageCache::get_local_info() {
    return std::make_pair(&store, &data);
}
