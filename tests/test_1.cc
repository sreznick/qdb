#include <gtest/gtest.h>
#include "pagecache.h"

using namespace std;

TEST(Test_1, Test_1_1) {

    Storage storage("");
    PageCache test_store {storage, {3, 4}};
    PageId pageId_0 = test_store.create_page(FileId{1});
    ASSERT_TRUE(pageId_0.id >= 0 && pageId_0.fileId.id == 1);

    PageId pageId_1 = test_store.create_page(FileId{2});
    ASSERT_TRUE(pageId_1.id >= 0 && pageId_1.fileId.id == 2);

    PageId pageId_2 = test_store.create_page(FileId{3});
    ASSERT_TRUE(pageId_2.id >= 0 && pageId_2.fileId.id == 3);

    PageId pageId_3 = test_store.create_page(FileId{4});
    ASSERT_TRUE(pageId_3.id >= 0 && pageId_3.fileId.id == 4);

    test_store.read_page(pageId_0);
    test_store.read_page(pageId_0);
    test_store.read_page(pageId_0);
    test_store.read_page(pageId_0);
    test_store.read_page(pageId_1);
    test_store.read_page(pageId_1);
    test_store.read_page(pageId_1);
    test_store.read_page(pageId_2);
    test_store.read_page(pageId_2);
    test_store.read_page(pageId_3);

    auto info = test_store.get_local_info();
    
    ASSERT_TRUE(info.second->size() == 4);
    ASSERT_TRUE(info.second->at(pageId_0) == 3);
    ASSERT_TRUE(info.second->at(pageId_1) == 3);
    ASSERT_TRUE(info.second->at(pageId_2) == 2);
    ASSERT_TRUE(info.second->at(pageId_3) == 1);

    PageId pageId_4 = test_store.create_page(FileId{5});
    ASSERT_TRUE(info.second->size() == 4);
    ASSERT_TRUE(info.second->at(pageId_0) == 1);
    ASSERT_TRUE(info.second->at(pageId_1) == 1);
    ASSERT_TRUE(info.second->at(pageId_2) == 0);
    ASSERT_TRUE(info.second->at(pageId_3) == 0);

    test_store.write(pageId_0);
    test_store.write(pageId_1);
    ASSERT_TRUE(info.first->at(pageId_0).dirty);
    ASSERT_TRUE(info.first->at(pageId_1).dirty);
    ASSERT_TRUE(!info.first->at(pageId_2).dirty);
    ASSERT_TRUE(!info.first->at(pageId_3).dirty);
    ASSERT_TRUE(!info.first->at(pageId_4).dirty);

    test_store.sync();
    ASSERT_TRUE(!info.first->at(pageId_0).dirty);
    ASSERT_TRUE(!info.first->at(pageId_1).dirty);
    ASSERT_TRUE(!info.first->at(pageId_2).dirty);
    ASSERT_TRUE(!info.first->at(pageId_3).dirty);
    ASSERT_TRUE(!info.first->at(pageId_4).dirty);
}