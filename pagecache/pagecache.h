#pragma once

#include "storage/storage.h"
#include "config.h"
#include <set>
#include <vector>

class PageCache {
private:
    Storage _storage;
    PageCacheConfig _config;
    int _initializerPosition;
    int _lastPosition;
    std::byte **_cache;
    std::set<PageId> _dirty;
    std::vector<int> _counter;
    std::map<PageId, int> _page2position;
    std::map<int, PageId> _position2page;

    // находит жертву для вытеснения
    // Это не часть публичного API, поэтому сигнатуру можно поменять, если удобно
    PageId find_victim();

    void remove_page(PageId pageId);

    void insert_page_into(PageId pageId, int position);

public:
    ~PageCache();

    PageCache(Storage storage, PageCacheConfig config);

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
    std::byte *read_page(PageId id);

    /*
     * Определяет наличие страницы с данным PageId в кеше.
     * Если она там есть, помечает ее как dirty и возвращает 0
     * Если нет, возвращает -1
     * Если PageId некорректен, возвращает -2
     */
    int write(PageId pageId);

    /*
     * Пишут на диск все страницы, помеченные как грязные, снимает с них флаг
     */
    int sync();
};

