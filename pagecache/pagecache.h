#pragma once

#include "storage/storage.h"
#include "config.h"

#include <vector>
#include <map>

class PageCache {
private:
    Storage _storage;
    PageCacheConfig _config;

    int _used = 0;
    int _clock_position = 0;
    std::vector<int> _clock; // -1 - empty, 0 - ready for deletion, 1 - not ready for deletion;

    std::vector<bool> _modified;
    std::vector<PageId> _page_ids;
    std::map<PageId, int> _reverse_page_ids;

    std::vector<std::byte*> _data;

    // находит жертву для вытеснения
    // Это не часть публичного API, поэтому сигнатуру можно поменять, если удобно
    int find_victim();
    int kill_victim(int i);
    void insert(int i, PageId id, std::byte* new_data);

public:
    PageCache(Storage storage, PageCacheConfig config);
    ~PageCache();

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

    int page_size();

    void test();
};

