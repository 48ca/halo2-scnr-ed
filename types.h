/*
 * Based on LordZedd's Assembly. (GPLv3)
 */
#include <cstddef>
#include <cstdint>

struct __attribute__((packed)) Header {
    union {
        struct __attribute__((packed)) {
            char pad1[8];
            uint32_t file_size;         // 0x8
            char pad2[4];
            uint32_t meta_offset;       // 0x10
            uint32_t tag_data_offset;   // 0x14
            uint32_t tag_data_size;     // 0x18
            uint32_t meta_size;         // 0x1c
            char pad3[0x100];           // 0x20
            char build_string[0x20];    // 0x120; length might be wrong
            uint32_t type;              // 0x140
            uint32_t crc_checksum;      // 0x144
            /* TODO: all the other fields */
            char pad4[0x188];           //
            uint32_t checksum;          // 0x2d0
        };
        char opaque[0x800];
    };
};

static_assert(sizeof(Header) == 0x800);
static_assert(offsetof(Header, file_size) == 0x8);
static_assert(offsetof(Header, meta_offset) == 0x10);
static_assert(offsetof(Header, tag_data_offset) == 0x14);
static_assert(offsetof(Header, tag_data_size) == 0x18);
static_assert(offsetof(Header, meta_size) == 0x1c);
static_assert(offsetof(Header, build_string) == 0x120);
static_assert(offsetof(Header, type) == 0x140);
static_assert(offsetof(Header, crc_checksum) == 0x144);
static_assert(offsetof(Header, checksum) == 0x2d0);

struct __attribute__((packed)) Meta {
    uint32_t tag_group_table_address;
    uint32_t nr_tag_groups;
    uint32_t tag_table_address;
    uint32_t scenario_datum_index;
    uint32_t map_globals_datum_index;
    char pad[4];
    uint32_t nr_tags;
    uint32_t magic;
};
static_assert(sizeof(Meta) == 0x20);
static_assert(offsetof(Meta, tag_group_table_address) == 0);
static_assert(offsetof(Meta, nr_tag_groups) == 0x4);
static_assert(offsetof(Meta, tag_table_address) == 0x8);
static_assert(offsetof(Meta, scenario_datum_index) == 0xc);
static_assert(offsetof(Meta, map_globals_datum_index) == 0x10);
static_assert(offsetof(Meta, nr_tags) == 0x18);
static_assert(offsetof(Meta, magic) == 0x1c);

struct __attribute__((packed)) TagGroup {
    uint32_t magic;
    uint32_t parent_magic;
    uint32_t grandparent_magic;
};
static_assert(sizeof(TagGroup) == 0xc);
static_assert(offsetof(TagGroup, magic) == 0x0);
static_assert(offsetof(TagGroup, parent_magic) == 0x4);
static_assert(offsetof(TagGroup, grandparent_magic) == 0x8);

struct __attribute__((packed)) Tag {
    uint32_t tag_group_magic;
    uint32_t datum_index;
    uint32_t memory_address;
    uint32_t data_size;
};
static_assert(sizeof(Tag) == 0x10);
static_assert(offsetof(Tag, tag_group_magic) == 0x0);
static_assert(offsetof(Tag, datum_index) == 0x4);
static_assert(offsetof(Tag, memory_address) == 0x8);
static_assert(offsetof(Tag, data_size) == 0xc);
