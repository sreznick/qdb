#include <gtest/gtest.h>
#include "pagecache.h"

TEST(Test_1, Test_1_1) {

    Storage storage("testing/qdb");
    PageCache implementation {storage, {5, 16384}};
    PageId pageId_0 = implementation.create_page(FileId{1});
    ASSERT_TRUE(pageId_0.id >= 0 && pageId_0.fileId.id == 1);

    PageId pageId_1 = implementation.create_page(FileId{2});
    ASSERT_TRUE(pageId_1.id >= 0 && pageId_1.fileId.id == 2);

    PageId pageId_2 = implementation.create_page(FileId{3});
    ASSERT_TRUE(pageId_2.id >= 0 && pageId_2.fileId.id == 3);

    PageId pageId_3 = implementation.create_page(FileId{4});
    ASSERT_TRUE(pageId_3.id >= 0 && pageId_3.fileId.id == 4);

    implementation.read_page(pageId_0);
    implementation.read_page(pageId_0);
    implementation.read_page(pageId_0);
    implementation.read_page(pageId_0);

    implementation.read_page(pageId_1);
    implementation.read_page(pageId_1);
    implementation.read_page(pageId_1);

    implementation.read_page(pageId_2);
    implementation.read_page(pageId_2);

    implementation.read_page(pageId_3);

    std::map<PageId, PageData, CMP>* data = implementation._getData();
    ASSERT_TRUE(data->size() == 4);
    ASSERT_TRUE(data->at(pageId_0).value == 3);
    ASSERT_TRUE(data->at(pageId_1).value == 3);
    ASSERT_TRUE(data->at(pageId_2).value == 2);
    ASSERT_TRUE(data->at(pageId_3).value == 1);

    // create another page that does not fit in cache buffer
    PageId pageId_4 = implementation.create_page(FileId{5});

    ASSERT_TRUE(data->size() == 4);
    ASSERT_TRUE(data->at(pageId_0).value == 1);
    ASSERT_TRUE(data->at(pageId_1).value == 1);
    ASSERT_TRUE(data->at(pageId_2).value == 0);
    ASSERT_TRUE(data->at(pageId_3).value == 0);

    implementation.write(pageId_0);
    implementation.write(pageId_1);
    ASSERT_TRUE(data->at(pageId_0).dirty);
    ASSERT_TRUE(data->at(pageId_1).dirty);
    ASSERT_TRUE(!data->at(pageId_2).dirty);
    ASSERT_TRUE(!data->at(pageId_3).dirty);
    ASSERT_TRUE(!data->at(pageId_4).dirty);

    implementation.sync();
    ASSERT_TRUE(!data->at(pageId_0).dirty);
    ASSERT_TRUE(!data->at(pageId_1).dirty);
    ASSERT_TRUE(!data->at(pageId_2).dirty);
    ASSERT_TRUE(!data->at(pageId_3).dirty);
    ASSERT_TRUE(!data->at(pageId_4).dirty);

    std::cout << "OK" << std::endl;
}