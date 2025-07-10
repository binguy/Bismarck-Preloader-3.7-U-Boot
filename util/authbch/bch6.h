#ifndef BCH6_H
#define BCH6_H

#define BCH6_PAGE_SIZE     512
#define BCH6_OOB_SIZE      6
#define BCH6_ECC_SIZE      10
#define BCH6_UNIT_SIZE     (BCH6_PAGE_SIZE+BCH6_OOB_SIZE+BCH6_ECC_SIZE)

extern void
bch6_ecc_512B_encode(unsigned char *ecc,                // ecc: output 10 bytes of ECC code
                     const unsigned char *input_buf,    // input_buf: the 512 bytes input data (BCH6_PAGE_SIZE bytes)
                     const unsigned char *oob);         // oob: 6 bytes out-of-band for input (BCH6_OOB_SIZE bytes)


#endif //BCH6_H

