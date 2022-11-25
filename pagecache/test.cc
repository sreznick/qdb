#include "test.h"
#include "cassert"

void PageCacheTest::run() {
    // create 4 pages (they fit cache buffer)
    PageId pageId_0 = implementation.create_page(FileId{1});
    assert(pageId_0.id >= 0 && pageId_0.fileId.id == 1);

    PageId pageId_1 = implementation.create_page(FileId{2});
    assert(pageId_1.id >= 0 && pageId_1.fileId.id == 2);

    PageId pageId_2 = implementation.create_page(FileId{3});
    assert(pageId_2.id >= 0 && pageId_2.fileId.id == 3);

    PageId pageId_3 = implementation.create_page(FileId{4});
    assert(pageId_3.id >= 0 && pageId_3.fileId.id == 4);

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

    std::map<PageId, int, pageIdComparator>* values = implementation._getValues();
    assert(values->size() == 4);
    assert(values->at(pageId_0) == 3);
    assert(values->at(pageId_1) == 3);
    assert(values->at(pageId_2) == 2);
    assert(values->at(pageId_3) == 1);

    // create another page that does not fit in cache buffer
    PageId pageId_4 = implementation.create_page(FileId{5});

    assert(values->size() == 4);
    std::cout << values->at(pageId_0) << std::endl;
    assert(values->at(pageId_0) == 1);
    assert(values->at(pageId_1) == 1);
    assert(values->at(pageId_2) == 0);
    assert(values->at(pageId_3) == 0);

    std::map<PageId, bool, pageIdComparator>* isDirty = implementation._getIsDirty();
    implementation.write(pageId_0);
    implementation.write(pageId_1);
    assert(isDirty->at(pageId_0));
    assert(isDirty->at(pageId_1));
    assert(!isDirty->at(pageId_2));
    assert(!isDirty->at(pageId_3));
    assert(!isDirty->at(pageId_4));

    implementation.sync();
    assert(!isDirty->at(pageId_0));
    assert(!isDirty->at(pageId_1));
    assert(!isDirty->at(pageId_2));
    assert(!isDirty->at(pageId_3));
    assert(!isDirty->at(pageId_4));

    std::cout << "OK" << std::endl;
}