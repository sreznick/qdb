#include "btree.h"

Record BTree::find(int value) {
    auto rootNode = BTreeNode(_rootPage, _rootPageId);
    int r = rootNode.records_count() - 1, l = 1, mid = -1;

    while (l <= r) {
        mid = l + (r - l) / 2;
        auto midNode = rootNode.at(mid);
        if (midNode.value == value) { return midNode; }
        else if (midNode.value < value) { l = mid + 1; }
        else { r = mid - 1; }
    }

    return {-1, -1};
}

void BTree::insert_fake_record() {
    if (_recordsCount > 0) return;

    insert(Record{FAKE_RECORD_VALUE, -1});
}

void BTree::persist(BTreeNode* node) {
    _pageCachePtr.get()->write(PageId{{_fileId}, node->page_id()}, node->page());
}

int BTree::insert(Record record) {
    int depth = 0;
    std::byte* parent = nullptr;
    std::byte* page = _rootPage;
    int nodePageId = _rootPageId;
    BTreeNode* node = new BTreeNode(page, nodePageId);
//
//    while (depth < _depth || node->is_full()) {
//        node = new BTreeNode(page, _rootPageId);
//        if (node->is_full()) {
//            if (parent == nullptr) {
//                auto parentPageId = _create_page();
//                auto parentPage = _read_new_page(parentPageId);
//                auto parentNode = new BTreeNode(parentPage, parentPageId.id);
//
//                auto siblingPageId = _create_page();
//                auto siblingPage = _read_new_page(siblingPageId);
//                auto siblingNode = new BTreeNode(siblingPage, siblingPageId.id);
//
//                node->reassign_records(node, parentNode, siblingNode);
//                persist(parentNode);
//                persist(siblingNode);
//                persist(node);
//
//                _depth += 1;
//                depth += 1;
//                update_meta_page();
//            } else {
//                // recursion?
//                // node->reassign_records(node, parent);
//            }
//        } else {
//            for (int i = node->records_count(); i >= 0; i--) {
//                if (node->at(i).value < record.value) {
//                    depth += 1;
//                    nodePageId = node->at(i).pointer;
//                    page = _read_new_page(PageId{{_fileId}, nodePageId});
//                    std::cout << "new page ID: " << nodePageId << std::endl;
//                }
//            }
//        }
//    }

    node->insert(record);
}

void BTreeNode::reassign_records(BTreeNode* node, BTreeNode* parent, BTreeNode* sibling) {
    Record middle = at(_recordsCount / 2);

    std::cout << "middle: " << middle.value << std::endl;
    middle.pointer = node->page_id();
    parent->insert(middle);

    for (int i = 1; i < _recordsCount / 2; i++) {
        sibling->insert(at(i));
    }

    memcpy(_page + META_SIZE, _page + META_SIZE + RECORD_SIZE * (_recordsCount / 2), RECORD_SIZE * (_recordsCount / 2));
    node->_recordsCount = (_recordsCount / 2);
}

void BTreeNode::insert(Record record) {
    _assert_not_full();

    bool done = false;

    auto value = record.value;
    auto pointer = record.pointer;

    if (_recordsCount == 0) {
        memcpy(_page + META_SIZE, &value, 4);
        memcpy(_page + META_SIZE + 4, &pointer, 4);
    } else {
        int offset = 0, key = 0;
        for (int i = _recordsCount; i > 0; i--) {
            offset = META_SIZE + (i - 1) * RECORD_SIZE;
            memcpy(&key, _page + offset, 4);
            if (key > value) {
                memcpy(_page + offset + RECORD_SIZE, _page + offset, 8);
            } else {
                memcpy(_page + offset + RECORD_SIZE, &value, 4);
                memcpy(_page + offset + RECORD_SIZE + 4, &pointer, 4);
                done = true;
                break;
            }
        }

        if (!done) {
            memcpy(_page + META_SIZE, &value, 4);
            memcpy(_page + META_SIZE + 4, &pointer, 4);
        }
    }

    _recordsCount += 1;
    memcpy(_page, &_recordsCount, 4);
}

Record BTreeNode::at(int index) {
    if (index <= 0 || index > _recordsCount) {
        std::cout << "Error: invalid index" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::int32_t value, pointer;
    memcpy(&value, _page + META_SIZE + index * RECORD_SIZE, 4);
    memcpy(&pointer, _page + META_SIZE + index * RECORD_SIZE + 4, 4);

    return Record{value, pointer};
}

void BTreeNode::_init_state() {
    std::memcpy(&_recordsCount, _page, 4);
}

void BTreeNode::_assert_not_full() {
    if (!is_full()) return;

    std::cout << "ERROR: BTreeNode invariant violation: node is already full" << std::endl;
    exit(EXIT_FAILURE);
}

bool BTreeNode::is_full() {
    return _recordsCount == MAX_RECORDS_COUNT;
}
