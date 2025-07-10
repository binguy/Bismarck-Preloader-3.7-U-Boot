#include <util.h>
#include <soc_exam_util/mt_ram_test.h>

#ifndef MSRRrv
#define MSRRrv      REG32(0xB8001038)
#define DOR0rv      REG32(0xB8004200)
#define DMAR0rv     REG32(0xB8004204)
#define UMSARrv(n)  REG32(0xB8001300+n*0x10)
#define UMSSRrv(n)  REG32(0xB8001304+n*0x10)
#endif
#define FLUSH_OCP_CMD()             ({ MSRRrv=0x80000000; while(0x40000000!=MSRRrv);})
#define ZONE0_OFFSET_CONFIG(off)    ({ DOR0rv=(off); })
#include <cpu/tlb.h>// MIPs CPU only now

#ifndef SECTION_MT
	#define SECTION_MT
#endif

typedef struct {
    u32_t end;
    u32_t start;
    u8_t en;
} UM_T;

UTIL_FAR SECTION_UNS_RO 
void mt_tlb_setting(u32_t va, u32_t la, u32_t tlbi) {
    tlb_entry_hi_t eh={.v=_TLB_GET_ENTRY_HI()};
    tlb_entry_lo_t el={.v=0};

    eh.f.vpn2 = va>>13;
    //eh.f.asid follows current

    el.f.pfn = la>>12;
    el.f.c = 2;//3;
    el.f.v = 1;
    el.f.d = 1;
    el.f.g = 1;
    TLB_SET_ENTRY(tlbi, eh.v, el.v, 0);  // FIXME
}

/* instruction side is SRAM only */
SECTION_MT
int mt_highmem_test(u32_t max_size)
{
    u32_t patWR, start, u, p, step=SIZE_256MB;
    u32_t addr_max, zen=0;
    volatile u32_t *addr;
    UM_T unmap[4];

    printf("II: %s...\n", __FUNCTION__);
    // Checking MC Zone exist?
    u32_t VA, LA, TLBidx=0;
    if(0xDEADBEEF==DMAR0rv) {   // Sheipa
        // using TLB
        // read pagemask, if nand boot => 4k, else 256MB
        u32_t pm = TLB_GET_PAGEMASK();
        if(pm==PAGEMASK_4KB) {
            VA=0xC8000000;
            step = 0x1000;
            TLBidx = 32;
        } else {
            VA=0xC0000000;
            TLB_SET_PAGEMASK(PAGEMASK_256MB);
        }
        LA=0x80000000;  // Sheipa: 0x8000_0000, non-Sheipa: 0x0000_0000
        unmap[0].en = unmap[1].en = unmap[2].en = unmap[3].en = 0;
    } else {                    // D.W.
        VA=0x80000000;
        LA=0x00000000;
        zen=1;

        // Checking bypass area
        for(u=0; u<4; u++) {
            unmap[u].start = UMSARrv(u);
            unmap[u].en = unmap[u].start&0x1;
            unmap[u].start &= (~0x1);
            unmap[u].end = unmap[u].start+(1<<(7+(UMSSRrv(u)&0xF)));
            if(unmap[u].en) {
                printf("II: unmap[%d]... start=0x%8x, end=0x%8x\n", u, unmap[u].start, unmap[u].end);
            }
        }
    }


    dcache_wr_inv_all();
    // write
    for (start=0, p=0; start<max_size; start+=step){
        if(zen) {
            u32_t _p = start>>28;
            if(p!=_p) {
                dcache_wr_inv_all();
                FLUSH_OCP_CMD();
                ZONE0_OFFSET_CONFIG(start);
                p=_p;
            }
        } else {
            mt_tlb_setting(VA, LA+start, TLBidx);
        }

        printf("WR to DDR PA 0x%x\r", start);
        addr = (volatile u32_t *)(VA);
        addr_max = (u32_t)addr+step;

        // using address as pattern
        for(patWR=(u32_t)(LA+start); (u32_t)addr<addr_max; addr++, patWR+=4) {
            if( (unmap[0].en && (patWR>=unmap[0].start && patWR<unmap[0].end)) ||
                (unmap[1].en && (patWR>=unmap[1].start && patWR<unmap[1].end)) ||
                (unmap[2].en && (patWR>=unmap[2].start && patWR<unmap[2].end)) ||
                (unmap[3].en && (patWR>=unmap[3].start && patWR<unmap[3].end))) {
                continue;
            }

            *(addr) = patWR;
/*            if(*(addr) != patWR) {
                EPRINTF("expected=0x%x, (0x%x)=0x%x zone_offset(0x%x)\n",
                         patWR, addr, *addr, start);

                return MT_FAIL;
            }*/
        }
    }

    // read
    for (start=0, p=0; start<max_size; start+=step){
        if(zen) {
            u32_t _p = start>>28;
            if(p!=_p) {
                dcache_wr_inv_all();
                FLUSH_OCP_CMD();
                ZONE0_OFFSET_CONFIG(start);
                p=_p;
            }
        } else {
            mt_tlb_setting(VA, LA+start, TLBidx);
        }

        printf("RD from DDR PA 0x%x\r", start);
        addr = (volatile u32_t *)(VA);
        addr_max = (u32_t)addr+step;

        for(patWR=(u32_t)(LA+start); (u32_t)addr<addr_max; addr++, patWR+=4) {
            if( (unmap[0].en && (patWR>=unmap[0].start && patWR<unmap[0].end)) ||
                (unmap[1].en && (patWR>=unmap[1].start && patWR<unmap[1].end)) ||
                (unmap[2].en && (patWR>=unmap[2].start && patWR<unmap[2].end)) ||
                (unmap[3].en && (patWR>=unmap[3].start && patWR<unmap[3].end))) {
                continue;
            }
            if(*(addr) != patWR) {
                EPRINTF("expected=0x%x, (0x%x)=0x%x, LA(0x%x)\n",
                         patWR, addr, *addr, LA+start);
                return MT_FAIL;
            }
        }
    }
    puts(" passed\n");
    return MT_SUCCESS;
}

/* #define USING_MEM_ZONE
  * This should be define in project/xxxx/arch.h
  */

SECTION_MT
int mt_highmem_simple_normal_test(u32_t max_size)
{
    // do some test here
    u32_t patWR, start, addr_max;
    volatile u32_t *addr;
    const u32_t dram_patterns[] = {0xa5a55a5a, 0x5a5aa5a5};
    u32_t pidx, total_pattern=sizeof(dram_patterns)/sizeof(u32_t);

    printf("II: %s...", __FUNCTION__);

#ifndef USING_MEM_ZONE
    #define VA (0xC0000000)
    #define LA (0x90000000)
    TLB_SET_PAGEMASK(0x1fffe000);  //pagemask = 256MB
    tlb_entry_hi_t eh={.v=0};
    tlb_entry_lo_t el={.v=0};

    eh.f.vpn2 = VA>>13;
    eh.f.asid = 0;

    el.f.pfn = LA>>12;
    el.f.c = 3;
    el.f.v = 1;
    el.f.d = 1;
    el.f.g = 1;
    TLB_SET_ENTRY(0, eh.v, el.v, 0);  // FIXME
#else
    #define VA (0x80000000)
    #define LA (0x00000000)
#endif

    // write
    dcache_wr_inv_all();
    puts("WR");
    for (start=0, pidx=0; start<max_size; start+=SIZE_256MB, pidx++){
        printf("(0x%x) ", start);
        patWR = dram_patterns[pidx%total_pattern];
        addr = (volatile u32_t *)(VA);
        addr_max = (u32_t)addr+SIZE_256MB;

#ifdef USING_MEM_ZONE
        FLUSH_OCP_CMD();
        ZONE0_OFFSET_CONFIG(start);
        FLUSH_OCP_CMD();
#endif

        for(;(u32_t)addr<addr_max;addr++){
            #ifndef OTTO_FLASH_NOR
            if((pidx==1)&&(((u32_t)addr>=0x8F000000)&&((u32_t)addr<0x8F008000))){
                continue;
            }
            #endif
            *(addr) = patWR;
        }
        dcache_wr_inv_all();
    }


    // read
    puts("RD");
    for (start=0, pidx=0; start<max_size; start+=SIZE_256MB, pidx++){
        printf("(0x%x) ", start);
        patWR = dram_patterns[pidx%total_pattern];
        addr = (volatile u32_t *)(VA);
        addr_max = (u32_t)addr+SIZE_256MB;

#ifdef USING_MEM_ZONE
        FLUSH_OCP_CMD();
        ZONE0_OFFSET_CONFIG(start);
        FLUSH_OCP_CMD();
#endif

        for(;(u32_t)addr<addr_max;addr++){
            #ifndef OTTO_FLASH_NOR
            if((pidx==1)&&(((u32_t)addr>=0x8F000000)&&((u32_t)addr<0x8F008000))){
                continue;
            }
            #endif
            if(*(addr) != patWR) {
                EPRINTF("expected=0x%x, (0x%x)=0x%x zone_offset(0x%x)\n", patWR, addr, *addr, start);
                return MT_FAIL;
            }
        }
        dcache_wr_inv_all();
    }
    puts(" passed\n");
    return MT_SUCCESS;
}

