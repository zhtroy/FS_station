/*
 * user_finsh_cmd.c
 *
 *  Created on: 2013��12��7��
 *      Author: Armink
 */

#include "shell.h"
#include "elog_flash.h"
#include "easyflash.h"
#include <string.h>
#include "common.h"
#include <stdlib.h>
#include <ti/sysbios/knl/Task.h>

void setenv(uint8_t argc, char **argv) {
    uint8_t i;
    char c_value = NULL;
    char *value = &c_value;
    if (argc > 3) {
        /* environment variable value string together */
        for (i = 0; i < argc - 2; i++) {
            argv[2 + i][strlen(argv[2 + i])] = ' ';
        }
    }
    if (argc == 1) {
        ef_set_env(value, value);
    } else if (argc == 2) {
        ef_set_env(argv[1], value);
    } else {
        ef_set_env(argv[1], argv[2]);
    }
}
MSH_CMD_EXPORT_EX(setenv, Set an envrionment variable., .ex setenv temp 1000);

void printenv(uint8_t argc, char **argv) {
    ef_print_env();
}
MSH_CMD_EXPORT(printenv, Print all envrionment variables.);

void saveenv(uint8_t argc, char **argv) {
    ef_save_env();
}
MSH_CMD_EXPORT(saveenv, Save all envrionment variables to flash.);

void getvalue(uint8_t argc, char **argv) {
    char *value = NULL;
    value = ef_get_env(argv[1]);
    if (value) {
        sb_printf("The %s value is %s.\n", argv[1], value);
    } else {
        sb_printf("Can't find %s.\n", argv[1]);
    }
}
MSH_CMD_EXPORT(getvalue, Get an envrionment variable by name.);

static void elog(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (!strcmp(argv[1], "on") || !strcmp(argv[1], "ON")) {
            elog_set_output_enabled(true);
        } else if (!strcmp(argv[1], "off") || !strcmp(argv[1], "OFF")) {
            elog_set_output_enabled(false);
        } else {
            sb_printf("Please input elog on or elog off.\n");
        }
    } else {
        sb_printf("Please input elog on or elog off.\n");
    }
}
MSH_CMD_EXPORT(elog, EasyLogger output enabled [on/off]);

static void elog_lvl(uint8_t argc, char **argv) {
    if (argc > 1) {
        if ((atoi(argv[1]) <= ELOG_LVL_VERBOSE) && (atoi(argv[1]) >= 0)) {
            elog_set_filter_lvl(atoi(argv[1]));
        } else {
            sb_printf("Please input correct level(0-5).\n");
        }
    } else {
        sb_printf("Please input level.\n");
    }
}
MSH_CMD_EXPORT(elog_lvl, Set EasyLogger filter level);

static void elog_tag(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (strlen(argv[1]) <= ELOG_FILTER_TAG_MAX_LEN) {
            elog_set_filter_tag(argv[1]);
        } else {
            sb_printf("The tag length is too long. Max is %d.\n", ELOG_FILTER_TAG_MAX_LEN);
        }
    } else {
        elog_set_filter_tag("");
    }
}
MSH_CMD_EXPORT(elog_tag, Set EasyLogger filter tag);

static void elog_kw(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (strlen(argv[1]) <= ELOG_FILTER_KW_MAX_LEN) {
            elog_set_filter_kw(argv[1]);
        } else {
            sb_printf("The keyword length is too long. Max is %d.\n", ELOG_FILTER_KW_MAX_LEN);
        }
    } else {
        elog_set_filter_kw("");
    }
}
MSH_CMD_EXPORT(elog_kw, Set EasyLogger filter keyword);

static void elog_flash(uint8_t argc, char **argv) {
    if (argc >= 2) {
        if (!strcmp(argv[1], "read")) {
            if (argc >= 3) {
                elog_flash_output_recent(atol(argv[2]));
            }else {
                elog_flash_output_recent(5000);
            }
        } else if (!strcmp(argv[1], "clean")) {
            elog_flash_clean();
        } else if (!strcmp(argv[1], "flush")) {

#ifdef ELOG_FLASH_USING_BUF_MODE
            elog_flash_flush();
#else
            sb_printf("EasyLogger flash log buffer mode is not open.\n");
#endif

        } else {
            sb_printf("Please input elog_flash {<read>, <clean>, <flush>}.\n");
        }
    } else {
        sb_printf("Please input elog_flash {<read>, <clean>, <flush>}.\n");
    }
}
MSH_CMD_EXPORT(elog_flash, EasyLogger <read> <clean> <flush> flash log);

#define LOG_BUF_SIZE (1024)
#define RETAIN_SIZE (64)
#define FIND_BEFORE_SIZE (1000)
#define FIND_AFTER_SIZE (1000)

static char buf[LOG_BUF_SIZE+RETAIN_SIZE+1];
static void elog_find(uint8_t argc, char **argv){
    size_t index;
    if (argc >= 2) {
        size_t log_total_size = ef_log_get_used_size();

        if(argc == 3)
            index = atoi(argv[2]);
        else
            index = log_total_size;

        char *res;
        size_t res_len,res_pos;
        /*set "\0" for string end*/
        memset(buf," ",LOG_BUF_SIZE+RETAIN_SIZE);
        buf[LOG_BUF_SIZE+RETAIN_SIZE] = '\0';
        while(true) {

            /*Get retain log for next search*/
            memcpy(&buf[LOG_BUF_SIZE],buf,RETAIN_SIZE);
                
            if(index > LOG_BUF_SIZE) {
                index -= LOG_BUF_SIZE;
                ef_log_read(index,buf,LOG_BUF_SIZE);
            
                res = strstr(buf,argv[1]);
                if(res != NULL) {
                    res_len = strlen(res);
                    res_pos = index+LOG_BUF_SIZE+RETAIN_SIZE-res_len;
                    break;;
                }
            } else {
            
                ef_log_read(0,&buf[LOG_BUF_SIZE-index],index+4-(index%4));
                
                res = strstr(&buf[LOG_BUF_SIZE-index],argv[1]);
                if(res != NULL) {
                    res_len = strlen(res);
                    res_pos = index+RETAIN_SIZE-res_len;
                    break;
                } else {
                    sb_printf("cannot find keyword:%s\n",argv[1]);
                    return;
                }
            }
            Task_sleep(1);
        }

        sb_printf("keyword<%s>:%d,total:%d\n",argv[1],res_pos,log_total_size);
        if(res_pos > FIND_BEFORE_SIZE)
        {
        	if(log_total_size> res_pos+FIND_AFTER_SIZE+ 4)
            	elog_flash_output(res_pos-FIND_BEFORE_SIZE,FIND_AFTER_SIZE+FIND_BEFORE_SIZE);
        	else
            	elog_flash_output(res_pos-FIND_BEFORE_SIZE,log_total_size - res_pos - 4 + FIND_BEFORE_SIZE );
        }
        else
            elog_flash_output(0,FIND_AFTER_SIZE);
        
    } else {
        sb_printf("Please input elog_find {keyword} [start_point]. keyword cannot include blankspace\n");
    }
}
MSH_CMD_EXPORT(elog_find, EasyLogger find keyword from flash log);

static void elog_read(uint8_t argc, char **argv){
        if (argc == 2) {
            elog_flash_output_recent(atol(argv[1]));
        } else if (argc == 3) {
            elog_flash_output(atol(argv[1]),atol(argv[2]));
        } else {
            sb_printf("Please input [elog_read <position> <length>] or [elog_read <length>]\n");
        }
}
MSH_CMD_EXPORT(elog_read, EasyLogger read from flash log);
