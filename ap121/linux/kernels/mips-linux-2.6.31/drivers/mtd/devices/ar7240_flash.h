#ifndef _AR7240_SPI_H
#define _AR7240_SPI_H
#if 0
#define AR7240_SPI_FS           0x1f000000
#define AR7240_SPI_CLOCK        0x1f000004
#define AR7240_SPI_WRITE        0x1f000008
#define AR7240_SPI_READ         0x1f000000
#define AR7240_SPI_RD_STATUS    0x1f00000c

#define AR7240_SPI_CS_DIS       0x70000
#endif
#define AR7240_SPI_CE_LOW       0x60000
#define AR7240_SPI_CE_HIGH      0x60100
#if 0
#define AR7240_SPI_CMD_WREN         0x06
#define AR7240_SPI_CMD_RD_STATUS    0x05
#define AR7240_SPI_CMD_FAST_READ    0x0b
#define AR7240_SPI_CMD_PAGE_PROG    0x02
#define AR7240_SPI_CMD_SECTOR_ERASE 0xd8
#endif
#define AR7240_SPI_SECTOR_SIZE      (1024*64)
#define AR7240_SPI_PAGE_SIZE        256

#define AR7240_FLASH_MAX_BANKS  1

#define display(_x)     ar7240_reg_wr_nf(0x18040008, (_x))

/*
 * primitives
 */

#define ar7240_be_msb(_val, __i) (((_val) & (1 << (7 - __i))) >> (7 - __i))

#define ar7240_spi_bit_banger(_byte)  do {        \
    int _i;                                      \
    for(_i = 0; _i < 8; _i++) {                    \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_LOW | ar7240_be_msb(_byte, _i));  \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_HIGH | ar7240_be_msb(_byte, _i)); \
    }       \
}while(0);

#define ar7240_spi_go() do {        \
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CE_LOW); \
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS); \
}while(0);


#define ar7240_spi_send_addr(__a) do {				\
	ar7240_spi_bit_banger(((__a & 0xff0000) >> 16));	\
	ar7240_spi_bit_banger(((__a & 0x00ff00) >> 8));		\
	ar7240_spi_bit_banger(__a & 0x0000ff);			\
}while(0);

#define ar7240_spi_delay_8()    ar7240_spi_bit_banger(0)
#define ar7240_spi_done()       ar7240_reg_wr(AR7240_SPI_FS, 0)

int ar7240_flash_erase(struct mtd_info *mtd,struct erase_info *instr);
int ar7240_flash_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen ,u_char *buf);
int ar7240_flash_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
void ar7240_spi_sector_erase(uint32_t addr);


#endif /*_AR7240_SPI_H*/
