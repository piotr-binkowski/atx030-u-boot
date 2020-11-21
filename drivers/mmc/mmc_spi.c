/*
 * generic mmc spi driver
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */
#include <common.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>
#include <crc.h>
#include <linux/bitops.h>
#include <linux/crc7.h>
#include <asm/byteorder.h>
#include <spi.h>

/* MMC/SD in SPI mode reports R1 status always */
#define R1_SPI_IDLE			BIT(0)
#define R1_SPI_ERASE_RESET		BIT(1)
#define R1_SPI_ILLEGAL_COMMAND		BIT(2)
#define R1_SPI_COM_CRC			BIT(3)
#define R1_SPI_ERASE_SEQ		BIT(4)
#define R1_SPI_ADDRESS			BIT(5)
#define R1_SPI_PARAMETER		BIT(6)
/* R1 bit 7 is always zero, reuse this bit for error */
#define R1_SPI_ERROR			BIT(7)

/* Response tokens used to ack each block written: */
#define SPI_MMC_RESPONSE_CODE(x)	((x) & 0x1f)
#define SPI_RESPONSE_ACCEPTED		((2 << 1)|1)
#define SPI_RESPONSE_CRC_ERR		((5 << 1)|1)
#define SPI_RESPONSE_WRITE_ERR		((6 << 1)|1)

/* Read and write blocks start with these tokens and end with crc;
 * on error, read tokens act like a subset of R2_SPI_* values.
 */
/* single block write multiblock read */
#define SPI_TOKEN_SINGLE		0xfe
/* multiblock write */
#define SPI_TOKEN_MULTI_WRITE		0xfc
/* terminate multiblock write */
#define SPI_TOKEN_STOP_TRAN		0xfd

/* MMC SPI commands start with a start bit "0" and a transmit bit "1" */
#define MMC_SPI_CMD(x) (0x40 | (x))

/* bus capability */
#define MMC_SPI_VOLTAGE			(MMC_VDD_32_33 | MMC_VDD_33_34)
#define MMC_SPI_MIN_CLOCK		400000	/* 400KHz to meet MMC spec */
#define MMC_SPI_MAX_CLOCK		25000000 /* SD/MMC legacy speed */

/* timeout value */
#define CMD_TIMEOUT			8
#define READ_TIMEOUT			3000000 /* 1 sec */
#define WRITE_TIMEOUT			3000000 /* 1 sec */
#define R1B_TIMEOUT			3000000 /* 1 sec */

static int mmc_spi_sendcmd(struct mmc *mmc,
			   ushort cmdidx, u32 cmdarg, u32 resp_type,
			   u8 *resp, u32 resp_size,
			   bool resp_match, u8 resp_match_value, bool r1b)
{
	struct spi_slave *spi = mmc->priv;
	int i, rpos = 0, ret = 0;
	u8 cmdo[7], r;

	debug("%s: cmd%d cmdarg=0x%x resp_type=0x%x "
	      "resp_size=%d resp_match=%d resp_match_value=0x%x\n",
	      __func__, cmdidx, cmdarg, resp_type,
	      resp_size, resp_match, resp_match_value);

	cmdo[0] = 0xff;
	cmdo[1] = MMC_SPI_CMD(cmdidx);
	cmdo[2] = cmdarg >> 24;
	cmdo[3] = cmdarg >> 16;
	cmdo[4] = cmdarg >> 8;
	cmdo[5] = cmdarg;
	cmdo[6] = (crc7(0, &cmdo[1], 5) << 1) | 0x01;
	ret = spi_xfer(spi, sizeof(cmdo) * 8, cmdo, NULL, SPI_XFER_BEGIN);
	if (ret)
		return ret;

	ret = spi_xfer(spi, 1 * 8, NULL, &r, 0);
	if (ret)
		return ret;

	if (!resp || !resp_size)
		return 0;

	debug("%s: cmd%d", __func__, cmdidx);

	if (resp_match) {
		r = ~resp_match_value;
		i = CMD_TIMEOUT;
		while (i) {
			ret = spi_xfer(spi, 1 * 8, NULL, &r, 0);
			if (ret)
				return ret;
			debug(" resp%d=0x%x", rpos, r);
			rpos++;
			i--;

			if (r == resp_match_value)
				break;
		}
		if (!i && (r != resp_match_value))
			return -ETIMEDOUT;
	}

	for (i = 0; i < resp_size; i++) {
		if (i == 0 && resp_match) {
			resp[i] = resp_match_value;
			continue;
		}
		ret = spi_xfer(spi, 1 * 8, NULL, &r, 0);
		if (ret)
			return ret;
		debug(" resp%d=0x%x", rpos, r);
		rpos++;
		resp[i] = r;
	}

	if (r1b == true) {
		i = R1B_TIMEOUT;
		while (i) {
			ret = spi_xfer(spi, 1 * 8, NULL, &r, 0);
			if (ret)
				return ret;

			debug(" resp%d=0x%x", rpos, r);
			rpos++;
			i--;

			if (r)
				break;
		}
		if (!i)
			return -ETIMEDOUT;
	}

	debug("\n");

	return 0;
}

static int mmc_spi_readdata(struct mmc *mmc,
			    void *xbuf, u32 bcnt, u32 bsize)
{
	struct spi_slave *spi = mmc->priv;
	u16 crc;
	u8 *buf = xbuf, r1;
	int i, ret = 0;

	while (bcnt--) {
		for (i = 0; i < READ_TIMEOUT; i++) {
			ret = spi_xfer(spi, 1 * 8, NULL, &r1, 0);
			if (ret)
				return ret;
			if (r1 == SPI_TOKEN_SINGLE)
				break;
		}
		debug("%s: data tok%d 0x%x\n", __func__, i, r1);
		if (r1 == SPI_TOKEN_SINGLE) {
			ret = spi_xfer(spi, bsize * 8, NULL, buf, 0);
			if (ret)
				return ret;
			ret = spi_xfer(spi, 2 * 8, NULL, &crc, 0);
			if (ret)
				return ret;
#ifdef CONFIG_MMC_SPI_CRC_ON
			if (be16_to_cpu(crc16_ccitt(0, buf, bsize)) != crc) {
				debug("%s: data crc error\n", __func__);
				r1 = R1_SPI_COM_CRC;
				break;
			}
#endif
			r1 = 0;
		} else {
			r1 = R1_SPI_ERROR;
			break;
		}
		buf += bsize;
	}

	if (r1 & R1_SPI_COM_CRC)
		ret = -ECOMM;
	else if (r1) /* other errors */
		ret = -ETIMEDOUT;

	return ret;
}

static int mmc_spi_writedata(struct mmc *mmc, const void *xbuf,
			     u32 bcnt, u32 bsize, int multi)
{
	struct spi_slave *spi = mmc->priv;
	const u8 *buf = xbuf;
	u8 r1, tok[2];
	u16 crc;
	int i, ret = 0;

	tok[0] = 0xff;
	tok[1] = multi ? SPI_TOKEN_MULTI_WRITE : SPI_TOKEN_SINGLE;

	while (bcnt--) {
#ifdef CONFIG_MMC_SPI_CRC_ON
		crc = cpu_to_be16(crc16_ccitt(0, (u8 *)buf, bsize));
#endif
		spi_xfer(spi, 2 * 8, tok, NULL, 0);
		spi_xfer(spi, bsize * 8, buf, NULL, 0);
		spi_xfer(spi, 2 * 8, &crc, NULL, 0);
		for (i = 0; i < CMD_TIMEOUT; i++) {
			spi_xfer(spi, 1 * 8, NULL, &r1, 0);
			if ((r1 & 0x10) == 0) /* response token */
				break;
		}
		debug("%s: data tok%d 0x%x\n", __func__, i, r1);
		if (SPI_MMC_RESPONSE_CODE(r1) == SPI_RESPONSE_ACCEPTED) {
			debug("%s: data accepted\n", __func__);
			for (i = 0; i < WRITE_TIMEOUT; i++) { /* wait busy */
				spi_xfer(spi, 1 * 8, NULL, &r1, 0);
				if (i && r1 == 0xff) {
					r1 = 0;
					break;
				}
			}
			if (i == WRITE_TIMEOUT) {
				debug("%s: data write timeout 0x%x\n",
				      __func__, r1);
				r1 = R1_SPI_ERROR;
				break;
			}
		} else {
			debug("%s: data error 0x%x\n", __func__, r1);
			r1 = R1_SPI_COM_CRC;
			break;
		}
		buf += bsize;
	}
	if (multi && bcnt == -1) { /* stop multi write */
		tok[1] = SPI_TOKEN_STOP_TRAN;
		spi_xfer(spi, 2 * 8, tok, NULL, 0);
		for (i = 0; i < WRITE_TIMEOUT; i++) { /* wait busy */
			spi_xfer(spi, 1 * 8, NULL, &r1, 0);
			if (i && r1 == 0xff) {
				r1 = 0;
				break;
			}
		}
		if (i == WRITE_TIMEOUT) {
			debug("%s: data write timeout 0x%x\n", __func__, r1);
			r1 = R1_SPI_ERROR;
		}
	}

	if (r1 & R1_SPI_COM_CRC)
		ret = -ECOMM;
	else if (r1) /* other errors */
		ret = -ETIMEDOUT;

	return ret;
}

static int mmc_spi_set_ios(struct mmc *mmc)
{
	return 0;
}

static int mmc_spi_request(struct mmc *mmc, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct spi_slave *spi = mmc->priv;
	int i, multi, ret = 0;
	u8 *resp = NULL;
	u32 resp_size = 0;
	bool resp_match = false, r1b = false;
	u8 resp8 = 0, resp16[2] = { 0 }, resp40[5] = { 0 }, resp_match_value = 0;

	spi_claim_bus(spi);

	for (i = 0; i < 4; i++)
		cmd->response[i] = 0;

	switch (cmd->cmdidx) {
	case SD_CMD_APP_SEND_OP_COND:
	case MMC_CMD_SEND_OP_COND:
		resp = &resp8;
		resp_size = sizeof(resp8);
		cmd->cmdarg = 0x40000000;
		break;
	case SD_CMD_SEND_IF_COND:
		resp = (u8 *)&resp40[0];
		resp_size = sizeof(resp40);
		resp_match = true;
		resp_match_value = R1_SPI_IDLE;
		break;
	case MMC_CMD_SPI_READ_OCR:
		resp = (u8 *)&resp40[0];
		resp_size = sizeof(resp40);
		break;
	case MMC_CMD_SEND_STATUS:
		resp = (u8 *)&resp16[0];
		resp_size = sizeof(resp16);
		break;
	case MMC_CMD_SET_BLOCKLEN:
	case MMC_CMD_SPI_CRC_ON_OFF:
		resp = &resp8;
		resp_size = sizeof(resp8);
		resp_match = true;
		resp_match_value = 0x0;
		break;
	case MMC_CMD_STOP_TRANSMISSION:
	case MMC_CMD_ERASE:
		resp = &resp8;
		resp_size = sizeof(resp8);
		r1b = true;
		break;
	case MMC_CMD_SEND_CSD:
	case MMC_CMD_SEND_CID:
	case MMC_CMD_READ_SINGLE_BLOCK:
	case MMC_CMD_READ_MULTIPLE_BLOCK:
	case MMC_CMD_WRITE_SINGLE_BLOCK:
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
	case MMC_CMD_APP_CMD:
	case SD_CMD_ERASE_WR_BLK_START:
	case SD_CMD_ERASE_WR_BLK_END:
		resp = &resp8;
		resp_size = sizeof(resp8);
		break;
	default:
		resp = &resp8;
		resp_size = sizeof(resp8);
		resp_match = true;
		resp_match_value = R1_SPI_IDLE;
		break;
	};

	ret = mmc_spi_sendcmd(mmc, cmd->cmdidx, cmd->cmdarg, cmd->resp_type,
			      resp, resp_size, resp_match, resp_match_value, r1b);
	if (ret)
		goto done;

	switch (cmd->cmdidx) {
	case SD_CMD_APP_SEND_OP_COND:
	case MMC_CMD_SEND_OP_COND:
		cmd->response[0] = (resp8 & R1_SPI_IDLE) ? 0 : OCR_BUSY;
		break;
	case SD_CMD_SEND_IF_COND:
	case MMC_CMD_SPI_READ_OCR:
		cmd->response[0] = resp40[4];
		cmd->response[0] |= (uint)resp40[3] << 8;
		cmd->response[0] |= (uint)resp40[2] << 16;
		cmd->response[0] |= (uint)resp40[1] << 24;
		break;
	case MMC_CMD_SEND_STATUS:
		if (resp16[0] || resp16[1])
			cmd->response[0] = MMC_STATUS_ERROR;
		else
			cmd->response[0] = MMC_STATUS_RDY_FOR_DATA;
		break;
	case MMC_CMD_SEND_CID:
	case MMC_CMD_SEND_CSD:
		ret = mmc_spi_readdata(mmc, cmd->response, 1, 16);
		if (ret)
			return ret;
		for (i = 0; i < 4; i++)
			cmd->response[i] =
				cpu_to_be32(cmd->response[i]);
		break;
	default:
		cmd->response[0] = resp8;
		break;
	}

	debug("%s: cmd%d resp0=0x%x resp1=0x%x resp2=0x%x resp3=0x%x\n",
	      __func__, cmd->cmdidx, cmd->response[0], cmd->response[1],
	      cmd->response[2], cmd->response[3]);

	if (data) {
		debug("%s: data flags=0x%x blocks=%d block_size=%d\n",
		      __func__, data->flags, data->blocks, data->blocksize);
		multi = (cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK);
		if (data->flags == MMC_DATA_READ)
			ret = mmc_spi_readdata(mmc, data->dest,
					       data->blocks, data->blocksize);
		else if  (data->flags == MMC_DATA_WRITE)
			ret = mmc_spi_writedata(mmc, data->src,
						data->blocks, data->blocksize,
						multi);
	}

done:
	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	spi_release_bus(spi);

	return ret;
}

static int mmc_spi_init_p(struct mmc *mmc)
{
	struct spi_slave *spi = mmc->priv;
	spi_claim_bus(spi);
	/* cs deactivated for 100+ clock */
	spi_xfer(spi, 18 * 8, NULL, NULL, 0);
	spi_release_bus(spi);
	return 0;
}

static const struct mmc_ops mmc_spi_ops = {
	.send_cmd	= mmc_spi_request,
	.set_ios	= mmc_spi_set_ios,
	.init		= mmc_spi_init_p,
};

static struct mmc_config mmc_spi_cfg = {
	.name		= "MMC_SPI",
	.ops		= &mmc_spi_ops,
	.host_caps	= MMC_MODE_SPI,
	.voltages	= MMC_SPI_VOLTAGE,
	.f_min		= MMC_SPI_MIN_CLOCK,
	.part_type	= PART_TYPE_DOS,
	.b_max		= CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

struct mmc *mmc_spi_init(uint bus, uint cs, uint speed, uint mode)
{
	struct mmc *mmc;
	struct spi_slave *spi;

	spi = spi_setup_slave(bus, cs, speed, mode);
	if (spi == NULL)
		return NULL;

	mmc_spi_cfg.f_max = speed;

	mmc = mmc_create(&mmc_spi_cfg, spi);
	if (mmc == NULL) {
		spi_free_slave(spi);
		return NULL;
	}
	return mmc;
}
