#include "types.h"

#include <cstdio>
#include <cerrno>
#include <cassert>
#include <climits>

#include <bit>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <iostream>

#define PROG_NAME "read_script"
#define ERROR_PREF PROG_NAME ": "

#define SCNR_MAGIC 0x73636e72 /* "scnr" */

std::string PrettyPrintMagic(uint32_t magic) {
    char raw[5];
    raw[0] = magic >> 3 * CHAR_BIT;
    raw[1] = magic >> 2 * CHAR_BIT;
    raw[2] = magic >> 1 * CHAR_BIT;
    raw[3] = magic;
    raw[4] = '\0';

    return std::string(raw);
}

template<class T>
class AddressFactory {
  public:
    AddressFactory(char *data, uint32_t offset) : data_(data), offset_(offset) {}

    T *Resolve(uint32_t addr) {
        assert(reinterpret_cast<uintptr_t>(data_ + addr - offset_) %
               alignof(T) == 0);
        return reinterpret_cast<T*>(&data_[addr - offset_]);
    }
  private:
    char *data_;
    uint32_t offset_;
};

static void decode_scripts(char *map, Tag *scnr)
{
    /* TODO */
}

static void analyze(char *map, uint32_t sz)
{
    /* Header is at offset 0 */
    Header *header = static_cast<Header *>(static_cast<void *>(map));

    printf("found checksum: %x\n", header->checksum);

    uint32_t meta_offset = header->meta_offset;

    if (reinterpret_cast<uintptr_t>(map + meta_offset) % alignof(Meta)) {
        printf("inappropriate alignment of meta pointer: %p\n", map + meta_offset);
        return;
    }

    Meta *meta = reinterpret_cast<Meta *>(map + meta_offset);

    uint32_t tag_table_address = meta->tag_table_address;

    AddressFactory<Tag> tag_addr_factory(map,
                                       meta->tag_group_table_address -
                                       sizeof(Meta) - meta_offset);

    Tag *first_tag = tag_addr_factory.Resolve(tag_table_address);
    uint32_t first_tag_address = first_tag->memory_address;

    Tag *scnr = nullptr;

    /* look for a single scnr tag */
    for (uint32_t i = 0; i < meta->nr_tags; ++i) {
        Tag *tag = &first_tag[i];
        if (tag->tag_group_magic == SCNR_MAGIC) {
            printf("tag: %d (offset: %lx, vaddr: %p): magic: %s\n", i,
                   reinterpret_cast<char *>(tag) - map, tag,
                   PrettyPrintMagic(tag->tag_group_magic).c_str());
            if (scnr) {
                printf(ERROR_PREF "broken map? found another scnr tag.\n");
                return;
            }
            scnr = tag;
            break;
        }
    }

    if (!scnr) {
        printf(ERROR_PREF "broken map? no scnr tag.\n");
        return;
    }

    AddressFactory<char> data_addr_factory(map, first_tag_address -
                                           header->tag_data_offset -
                                           meta_offset);
    char *tag_data = data_addr_factory.Resolve(scnr->memory_address);
    printf("scnr data: %lx (vaddr: %p)\n", tag_data - map, tag_data);
    uint32_t *nr_scripts = reinterpret_cast<uint32_t*>(tag_data + 0x1b8);
    printf("Found %d scripts\n", *nr_scripts);
    // decode_scripts(map, scnr);
}

static bool verify_filesize(int map, uint32_t sz)
{
    struct stat buf;
    if (fstat(map, &buf)) {
        perror(ERROR_PREF "fstat failed");
        return false;
    }

    return sz == buf.st_size;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: " PROG_NAME " <map file>\n");
        return -1;
    }

    char *filename = argv[1];
    int map = open(filename, O_RDONLY | O_CLOEXEC);
    if (map < 0) {
        perror(ERROR_PREF "open failed");
        return errno;
    }

    void *header_bytes = mmap(NULL, sizeof(Header), PROT_READ,
                              MAP_PRIVATE, map, 0);
    if (header_bytes == MAP_FAILED) {
        perror(ERROR_PREF "mmap header failed");
        return errno;
    }

    Header *header = static_cast<Header *>(header_bytes);
    uint32_t sz = header->file_size;
    if (!verify_filesize(map, sz)) {
        printf(ERROR_PREF "file size mismatch");
        return -1;
    }

    if (munmap(header_bytes, sizeof(Header))) {
        perror(ERROR_PREF "unmapping header failed");
        return errno;
    }

    void *map_bytes = mmap(NULL, sz, PROT_READ, MAP_PRIVATE, map, 0);
    if (map_bytes == MAP_FAILED) {
        perror(ERROR_PREF "mapping map failed");
        return errno;
    }

    close(map);
    printf("mapped %s @ %p\n", filename, map_bytes);
    analyze(static_cast<char *>(map_bytes), sz);

    return 0;
}
