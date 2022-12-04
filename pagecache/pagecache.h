#pragma once

#include "storage/storage.h"
#include "config.h"

#include <cstddef>
#include <map>
#include <vector>

struct State {
        int position;
        bool dirty;
    };

class PageCache {
private:

    Storage storage;
    PageCacheConfig config;

    std::map<PageId, int> data;
    std::map<int, PageId> connection;
    std::map<PageId, State> store;

    int capacity;
    int nominant;
    std::byte** cdata;

    // находит жертву для вытеснения
    // Это не часть публичного API, поэтому сигнатуру можно поменять, если удобно
    PageId find_victim();

    void add_page(PageId id, int position);

public:
    PageCache(Storage storage, PageCacheConfig config): storage(storage), config(config), capacity(0), nominant(0), cdata(new std::byte*[config.pageCount]) {}
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
    int write(PageId id);

    /*
     * Пишут на диск все страницы, помеченные как грязные, снимает с них флаг
     */
    int sync();

    std::pair<std::map<PageId, State>*, std::map<PageId, int>*> get_local_info();
};
