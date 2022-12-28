#pragma once

#include "storage/storage.h"
#include "config.h"
// #include "common/common.h"

#include <cstddef>
#include <map>
#include <vector>

struct pageIdComparator {
    bool operator()(const PageId& p1, const PageId& p2) const {
        return p1.id < p2.id || (p1.id == p2.id && p1.fileId.id < p2.fileId.id);
    }
};

class PageCache {
private:
    Storage _storage;
    PageCacheConfig _config;

    int capacity;
    int lastVictimPosition;
    std::map<PageId, int, pageIdComparator> _cachePosition;
    std::map<int, PageId> _pageIdByPosition;
    std::map<PageId, bool, pageIdComparator> _isDirty;
    std::map<PageId, int, pageIdComparator> _values;
    std::byte** _cache;

    // находит жертву для вытеснения
    // Это не часть публичного API, поэтому сигнатуру можно поменять, если удобно
    PageId find_victim();

    // Increments clock counter for Page with specified ID
    // with respect to _config.maxCount value.
    void incrementCounter(PageId pageId);

    // Removes page with specified ID from internal data structures.
    void removePage(PageId pageId);

    // Inserts page with specified ID to internal data structures.
    void insertPage(PageId pageId, int position);

public:
    PageCache(Storage storage, PageCacheConfig config): _storage(storage), _config(config) {
        capacity = 0;
        lastVictimPosition = 0;
        _cache = new std::byte*[_config.pageCount];
    }

    PageCacheConfig getConfig() {
        return _config;
    }

    /*
      Создает новую страницу в хранилище и размещает ее в кеше
      Точнее, выделяет страницу в кеше и заполняет ее нулями
      При успехе возвращает PageId, при неудаче в работе с хранилищем - -1 в поле id
     */

    PageId create_page(FileId id);

    /*
     * Определяет наличие страницы с данным PageId в кеше.
     * Если она там есть, возвращает указатель на нее
     * Если нет, загружает ее из файла
     * Если PageId некорректен, возвращает 0
     */
    std::byte* read_page(PageId id);

    /*
     * Определяет наличие страницы с данным PageId в кеше.
     * Если она там есть, помечает ее как dirty и возвращает 0
     * Если нет, возвращает -1
     * Если PageId некорректен, возвращает -2
     */
    int write(PageId id, std::byte*);

    /*
     * Пишут на диск все страницы, помеченные как грязные, снимает с них флаг
     */
    int sync();

    // NOTE: only for test purposes
    // encapsulation is good but we don't have any other way
    // to verify algorithm correctness but check internal structures.
    std::map<PageId, int, pageIdComparator>* _getValues();
    std::map<PageId, bool, pageIdComparator>* _getIsDirty();

    void openFile(FileId fileId);
};
