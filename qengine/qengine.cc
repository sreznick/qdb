#include "qengine.h"

void write_page_header(std::byte* page, PageMeta* pageMeta, TableMeta* tableMeta) { // TODO: check.
    std::int32_t accumulator = 0;

    bool flag = tableMeta != nullptr;
    pageMeta->hasTableMeta = (std::int32_t) flag;
    memcpy(page + accumulator, &pageMeta->hasTableMeta, sizeof pageMeta->hasTableMeta);
    accumulator += sizeof pageMeta->hasTableMeta;

    memcpy(page + accumulator, &pageMeta->dataPtr, sizeof pageMeta->dataPtr);
    accumulator += sizeof pageMeta->dataPtr;

    if (!flag) return;

    memcpy(page + accumulator, &tableMeta->fileId, sizeof tableMeta->fileId);
    accumulator += sizeof tableMeta->fileId;

    memcpy(page + accumulator, &tableMeta->lastPageId, sizeof tableMeta->lastPageId);
    accumulator += sizeof tableMeta->lastPageId;

    memcpy(page + accumulator, &tableMeta->tupleNum, sizeof tableMeta->tupleNum);
    accumulator += sizeof tableMeta->tupleNum;
}

int get_header_length(std::byte* page) {
    std::int32_t hasTableMeta;
    memcpy(&hasTableMeta, page, sizeof PageMeta::hasTableMeta);

    int headerLength = (sizeof PageMeta::hasTableMeta) +
            hasTableMeta * ((sizeof TableMeta::fileId) + (sizeof TableMeta::lastPageId));
    return headerLength; // TODO: check.
}

int get_data_ptr(std::byte* page) {
    std::int32_t dataPtr;
    memcpy(&dataPtr, page + (sizeof PageMeta::hasTableMeta), sizeof dataPtr); // TODO: check.
    return dataPtr;
}

void set_data_ptr(std::byte* page, std::int32_t dataPtr) {
    memcpy(page + (sizeof PageMeta::hasTableMeta), &dataPtr, sizeof dataPtr); // TODO: check.
}

PageId newPage(std::shared_ptr<PageCache> pageCache, FileId fileId) {
    auto pageId = pageCache->create_page(fileId);

    PageMeta page_meta{ 0, pageCache->page_size() };

    std::byte* page = pageCache->read_page(pageId);
    write_page_header(page, &page_meta, nullptr);

    return pageId;
}

void set_last_page(std::shared_ptr<PageCache> pageCache, PageId newPageId) {
    PageId firstPageId { newPageId.fileId, 0 };
    std::byte* page = pageCache->read_page(firstPageId);

    std::int32_t offset = (sizeof PageMeta::hasTableMeta) + (sizeof PageMeta::dataPtr) + (sizeof TableMeta::fileId);
    memcpy(page + offset, &newPageId.id, sizeof newPageId.id); // TODO: check.

    if (pageCache->write(firstPageId) < 0) {
        std::cout << "FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::int32_t get_last_page(std::shared_ptr<PageCache> pageCache, FileId fileId) {
    PageId firstPageId { fileId, 0 };
    std::byte* page = pageCache->read_page(firstPageId);

    std::int32_t lastPage;
    std::int32_t offset = (sizeof PageMeta::hasTableMeta) + (sizeof PageMeta::dataPtr) + (sizeof TableMeta::fileId);
    memcpy(&lastPage, page + offset, sizeof lastPage); // TODO: check.

    return lastPage;
}

void set_tuple_num(std::shared_ptr<PageCache> pageCache, FileId fileId, int newTupleNum) {
    PageId firstPageId { fileId, 0 };
    std::byte* page = pageCache->read_page(firstPageId);

    std::int32_t offset = (sizeof PageMeta::hasTableMeta) + (sizeof PageMeta::dataPtr) +
            (sizeof TableMeta::fileId) + (sizeof TableMeta::lastPageId);
    memcpy(page + offset, &newTupleNum, sizeof newTupleNum); // TODO: check.

    if (pageCache->write(firstPageId) < 0) {
        std::cout << "FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::int32_t get_tuple_num(std::shared_ptr<PageCache> pageCache, FileId fileId) {
    PageId firstPageId { fileId, 0 };
    std::byte* page = pageCache->read_page(firstPageId);

    std::int32_t tupleNum;
    std::int32_t offset = (sizeof PageMeta::hasTableMeta) + (sizeof PageMeta::dataPtr) +
                          (sizeof TableMeta::fileId) + (sizeof TableMeta::lastPageId);
    memcpy(&tupleNum, page + offset, sizeof tupleNum); // TODO: check.

    return tupleNum;
}

void insert_tuple_on_page(std::shared_ptr<PageCache> pageCache,
                          PageId pageId,
                          std::shared_ptr<DenseTuple> data) {

    std::byte* page = pageCache->read_page(pageId);
    auto pageDataPtr = get_data_ptr(page);

    auto extractedData = data->extract();
    auto dataPtr = extractedData.first;
    auto dataSize = extractedData.second;

    if (pageDataPtr - get_header_length(page) < dataSize) {
        auto newPageId = newPage(pageCache, pageId.fileId);
        set_last_page(pageCache, newPageId);
        return insert_tuple_on_page(pageCache, newPageId, data);
    }

    memcpy(page + (pageDataPtr - dataSize), dataPtr, dataSize);
    set_data_ptr(page, pageDataPtr - dataSize);

    set_tuple_num(pageCache, pageId.fileId, get_tuple_num(pageCache, pageId.fileId) + 1);

    if (pageCache->write(pageId) < 0) {
        std::cout << "FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void insert_tuple_in_file(std::shared_ptr<PageCache> pageCache,
                          FileId fileId,
                          std::shared_ptr<DenseTuple> data) {
    auto lastPage = get_last_page(pageCache, fileId);
    PageId lastPageId { fileId, lastPage };
    insert_tuple_on_page(pageCache, lastPageId, data);
}

std::shared_ptr<TableScheme> get_m_tables_scheme() {
    std::vector<ColumnScheme> m_tables_columns;
    m_tables_columns.push_back(ColumnScheme("qid", TypeTag::INT));
    m_tables_columns.push_back(ColumnScheme("name", TypeTag::VARCHAR, MAX_NAME_LENGTH));
    auto m_tables_columns_ptr = std::make_shared<std::vector<ColumnScheme>>(m_tables_columns);

    TableScheme m_tables_scheme(m_tables_columns_ptr);
    auto m_tables_scheme_ptr = std::make_shared<TableScheme>(m_tables_scheme);

    return m_tables_scheme_ptr;
}

std::shared_ptr<TableScheme> get_m_columns_scheme() {
    std::vector<ColumnScheme> m_columns_columns;
    m_columns_columns.push_back(ColumnScheme("qid", TypeTag::INT));
    m_columns_columns.push_back(ColumnScheme("table_qid", TypeTag::INT));
    m_columns_columns.push_back(ColumnScheme("name", TypeTag::VARCHAR, MAX_NAME_LENGTH));
    m_columns_columns.push_back(ColumnScheme("type_id", TypeTag::INT));
    auto m_columns_columns_ptr = std::make_shared<std::vector<ColumnScheme>>(m_columns_columns);

    TableScheme m_columns_scheme(m_columns_columns_ptr);
    auto m_columns_scheme_ptr = std::make_shared<TableScheme>(m_columns_scheme);

    return m_columns_scheme_ptr;
}

void create_table_file(std::shared_ptr<PageCache> pageCache,
                       FileId fileId) {
    auto pageId = pageCache->create_page(fileId);

    PageMeta page_meta{ 1, pageCache->page_size() };
    TableMeta table_meta{ pageId.fileId.id, pageId.id };

    std::byte* page = pageCache->read_page(pageId);
    write_page_header(page, &page_meta, &table_meta);

    if (pageCache->write(pageId) < 0) {
        std::cout << "FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void insert_into_m_tables(std::shared_ptr<PageCache> pageCache,
                          FileId fileId, std::string name) {
    auto m_tables_scheme_ptr = get_m_tables_scheme();
    DenseTuple denseTuple = DenseTuple(m_tables_scheme_ptr);

    denseTuple.setInt(0, fileId.id);
    denseTuple.setChar(1, name);

    auto denseTuplePtr = std::make_shared<DenseTuple>(denseTuple);
    insert_tuple_in_file(pageCache, FileId{ M_TABLES_FILE_ID }, denseTuplePtr);
}

void insert_into_m_columns(std::shared_ptr<PageCache> pageCache,
                           FileId fileId,
                           std::shared_ptr<TableScheme> tableScheme) {
    auto m_columns_scheme_ptr = get_m_columns_scheme();

    for (int i = 0; i < tableScheme->columnsCount(); ++i) {
        DenseTuple denseTuple = DenseTuple(m_columns_scheme_ptr);

        denseTuple.setInt(0, i);
        denseTuple.setInt(1, fileId.id);
        denseTuple.setChar(2, tableScheme->name(i));
        denseTuple.setInt(3, tableScheme->typeTag(i));

        insert_tuple_in_file(pageCache, FileId{ M_COLUMNS_FILE_ID }, std::make_shared<DenseTuple>(denseTuple));
    }
}

void init_m_tables(std::shared_ptr<PageCache> pageCache) {
    FileId m_tables_file_id { M_TABLES_FILE_ID };
    FileId m_columns_file_id { M_COLUMNS_FILE_ID };

    create_table_file(pageCache, m_tables_file_id);
    create_table_file(pageCache, m_columns_file_id);

    insert_into_m_tables(pageCache, m_tables_file_id, "m_tables");
    insert_into_m_columns(pageCache, m_tables_file_id, get_m_tables_scheme());

    insert_into_m_tables(pageCache, m_columns_file_id, "m_columns");
    insert_into_m_columns(pageCache, m_columns_file_id, get_m_columns_scheme());

    pageCache->sync();
}

std::shared_ptr<Table> create_table(std::shared_ptr<PageCache> pageCache,
                                    FileId fileId, std::string name,
                                    std::shared_ptr<TableScheme> tableScheme) {
    /*
     * TODO:
     *
     * Реализовать функцию создания таблицы
     *
     * Она должна:
     * 
     *   - создать табличный файл
     *   - создать в нем мета-страницу
     *   - зарегистрировать таблицу и поля в мета-таблицах
     *
     * Все манипуляции со содердимым таблиц - непосредственно в кеше страниц
     * без дополнительных копирований
     *
     * Приводим указатель к соотвествующей Meta-структуре и пишем-читаем поля
     *
     * Конкретный состав данных для хранения можно определить самостоятельно
     * Meta-структуры можно редактировать
     *
     * Избегайте хранения того, что известно из других источников
     * Например, не стоит хранить в каждой записи о том, что длина конкретного 
     * поля равна 4, если мы и так знаем, что это поле типа INT
     * 
     * Для простоты считаем, что ошибок не бывает
     * И пока не поддерживаем тип TEXT
     *
     */

    create_table_file(pageCache, fileId);
    insert_into_m_tables(pageCache, fileId, name);
    insert_into_m_columns(pageCache, fileId, tableScheme);

    pageCache->sync();

    Table table(name, tableScheme, fileId);
    return std::make_shared<Table>(table);
}

int insert_tuple(std::shared_ptr<PageCache> pageCache,
                  std::shared_ptr<Table> table,
                  std::shared_ptr<DenseTuple> data) {
    /*
     * TODO:
     *
     * Реализовать функцию добавления записи
     *
     * Она должна:
     *  
     *   - при необходимости создать в нем новую страницу в таблице
     *   - обновив данные, зафиксировать добавление новой записи
     *
     * 
     * Для простоты считаем, что  ошибок не бывает
     *
     */

    insert_tuple_in_file(pageCache, table->fileId(), data);

    pageCache->sync();

    return get_tuple_num(pageCache, table->fileId());
}

std::vector<DenseTuple> select_all(std::shared_ptr<PageCache> pageCache, std::shared_ptr<Table> table) {
    /*
     * TODO:
     *
     * Реализовать упрощенный SELECT 
     *
     * Она должна:
     *  
     *   - выбрать все записи таблицы в произвольном порядке 
     *
     * 
     * Для простоты считаем, что  ошибок не бывает
     *
     */

    std::vector<DenseTuple> selected;

    auto fileId = table->fileId();
    auto dataSize = table->scheme()->totalSize();

    auto lastPageId = get_last_page(pageCache, table->fileId());
    for (int i = 0; i <= lastPageId; ++i) {
        PageId pageId{ fileId, i };

        std::byte* page = pageCache->read_page(pageId);
        auto pageDataPtr = get_data_ptr(page);

        while (pageDataPtr < pageCache->page_size()) {
            selected.push_back(DenseTuple(table->scheme(), page + pageDataPtr));
            pageDataPtr += dataSize;
        }
    }

    return selected;
}

std::vector<DenseTuple> select(std::shared_ptr<Storage> pageCache, std::shared_ptr<Table> table) {
    /*
     * TODO:
     *
     * Реализовать SELECT 
     *
     * Поскольку у нас не было общей структуры данных для хранения результатов синтаксического разбора -
     * можно (и нужно) добавить параметры, опимсывающие перечень выражений и критерий выбора
     *
     * Она должна:
     *  
     *   - выбрать все записи таблицы в произвольном порядке 
     *
     * 
     * Для простоты считаем, что  ошибок не бывает
     *
     */

     return std::vector <DenseTuple>{};
}

