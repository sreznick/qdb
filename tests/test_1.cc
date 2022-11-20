#include <gtest/gtest.h>
#include "pagecache.h"

TEST(Test_1, Test_1_1) {
    Storage storage("Testing/qdb");
    if (!storage.is_present()) {
        if (!storage.can_initialize()) {
            std::cerr << "Please specify one of" << std::endl;
            std::cerr << "  - path to existing storage (to use it)" << std::endl;
            std::cerr << "  - path to absolutely empty directory (to create storage there)" << std::endl;
            std::cerr << "  - non-existing path (to create storage there)" << std::endl;
            return;
        }
        storage.initialize();
    }

    PageCache pageCache(storage, PageCacheConfig(3, 4));
    PageId pages[4];
    for (int i = 0; i < 4; i++) {
        pages[i] = pageCache.create_page(FileId{i + 1});
        ASSERT_TRUE(pages[i].id >= 0 && pages[i].fileId.id == i + 1);
    }
    int cnts[] = {4, 3, 2, 1};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < cnts[i]; j++) {
            pageCache.read_page(pages[i]);
        }
    }
    PageId additionalPage = pageCache.create_page(FileId{5});
    auto mem = pageCache.read_page(additionalPage);
    *mem = static_cast<std::byte>(5);
    ASSERT_TRUE(pageCache.write(additionalPage) == 0);
    ASSERT_TRUE(pageCache.write(pages[3]) == -1);
    pageCache.sync();
    mem = pageCache.read_page(additionalPage);
    ASSERT_EQ(*mem, static_cast<std::byte>(5));
    for (auto page: pages) {
        mem = pageCache.read_page(page);
        ASSERT_EQ(*mem, static_cast<std::byte>(0));
    }
    PageId pages2[10];
    for (int i = 0; i < 10; i++) {
        pages2[i] = pageCache.create_page(FileId{i + 1 + 40});
    }
    for (auto i: pages2) {
        for (int j = 0; j < 10; j++) {
            pageCache.read_page(i);
        }
    }
    mem = pageCache.read_page(pages2[7]);
    *mem = static_cast<std::byte>(42);
    mem = pageCache.read_page(pages2[8]);
    *mem = static_cast<std::byte>(43);
    mem = pageCache.read_page(pages2[2]);
    *mem = static_cast<std::byte>(44);
    mem = pageCache.read_page(pages2[5]);
    *mem = static_cast<std::byte>(45);

    mem = pageCache.read_page(additionalPage);
    ASSERT_EQ(*mem, static_cast<std::byte>(5));
    pageCache.sync();
    mem = pageCache.read_page(additionalPage);
    ASSERT_EQ(*mem, static_cast<std::byte>(5));
    mem = pageCache.read_page(pages2[8]);
    ASSERT_EQ(*mem, static_cast<std::byte>(43));
}
