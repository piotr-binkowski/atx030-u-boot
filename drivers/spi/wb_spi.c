#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>

#define SPI0_BASE	0xC4000000
#define SPI1_BASE	0xCC000000
#define SPI2_BASE	0xD0000000

#define WB_SPI_DATA_WR		0x0
#define WB_SPI_DATA_RD		0x0
#define WB_SPI_DATA_RD_B	0x3
#define WB_SPI_STATUS		0x4

#define WB_SPI_SS		0x1
#define WB_SPI_RX_EMPTY		0x2
#define WB_SPI_TX_FULL		0x4
#define WB_SPI_FIFO_DEPTH	16

struct wb_spi {
	struct spi_slave slave;
	void *base;
};

static inline struct wb_spi *to_wb_spi(struct spi_slave *slave)
{
	return container_of(slave, struct wb_spi, slave);
}

void spi_init(void)
{

}

void spi_set_speed(struct spi_slave *slave, uint hz)
{

}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if((bus == 0 || bus == 1 || bus == 2) && cs == 0)
		return 1;
	else
		return 0;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct wb_spi *ss;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ss = spi_alloc_slave(struct wb_spi, bus, cs);
	if (!ss)
		return NULL;

	switch(bus) {
		case 0:
			ss->base = (void*)SPI0_BASE;
			break;
		case 1:
			ss->base = (void*)SPI1_BASE;
			break;
		case 2:
			ss->base = (void*)SPI2_BASE;
			break;
	}

	return &ss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct wb_spi *ss = to_wb_spi(slave);

	free(ss);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{

}

void spi_cs_activate(struct spi_slave *slave)
{
	struct wb_spi *ss = to_wb_spi(slave);
	writeb(0, ss->base + WB_SPI_STATUS);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct wb_spi *ss = to_wb_spi(slave);
	writeb(1, ss->base + WB_SPI_STATUS);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct wb_spi *ss = to_wb_spi(slave);
	void *base = ss->base;
	const u8 *txd = dout;
	u8 *rxd = din;
	u8 leftover;
	u32 len;
	u16 i;

	if(bitlen % 8)
		return -1;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	len = bitlen / 8;

	leftover = len % 4;

	for(len = (len - leftover) / 4; len > 0;) {
		const u16 tgt_len = (len > WB_SPI_FIFO_DEPTH) ? WB_SPI_FIFO_DEPTH : len;
		u32 tmp = 0xffffffff;
		for(i = 0; i < tgt_len; i++) {
			tmp = txd ? *((u32*)txd) : tmp;
			__raw_writel(tmp, base + WB_SPI_DATA_WR);
			if(txd)
				txd += 4;
		}
		for(i = 0; i < tgt_len; i++) {
			while(readb(base + WB_SPI_STATUS) & WB_SPI_RX_EMPTY);
			tmp = __raw_readl(base + WB_SPI_DATA_RD);
			if(rxd) {
				*((u32*)rxd) = tmp;
				rxd += 4;
			}
		}
		len -= tgt_len;
	}

	for(len = leftover; len > 0; len--) {
		u8 tmp = txd ? *(txd++) : 0xff;
		writeb(tmp, base + WB_SPI_DATA_WR);
		while(readb(base + WB_SPI_STATUS) & WB_SPI_RX_EMPTY);
		tmp = readb(base + WB_SPI_DATA_RD_B);
		if(rxd)
			*(rxd++) = tmp;
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
