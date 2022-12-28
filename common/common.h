#include <table/table.h>
#include <pagecache/pagecache.h>

void print_page_meta(PageMeta pageMeta);
PageMeta* read_page_meta(std::byte* data);
void write_page_meta(std::byte* dest, PageMeta pageMeta);

void create_oracle_table(std::shared_ptr<PageCache> pageCachePtr);


std::vector<DenseTuple> read_from_table(std::shared_ptr<PageCache> pageCachePtr, PageId pageId, std::shared_ptr<TableScheme> tableScheme);
