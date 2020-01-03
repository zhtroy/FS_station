/*
 * nandflash.h
 *
 *  Created on: 2019-12-18
 *      Author: DELL
 */

#ifndef NANDFLASH_H_
#define NANDFLASH_H_

#include "nandlib.h"
#include "stdint.h"
#include "stdio.h"

// 定义 NAND 数据线位宽，片选，页大小，块大小，设备ID
#define NAND_DATA_XFER_MODE                     (NAND_XFER_MODE_CPU)
#define NAND_BUSWIDTH                           (NAND_BUSWIDTH_16BIT)
#define NAND_CHIP_SELECT                        (EMIFA_CHIP_SELECT_3)
#define NAND_PAGE_SIZE_IN_BYTES                 (NAND_PAGESIZE_2048BYTES)
#define NAND_BLOCK_SIZE_IN_BYTES                (NAND_BLOCKSIZE_128KB)
#define NAND_NUMOF_BLK                          (4096)
#define NAND_MANUFATURER_MICRON_ID              (0x2C)
#define NAND_DEVICE_ID                          (0xCC)

// 定义默认读写的块，页和页数
#define NAND_DEFAULT_START_PAGE                 (5)
#define NAND_DEFAULT_BLK                        (8)
#define NAND_DEFAULT_NMBR_OF_PAGES              (1)

// 定义数据校验状态标志
#define NAND_DATA_INTEGRITY_PASS                (0)
#define NAND_DATA_INTEGRITY_FAIL                (1)

/* 定义 NAND 的时序信息 */
// 最大外部等待周期
/* Setup,strobe,hold times for read/write for the dev MT29F4G08AAA  */
#define NAND_WRITE_SETUP_TIME_IN_NS             (0u)
#define NAND_WRITE_STROBE_TIME_IN_NS            (30u)
#define NAND_WRITE_HOLD_TIME_IN_NS              (30u)
#define NAND_READ_SETUP_TIME_IN_NS              (20u)
#define NAND_READ_STROBE_TIME_IN_NS             (40u)
#define NAND_READ_HOLD_TIME_IN_NS               (0u)
#define NAND_TURN_ARND_TIME_IN_NS               (0u)

/* Setup,strobe,hold times reset values                             */
#define EMIFA_WRITE_SETUP_RESETVAL              (0x0F)
#define EMIFA_WRITE_STROBE_RESETVAL             (0x3F)
#define EMIFA_WRITE_HOLD_RESETVAL               (0x07)
#define EMIFA_READ_SETUP_RESETVAL               (0x0F)
#define EMIFA_READ_STROBE_RESETVAL              (0x3F)
#define EMIFA_READ_HOLD_RESETVAL                (0x07)
#define EMIFA_TA_RESETVAL                       (0x03)

/* NAND Module clk frequency                                        */
#define NAND_MODULE_CLK                         ((100u)*(1000u)*(1000u))
#define NAND_MODULE_CLK_IN_MHZ                  (NAND_MODULE_CLK / 1000000)

#define NAND_DATA_BUFF_SIZE                     (NAND_PAGE_SIZE_IN_BYTES)
#define NAND_ECC_BUFF_SIZE                      ((NAND_PAGE_SIZE_IN_BYTES/NAND_BYTES_PER_TRNFS) \
                                                    * NAND_MAX_ECC_BYTES_PER_TRNFS)

void nand_init();
NandStatus_t nand_block_erase(uint32_t startBlock, uint32_t blockNums);
NandStatus_t nand_read_bytes(uint32_t addr, uint8_t *buf, size_t size);
NandStatus_t nand_write_bytes(uint32_t addr, uint8_t *buf, size_t size);
#endif /* NANDFLASH_H_ */
