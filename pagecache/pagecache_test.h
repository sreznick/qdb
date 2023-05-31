#include <cassert>

void PageCache::test() {
    assert(_config.pageCount == 3);

    std::vector<PageId> pages;
    PageId page;
    std::byte* random_data = new std::byte[_storage.page_size()];
    std::byte* random_data_2 = new std::byte[_storage.page_size()];
    std::byte* random_data_3 = new std::byte[_storage.page_size()];

    assert(_used == 0 && find_victim() == 0);
    page = create_page(FileId{1});
    pages.push_back(page);
    assert(_used == 1 && _clock[0] != -1 && pages[0] == _page_ids[0]);

    kill_victim(0);
    assert(_used == 0 && _clock[0] == -1);
    insert(0, pages[0], random_data);
    assert(_used == 1 && _clock[0] != -1 && pages[0] == _page_ids[0]);
    insert(2, pages[0], random_data_2);
    assert(_used == 2 && _clock[2] != -1 && pages[0] == _page_ids[2]);

    assert(find_victim() == 1);
    pages.push_back(create_page(FileId{1}));
    assert(_used == 3 && _clock[1] != -1 && pages[1] == _page_ids[1]);
    assert(pages[0] != pages[1]);

    kill_victim(2);
    assert(_used == 2 && _clock[2] == -1);
    assert(find_victim() == 2);

    kill_victim(0);
    insert(0, pages[0], random_data_3);
    pages.push_back(create_page(FileId{1}));
    assert(_used == 3 && _clock[2] != -1 && pages[2] == _page_ids[2]);

    assert(pages[0] == _page_ids[0] && pages[1] == _page_ids[1] && pages[2] == _page_ids[2]);

    assert(read_page(pages[0]) == random_data_3);
    page.id = -1;
    assert(read_page(page) == nullptr);
    assert(_used == _config.pageCount);

    find_victim();
    read_page(pages[0]);
    find_victim();
    read_page(pages[2]);
    assert(find_victim() == 1);

    pages.push_back(create_page(FileId{1}));
    assert(pages[3] == _page_ids[1]);

    find_victim();
    read_page(pages[0]);
    find_victim();
    read_page(pages[2]);
    assert(find_victim() == 1);

    read_page(pages[1]);
    assert(pages[1] == _page_ids[1]);

    assert(write(page) == -2);
    assert(write(pages[3]) == -1);
    assert(write(pages[2]) == 0);
    assert(write(pages[1]) == 0);
    assert(!_modified[0] && _modified[1] && _modified[2]);

    kill_victim(2);
    assert(!_modified[0] && _modified[1] && !_modified[2]);
    sync();
    assert(!_modified[0] && !_modified[1] && !_modified[2]);
}
