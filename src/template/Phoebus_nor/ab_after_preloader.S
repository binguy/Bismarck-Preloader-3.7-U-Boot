#include <soc.h>

  .section .next_env, "a"
  .global next_env
next_env:
#if (BUILDTEST != 1)
#if (PLR_VERSION==0x03000700)
  .incbin "uboot_v2020.01/u-boot.pimg"
#elif (PLR_VERSION==0x03000500)
  .incbin "uboot/u-boot.pimg"
#endif
#endif
