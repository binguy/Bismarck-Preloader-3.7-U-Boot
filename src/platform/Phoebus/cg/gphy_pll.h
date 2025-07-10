#ifndef __GPHY_PLL_H__
#define __GPHY_PLL_H__


typedef union {
	struct {
		unsigned int mbz:16;
		unsigned int ck200m_lx_pll:2;      //0:/5, 1: /6, 2: /7, 3: /8
		unsigned int ck250m_pbo_pll:2; 
		unsigned int ck250m_spif_pll:2;    //0:/4, 1: /5, 2: /6, 3: /7
		unsigned int tbc_9_0:10; 
	} f;
	unsigned int v;
} PHY_RG5X_PLL_T;
#define PHY_RG5X_PLLrv (*((regval)0xBB01F054))
#define RMOD_PHY_RG5X_PLL(...) rset(PHY_RG5X_PLL, PHY_RG5X_PLLrv, __VA_ARGS__)
#define RFLD_PHY_RG5X_PLL(fld) (*((const volatile PHY_RG5X_PLL_T *)0xBB01F054)).f.fld




#endif  //__GPHY_PLL_H__


