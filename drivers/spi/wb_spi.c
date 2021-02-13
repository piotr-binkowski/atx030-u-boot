#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>

#define SPI0_BASE	0xC4000000
#define SPI1_BASE	0xCC000000
#define SPI2_BASE	0xD0000000

#define SPI_DATA	0x0
#define SPI_STATUS	0x4

#define SPI_SS		0x1
#define SPI_BUSY	0x2

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
	writeb(0, ss->base + SPI_STATUS);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct wb_spi *ss = to_wb_spi(slave);
	writeb(1, ss->base + SPI_STATUS);
}

static int spi_check_done(struct wb_spi *ss)
{
	return !(readb(ss->base + SPI_STATUS) & SPI_BUSY);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct wb_spi *ss = to_wb_spi(slave);
	void * const base = ss->base;
	const u8 *txd = dout;
	unsigned int bytes;
	u8 *rxd = din;

	if(bitlen % 8)
		return -1;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	bytes = bitlen / 8;

	for(bytes; bytes > 0; bytes--) {
		u8 tx = txd ? *(txd++) : 0xff;

		writeb(tx, base + SPI_DATA);

		while(!spi_check_done(ss));

		if(rxd)
			*rxd++ = readb(base + SPI_DATA);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return(0);
}
