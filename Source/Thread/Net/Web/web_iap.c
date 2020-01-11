/*
 * web_iap.c
 *
 *  Created on: 2020-1-9
 *      Author: DELL
 */

/*
 * type:0-bootloader,1-user app
 */
#define LOG_TAG "web_iap"
#define LOG_LVL ELOG_LVL_INFO

#include "elog.h"
#include "easyflash.h"


#define BOOTLOADER_MAX_SIZE (1*BLOCK_SIZE)
#define BOOTLOADER_UPDATE_ADDR (1*BLOCK_SIZE)

#define USER_APP_UPDATE_ADDR (2*BLOCK_SIZE)
#define USER_APP_MAX_SIZE (80*BLOCK_SIZE)

int web_iap_update(uint8_t type, void *pbuf, int32_t size)
{
    uint32_t cur_size = 0;
    if(type == 0 && size > BOOTLOADER_MAX_SIZE)
    {
        log_e("BootLoader is too bigger.(size:%d,max_size:%d)",size,BOOTLOADER_MAX_SIZE);
        return -1;
    }
    else if(type == 1 && size > USER_APP_MAX_SIZE)
    {
        log_e("User Application is too bigger.(size:%d,max_size:%d)",size,USER_APP_MAX_SIZE);
        return -1;
    }

    log_i("-- start update ...");

    /*
     * copy file to backup area
     */

    if(EF_NO_ERR == ef_erase_bak_app(size))
    {
        log_i("-- Erase backup area success");
    }
    else
    {
        log_e("-- Erase backup area failed");
        return -1;
    }

    if(EF_NO_ERR == ef_write_data_to_bak(pbuf, size, &(cur_size), size))
    {
        log_i("-- Write backup success");
    }
    else
    {
        log_e("-- Write backup failed");
        return -1;
    }


    if(type == 0)
    {
        /*
         * update bootloader
         */
        if(EF_NO_ERR == ef_erase_bl(BOOTLOADER_UPDATE_ADDR,size))
        {
            log_i("-- Erase bootloader area success(%d)",size);
        }
        else
        {
            log_e("-- Erase bootloader area failed");
            return -1;
        }
        

        if(EF_NO_ERR == ef_copy_bl_from_bak(BOOTLOADER_UPDATE_ADDR,size))
        {
            log_i("-- write bootloader area success(%d)",size);
        }
        else
        {
            log_e("-- write bootloader area failed");
            return -1;
        }
    }
    else
    {
        /*
         * update bootloader
         */
        if(EF_NO_ERR == ef_erase_user_app(USER_APP_UPDATE_ADDR,size))
        {
            log_i("-- Erase application area success(%d)",size);
        }
        else
        {
            log_e("-- Erase application area failed");
            return -1;
        }
        

        if(EF_NO_ERR == ef_copy_app_from_bak(USER_APP_UPDATE_ADDR,size))
        {
            log_i("-- write application area success(%d)",size);
        }
        else
        {
            log_e("-- write application area failed");
            return -1;
        }
    }

    log_i("-- update done");
    return 0;
    
}
