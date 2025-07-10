#ifndef _MEMCNTLR_UTIL_H_
#define _MEMCNTLR_UTIL_H_


#define ARY(var, bnum, q) s##bnum##_t var[q]
#define FLD(var, b)    s32_t var:b
#define BOOL(var)      FLD(var, 2)

#define ARYR(var, tpe, q) tpe var[q]
#define FLDR(var, tpe)    tpe var


#include <init_result_helper.h>

INIT_RESULT_GROUP(cal,
                  MEM_CAL_UNINIT,
                  MEM_CAL_OK,
                  MEM_CAL_FAIL);
#endif //_MEMCNTLR_UTIL_H_

