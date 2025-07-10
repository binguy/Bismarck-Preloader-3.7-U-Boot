#ifndef _MEM_SIZE_DEF_H_
#define _MEM_SIZE_DEF_H_

#define NA (0x4E41)

typedef enum {
    SIZE_128Mb = 0,
    SIZE_256Mb = 1,
    SIZE_512Mb = 2,
    SIZE_1Gb   = 3,
    SIZE_2Gb   = 4,
    SIZE_4Gb   = 5,
    SIZE_8Gb   = 6,
    SIZE_NA    = 0x4E41,
} memory_size_mapping_t;

#endif
