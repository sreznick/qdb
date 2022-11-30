#pragma once

#include "storage/storage.h"
#include "config.h"
#include <set>
#include <vector>

class PageCache {
private:
    Storage storage;
    PageCacheConfig config;
    int next_free_slot;
    int last_checked_position;
    std::vector<int> access_counters;
    std::set<PageId> dirty;
    std::map<PageId, int> page2index;
    std::map<int, PageId> index2page;
    std::byte** cache;

    // находит жертву для вытеснения
    // Это не часть публичного API, поэтому сигнатуру можно поменять, если удобно
    PageId find_victim();

    void remove_page(PageId pageId);

    void insert_page_into(PageId pageId, int position);

public:
    PageCache(Storage storage, PageCacheConfig config);
    ~PageCache();

    /*
      Создает новую страницу в хранилище и размещает ее в кеше
      Точнее, выделяет страницу в кеше и заполняет ее нулями

      При успехе возвращает PageId, при неудаче в работе с хранилищем - -1 в поле id 
     */
    PageId create_page(FileId fileId);

    /*
     * Определяет наличие страницы с данным PageId в кеше.
     * Если она там есть, возвращает указатель на нее
     * Если нет, загружает ее из файла
     * Если PageId некорректен, возвращает 0
     */  
    std::byte* read_page(PageId pageId);

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

