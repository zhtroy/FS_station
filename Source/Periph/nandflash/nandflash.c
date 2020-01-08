/*
 * nandflash.c
 *
 *  Created on: 2019-12-18
 *      Author: DELL
 */

#include "nandflash.h"
#include "emifa.h"
#include "nand_emifa2.h"
#include "nand_gpmc.h"
#include "soc_C6748.h"
#include "uartStdio.h"
#include "common.h"

/* 全局变量 */
#pragma DATA_ALIGN(txData, 4);
volatile unsigned char txData[NAND_DATA_BUFF_SIZE];
#pragma DATA_ALIGN(rxData, 4);
volatile unsigned char rxData[NAND_DATA_BUFF_SIZE];
#pragma DATA_ALIGN(eccData, 4);
volatile unsigned char eccData[NAND_ECC_BUFF_SIZE];

/* NAND的信息结构体申请 */
NandInfo_t              nandInfo;
NandCtrlInfo_t          nandCtrlInfo;
NandEccInfo_t           nandEccInfo;
NandDmaInfo_t           nandDmaInfo;
EMIFANANDTimingInfo_t   nandTimingInfo;

/****************************************************************************/
/*                                                                          */
/*             初始化 EMIF NAND 时序信息                                    */
/*                                                                          */
/****************************************************************************/
static void nand_timing_info_init(void *TimingInfo)
{
    int moduleClkInMHz = NAND_MODULE_CLK_IN_MHZ;
    EMIFANANDTimingInfo_t *nandTimingInfo;

    nandTimingInfo = (EMIFANANDTimingInfo_t * )TimingInfo;

    /* 设置异步等待时序 */
    nandTimingInfo->writeSetup  = (((moduleClkInMHz * NAND_WRITE_SETUP_TIME_IN_NS)/1000u) &
                                     EMIFA_WRITE_SETUP_RESETVAL);
    nandTimingInfo->writeStrobe = (((moduleClkInMHz * NAND_WRITE_STROBE_TIME_IN_NS)/1000u) &
                                    EMIFA_WRITE_STROBE_RESETVAL);
    nandTimingInfo->writeHold   = (((moduleClkInMHz * NAND_WRITE_HOLD_TIME_IN_NS)/1000u) &
                                    EMIFA_WRITE_HOLD_RESETVAL);

    nandTimingInfo->readSetup   = (((moduleClkInMHz * NAND_READ_SETUP_TIME_IN_NS)/1000u) &
                                    EMIFA_READ_SETUP_RESETVAL);
    nandTimingInfo->readStrobe  = (((moduleClkInMHz * NAND_READ_STROBE_TIME_IN_NS)/1000u) &
                                    EMIFA_READ_STROBE_RESETVAL);
    nandTimingInfo->readHold    = (((moduleClkInMHz * NAND_READ_HOLD_TIME_IN_NS)/1000u) &
                                    EMIFA_READ_HOLD_RESETVAL);

    nandTimingInfo->turnAround  = (((moduleClkInMHz * NAND_TURN_ARND_TIME_IN_NS)/1000u) &
                                    EMIFA_TA_RESETVAL);
}

/****************************************************************************/
/*                                                                          */
/*              初始化 NAND 的信息                                          */
/*                                                                          */
/****************************************************************************/
static void nand_info_init(NandInfo_t *nandInfo, unsigned int cs)
{
    NandCtrlInfo_t *hNandCtrlInfo = nandInfo->hNandCtrlInfo;
    NandDmaInfo_t  *hNandDmaInfo  = nandInfo->hNandDmaInfo;
    NandEccInfo_t  *hNandEccInfo  = nandInfo->hNandEccInfo;

    /* 初始化NAND设备信息 */
    nandInfo->opMode                        = NAND_DATA_XFER_MODE;
    //nandInfo->eccType                       = NAND_ECC_ALGO_RS_4BIT;
    nandInfo->eccType                       = NAND_ECC_ALGO_NONE;

    nandInfo->chipSelectCnt                 = 1;
    nandInfo->dieCnt                        = 1;
    nandInfo->chipSelects[0]                = cs;
    nandInfo->busWidth                      = NAND_BUSWIDTH;
    nandInfo->pageSize                      = NAND_PAGE_SIZE_IN_BYTES;
    nandInfo->blkSize                       = NAND_BLOCK_SIZE_IN_BYTES;
    nandInfo->manId                         = NAND_MANUFATURER_MICRON_ID;
    nandInfo->devId                         = NAND_DEVICE_ID;

    nandInfo->dataRegAddr                   = (SOC_EMIFA_CS3_ADDR + 0x00);
    nandInfo->addrRegAddr                   = (SOC_EMIFA_CS3_ADDR + 0x08);
    nandInfo->cmdRegAddr                    = (SOC_EMIFA_CS3_ADDR + 0x10);

    /* 初始化NAND控制器信息 */
    hNandCtrlInfo->CtrlInit                 = EMIFANANDInit;
    hNandCtrlInfo->WaitPinStatusGet         = EMIFANANDWaitPinStatusGet;
    hNandCtrlInfo->currChipSelect           = cs;
    hNandCtrlInfo->baseAddr                 = SOC_EMIFA_0_REGS;
    //hNandCtrlInfo->eccSupported             = ( NAND_ECC_ALGO_HAMMING_1BIT |
    //                                            NAND_ECC_ALGO_RS_4BIT );
    hNandCtrlInfo->eccSupported             = NAND_ECC_ALGO_NONE;
    hNandCtrlInfo->waitPin                  = EMIFA_EMA_WAIT_PIN0;
    hNandCtrlInfo->waitPinPol               = EMIFA_EMA_WAIT_PIN_POLARITY_HIGH;
    hNandCtrlInfo->wpPinPol                 = 0;
    hNandCtrlInfo->chipSelectBaseAddr[0]    = SOC_EMIFA_CS3_ADDR;
    hNandCtrlInfo->chipSelectRegionSize[0]  = EMIFA_CHIP_SELECT_3_SIZE;
    nand_timing_info_init(hNandCtrlInfo->hNandTimingInfo);

    /* 初始化NAND ECC信息 */
    hNandEccInfo->baseAddr                  = 0;
    hNandEccInfo->ECCInit                   = EMIFANANDECCInit;
    hNandEccInfo->ECCEnable                 = EMIFANANDECCEnable;
    hNandEccInfo->ECCDisable                = EMIFANANDECCDisable;
    hNandEccInfo->ECCWriteSet               = EMIFANANDECCWriteSet;
    hNandEccInfo->ECCReadSet                = EMIFANANDECCReadSet;
    hNandEccInfo->ECCCalculate              = EMIFANANDECCCalculate;
    hNandEccInfo->ECCCheckAndCorrect        = EMIFANANDECCCheckAndCorrect;

    /* 初始化NAND DMA信息 */
    hNandDmaInfo->DMAXfer                   = NULL;
    hNandDmaInfo->DMAInit                   = NULL;
    hNandDmaInfo->DMAXferSetup              = NULL;
    hNandDmaInfo->DMAXferStatusGet          = NULL;
}

void nand_init()
{
    uint32_t i;
    NandStatus_t retVal;
    NANDPinMuxSetup();

    /* 初始化NAND信息结构体 */
    nandCtrlInfo.hNandTimingInfo = (void *) &nandTimingInfo;
    nandInfo.hNandCtrlInfo = &nandCtrlInfo;
    nandInfo.hNandEccInfo = &nandEccInfo;
    nandInfo.hNandDmaInfo = &nandDmaInfo;
    nand_info_init(&nandInfo, NAND_CHIP_SELECT);

    for(i=0;i<NAND_ECC_BUFF_SIZE;i++)
    {
        eccData[i] = 0x00;
    }

    /* 打开NAND设备 */
    retVal = NANDOpen(&nandInfo);
    if (retVal & NAND_STATUS_FAILED)
    {
        sb_puts("\r\n*** ERROR : NAND Open Failed... ",-1);
        while(1);
    }
    else if (retVal & NAND_STATUS_WAITTIMEOUT)
    {
        sb_puts("\r\n*** ERROR : Device Is Not Ready...!!!\r\n", -1);
        while(1);
    }
    else if (retVal & NAND_STATUS_NOT_FOUND)
    {
        sb_puts("\r\n*** ERROR : DEVICE MAY NOT BE ACCESSABLE OR NOT PRESENT."
                 "\r\n", -1);
        while(1);
    }
    else if(nandInfo.devId != NAND_DEVICE_ID)
    {
        /* 检查NAND设备ID为不匹配 */
        sb_puts("\r\n*** ERROR : INVALID DEVICE ID.", -1);
        while(1);
    }
    else
    {
        sb_puts("\r\n*** NAND Open Success... ",-1);
    }
}

NandStatus_t nand_block_erase(uint32_t startBlock, uint32_t blockNums)
{
    int32_t i;
    NandStatus_t status = NAND_STATUS_PASSED;
    for(i=0;i<blockNums;i++)
    {
        status = NANDBlockErase(&nandInfo,startBlock+i);
        if(status != NAND_STATUS_PASSED)
        {
            break;
        }
    }

    return status;
}

NandStatus_t nand_read_bytes(uint32_t addr, uint8_t *buf, size_t size)
{
    NandStatus_t status = NAND_STATUS_PASSED;
    uint32_t memBufferPtr = 0;
    uint32_t bytesLeftInBuff = 0;
    uint32_t bytesToCopy = 0;
    uint32_t pageSize   = nandInfo.pageSize;
    uint32_t blockSize  = nandInfo.blkSize;
    /* Convert the flashAddr to block, page numbers */
    uint32_t blkNum = (addr / blockSize);
    uint32_t pageNum = (addr - (blkNum * blockSize)) / pageSize;

    uint32_t currBlock = blkNum;
    uint32_t currPage = pageNum;

    NANDBadBlockCheck(&nandInfo,currBlock);
    status = NANDPageRead( &nandInfo, currBlock, currPage,rxData, &eccData[0]);

    if(status != NAND_STATUS_PASSED)
    {
        return status;
    }


    /* Figure out offset in buffered page */
    memBufferPtr = addr - (currBlock * blockSize) - (currPage * pageSize);

    // Now we do the actual reading of bytes
    // If there are bytes in the memory buffer, use them first
    bytesLeftInBuff = NAND_DATA_BUFF_SIZE - memBufferPtr;
    if (bytesLeftInBuff > 0)
    {
        bytesToCopy = (bytesLeftInBuff >= size) ? size : bytesLeftInBuff;

        // Copy bytesToCopy bytes from current buffer pointer to the dest
        memcpy((void *)buf, (void *)&rxData[memBufferPtr], bytesToCopy);
        buf  += bytesToCopy;
        size -= bytesToCopy;
        addr += bytesToCopy;

        bytesLeftInBuff -= bytesToCopy;
    }

    // If we have one or more full blocks to copy, copy them directly
    // Any leftover data (partial page) gets copied via the memory buffer
    while (size > 0)
    {
        unsigned char *tempPtr = NULL;
        currPage += 1;

        // Check to see if curr page is at end of a block
        if (currPage >= nandInfo.pagesPerBlk)
        {
            currPage  = 0;
            currBlock++;
        }

        // Read the new current page in the current block to its destination
        tempPtr = (unsigned char *)(size >= pageSize) ? buf : ((unsigned char *)rxData);
        bytesToCopy = (size >= pageSize) ? pageSize : size;

        status = NANDPageRead( &nandInfo, currBlock, currPage,
                               tempPtr, &eccData[0]);

        if(status != NAND_STATUS_PASSED)
        {
            return status;
        }


        if (tempPtr != buf)
        {
            // If the last copy was a partial page, copy byteCnt
            // bytes from memory buffer pointer to the dest
            memcpy((void *)buf, (void *)rxData, bytesToCopy);
        }

        size -= bytesToCopy;
        buf  += bytesToCopy;
        addr += bytesToCopy;
    }
    return status;
}


NandStatus_t nand_write_bytes(uint32_t addr, uint8_t *buf, size_t size)
{
    NandStatus_t status = NAND_STATUS_PASSED;
    uint32_t memBufferPtr = 0;
    uint32_t bytesLeftInBuff = 0;
    uint32_t bytesToCopy = 0;
    uint32_t pageSize   = nandInfo.pageSize;
    uint32_t blockSize  = nandInfo.blkSize;
    /* Convert the flashAddr to block, page numbers */
    uint32_t blkNum = (addr / blockSize);
    uint32_t pageNum = (addr - (blkNum * blockSize)) / pageSize;

    uint32_t currBlock = blkNum;
    uint32_t currPage = pageNum;

    status = NANDPageRead( &nandInfo, currBlock, currPage,txData, &eccData[0]);
    /*
    if(status != NAND_STATUS_PASSED)
    {
        return status;
    }
    */

    /* Figure out offset in buffered page */
    memBufferPtr = addr - (currBlock * blockSize) - (currPage * pageSize);

    // Now we do the actual reading of bytes
    // If there are bytes in the memory buffer, use them first
    bytesLeftInBuff = NAND_DATA_BUFF_SIZE - memBufferPtr;
    if (bytesLeftInBuff > 0)
    {
        bytesToCopy = (bytesLeftInBuff >= size) ? size : bytesLeftInBuff;

        // Copy bytesToCopy bytes from current buffer pointer to the dest
        memcpy((void *)&txData[memBufferPtr], (void *)buf,  bytesToCopy);

        status = NANDPageWrite( &nandInfo, currBlock, currPage,txData, &eccData[0]);
        if(status != NAND_STATUS_PASSED)
        {
            return status;
        }
        buf  += bytesToCopy;
        size -= bytesToCopy;
        addr += bytesToCopy;

        bytesLeftInBuff -= bytesToCopy;
    }

    // If we have one or more full blocks to copy, copy them directly
    // Any leftover data (partial page) gets copied via the memory buffer
    while (size > 0)
    {
        unsigned char *tempPtr = NULL;
        currPage += 1;

        // Check to see if curr page is at end of a block
        if (currPage >= nandInfo.pagesPerBlk)
        {
            currPage  = 0;
            currBlock++;
        }

        // Read the new current page in the current block to its destination
        tempPtr = (unsigned char *)(size >= pageSize) ? buf : ((unsigned char *)txData);
        bytesToCopy = (size >= pageSize) ? pageSize : size;

        if (tempPtr != buf)
        {
            // If the last copy was a partial page, copy byteCnt
            // bytes from memory buffer pointer to the dest
            status = NANDPageRead( &nandInfo, currBlock, currPage,txData, &eccData[0]);
            /*
            if(status != NAND_STATUS_PASSED)
            {
                return status;
            }
            */
            memcpy((void *)txData, (void *)buf, bytesToCopy);
        }

        status = NANDPageWrite( &nandInfo, currBlock, currPage,
                               tempPtr, &eccData[0]);
        if(status != NAND_STATUS_PASSED)
        {
            return status;
        }

        size -= bytesToCopy;
        buf  += bytesToCopy;
        addr += bytesToCopy;
    }
    return status;
}
