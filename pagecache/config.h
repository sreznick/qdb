#pragma once

struct PageCacheConfig {
    PageCacheConfig(int countLimit, int pageCount) : countLimit(countLimit), pageCount(pageCount) {
    }

    int countLimit;
    int pageCount;
};

