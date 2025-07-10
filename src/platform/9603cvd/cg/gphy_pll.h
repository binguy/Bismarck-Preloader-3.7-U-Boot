#ifndef __GPHY_PLL_H__
#define __GPHY_PLL_H__

SKC35_REG_DEC(
	PHY_RG5X_PLL, 0xbb01f054,
	RF_RSV(31, 16);
	RFIELD(15, 14, ck200m_lx_pll_div);
	RF_RSV(13, 12);
	RFIELD(11, 10, ck250m_spif_pll_div);
	RF_RSV(9, 0);
	);

#endif  //__GPHY_PLL_H__


