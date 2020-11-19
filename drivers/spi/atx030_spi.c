#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>

#define SPI0_BASE 0xE0040000
#define SPI1_BASE 0xE0070000
#define SPI_SR 1
#define SPI_CR 1
#define SPI_DRWB 2
#define SPI_DRRB 3
#define SPI_DRW 4
#define SPI_DONE 0x02

struct atx030_spi {
	struct spi_slave slave;
	void *base;
};

static inline struct atx030_spi *to_atx030_spi(struct spi_slave *slave)
{
	return container_of(slave, struct atx030_spi, slave);
}

void spi_init(void)
{

}

void spi_set_speed(struct spi_slave *slave, uint hz)
{

}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if((bus == 0 || bus == 1) && cs == 0)
		return 1;
	else
		return 0;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct atx030_spi *ss;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ss = spi_alloc_slave(struct atx030_spi, bus, cs);
	if (!ss)
		return NULL;

	if(bus == 0)
		ss->base = (void*)SPI0_BASE;
	else
		ss->base = (void*)SPI1_BASE;

	return &ss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct atx030_spi *ss = to_atx030_spi(slave);

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
	struct atx030_spi *ss = to_atx030_spi(slave);
	writeb(0x00, ss->base + SPI_CR);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	struct atx030_spi *ss = to_atx030_spi(slave);
	writeb(0x01, ss->base + SPI_CR);
}

static int spi_check_done(struct atx030_spi *ss)
{
	return readb(ss->base + SPI_SR) & SPI_DONE;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct atx030_spi *ss = to_atx030_spi(slave);
	const u8 *txd = dout;
	unsigned bytes;
	u8 *rxd = din;

	if(bitlen % 8)
		return -1;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	for(bytes = bitlen / 8; bytes > 1; bytes-=2) {
		u16 tx;

		if(txd) {
			tx = *((u16*)txd);
			txd += 2;
		} else {
			tx = 0xffff;
		}

		writew(tx, ss->base + SPI_DRW);

		while(!spi_check_done(ss));

		if(rxd) {
			*((u16*)rxd) = readw(ss->base + SPI_DRW);
			rxd += 2;
		}
	}

	if (bytes) {
		u8 tx = txd ? *txd : 0xff;

		writeb(tx, ss->base + SPI_DRWB);

		while(!spi_check_done(ss));

		if(rxd)
			*rxd++ = readb(ss->base + SPI_DRRB);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return(0);
}
