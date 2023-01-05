#include <stdio.h>
#include <cstdint>
#include <cstddef>

#ifndef QDB_INDEX_H
#define QDB_INDEX_H

int const INDEX_PAGE_META_SIZE = 20;

struct IndexPageMeta {
    std::int32_t pointerLeft;
    std::int32_t pointerRight;
    std::int32_t tuplesCount;
};

#endif //QDB_INDEX_H
