#ifndef _EFUSE_SETUP_H_
#define _EFUSE_SETUP_H_


/***** APro *********/
extern void apro_efuse_setup_fcn(void);
extern unsigned short apro_efuse_read_entry(unsigned char entry);

/***** APro Gen2 ****/
extern void apro_otp_setup_fcn(void);
extern unsigned short apro_otp_read_entry(unsigned char entry);

/***** 9603C-VD *****/
extern void rtl9603cvd_efuse_setup_fcn(void);
extern unsigned short rtl9603cvd_efuse_read_entry(unsigned char entry);

/***** Cross Project *****/
unsigned short efuse_read(unsigned char entry);

#endif
