#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/unistd.h>
#include <sys/mman.h>
#include <stdint.h>

int main(int argc, char **argv)
{
    if (argc < 1 + 3) {
        printf("usage: calc <file> 0x<header length> 0x<full_length>\n");
        return -1;
    }

    char *fn = argv[1];
    char *h_len_s = argv[2];
    char *length_s = argv[3];

    off_t hlength = strtol(h_len_s, NULL, 16);
    off_t length = strtol(length_s, NULL, 16);

    if (hlength % sizeof(uint32_t)) {
        printf("header length is not uint32_t-aligned??\n");
        return -2;
    }

    if (length % sizeof(uint32_t)) {
        printf("data length is not uint32_t-aligned??\n");
        return -2;
    }

    printf("file: %s\n", fn);
    printf("hlen: %lx\n", hlength);
    printf("len: %lx\n", length);

    int fd = open(fn, 0, 0);
    if (fd < 0) {
        perror("open failed");
        return -errno;
    }

    char *data = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap failed");
        return -errno;
    }

    close(fd);

    uint32_t *intarr = (uint32_t*)&data[hlength];

    uint32_t result = 0x0;

    size_t intsleft = (length - hlength)/sizeof(uint32_t);
    while (intsleft-- > 0) {
        result ^= *intarr++;
    }

    printf("checksum: 0x%04x\n", result);
    return 0;
}
