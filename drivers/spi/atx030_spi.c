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

static void spi_rx(struct atx030_spi *ss, unsigned int len, u8 *rxd)
{
	void * const base = ss->base;
	for(len; len > 0; len--) {
		u16 tx = 0xffff;

		writew(tx, base + SPI_DRW);

		while(!spi_check_done(ss));

		*((u16*)rxd) = readw(base + SPI_DRW);
		rxd += 2;
	}
}

static void spi_tx(struct atx030_spi *ss, unsigned int len, const u8 *txd)
{
	void * const base = ss->base;
	for(len; len > 0; len--) {
		u16 tx = *(u16*)txd;
		txd += 2;

		writew(tx, base + SPI_DRW);

		while(!spi_check_done(ss));
	}
}

static void spi_txrx(struct atx030_spi *ss, unsigned int len, u8 *rxd, const u8 *txd)
{
	void * const base = ss->base;
	for(len; len > 0; len--) {
		u16 tx = *(u16*)txd;
		txd += 2;

		writew(tx, base + SPI_DRW);

		while(!spi_check_done(ss));

		*((u16*)rxd) = readw(base + SPI_DRW);
		rxd += 2;
	}
}

static void spi_dummy(struct atx030_spi *ss, unsigned int len)
{
	void * const base = ss->base;
	for(len; len > 0; len--) {
		u16 tx = 0xffff;

		writew(tx, base + SPI_DRW);

		while(!spi_check_done(ss));
	}
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct atx030_spi *ss = to_atx030_spi(slave);
	void * const base = ss->base;
	const u8 *txd = dout;
	unsigned int bytes;
	u8 *rxd = din;

	if(bitlen % 8)
		return -1;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	bytes = bitlen / 8;

	if (bytes > 1) {
		if(txd && rxd)
			spi_txrx(ss, bytes / 2, rxd, txd);
		else if (txd)
			spi_tx(ss, bytes / 2, txd);
		else if (rxd)
			spi_rx(ss, bytes / 2, rxd);
		else
			spi_dummy(ss, bytes / 2);

		if (txd)
			txd += (bytes / 2) * 2;

		if (rxd)
			rxd += (bytes / 2) * 2;

		bytes = bytes % 2;
	}


	if (bytes) {
		u8 tx = txd ? *txd : 0xff;

		writeb(tx, base + SPI_DRWB);

		while(!spi_check_done(ss));

		if(rxd)
			*rxd++ = readb(base + SPI_DRRB);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return(0);
}
