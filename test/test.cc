#include <gtest/gtest.h>
#include "pagecache.h"

TEST(TEST_1, TEST_1_1) {
    Storage storage("test");
    if (!storage.is_present()) {
        storage.initialize();
    }
    PageCache pageCache(storage, PageCacheConfig {3, 3});
    PageId pages[3];
    for (int i = 0; i < 3; i++) {
        pages[i] = pageCache.create_page(FileId { i });
    }
    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(pages[i].id >= 0);
    }
    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(pages[i].fileId.id == i);
    }
    for (int i = 0; i < 3; i++) {
        auto old_page = pages[i];
        pages[i] = pageCache.create_page(FileId { 3 + i });
        ASSERT_TRUE(pageCache.write(old_page) == -1);
    }
    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(pages[i].id >= 0);
    }
    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(pages[i].fileId.id == (3 + i));
    }
    {
        pageCache.read_page(pages[0]);
        auto old_page = pages[1];
        auto page = pageCache.create_page(FileId { 6 });
        ASSERT_TRUE(pageCache.write(old_page) == -1);
    }
    {
        auto page_space = pageCache.read_page(pages[0]);
        *page_space = static_cast<std::byte>(2);
        ASSERT_TRUE(pageCache.write(pages[0]) == 0);
        ASSERT_EQ(*pageCache.read_page(pages[0]), static_cast<std::byte>(2));
        pageCache.sync();
        ASSERT_EQ(*pageCache.read_page(pages[0]), static_cast<std::byte>(2));
    }
}