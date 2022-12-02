#pragma once

#include "storage/storage.h"
#include "config.h"
#include <map>
#include <cstddef>
#include <vector>

struct PageData {
    bool dirty;
    int value; 
};

struct CMP {
    bool operator()(const PageId& a, const PageId& b) const {
        return a.id < b.id || (a.id == b.id && a.fileId.id < b.fileId.id);
    }
};

class PageCache {
private:

    int _cap;
    int _position_state;
    std::map<PageId, int, CMP> _positions;
    std::map<int, PageId> _position_to_id;
    std::map<PageId, PageData, CMP> _data;
    std::byte** _cache;

    Storage _storage;
    PageCacheConfig _config;

    // находит жертву для вытеснения
    // Это не часть публичного API, поэтому сигнатуру можно поменять, если удобно
    PageId find_victim();

    void remove(PageId pageId);
    void insert(PageId pageId, int position);
    void inc(PageId pageId);

public:
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
    std::byte* read_page(PageId id);

    /*
     * Определяет наличие страницы с данным PageId в кеше.
     * Если она там есть, помечает ее как dirty и возвращает 0
     * Если нет, возвращает -1
     * Если PageId некорректен, возвращает 2
     */
    int write(PageId id);

    /*
     * Пишут на диск все страницы, помеченные как грязные, снимает с них флаг
     */
    int sync();

    std::map<PageId, PageData, CMP>* _getData();
};
