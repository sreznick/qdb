#include "storage/storage.h"
#include "pagecache.h"

class PageCacheTest {
private:
    PageCache implementation;
public:
    PageCacheTest(PageCache implementation) : implementation(implementation) {}

    void run();
};