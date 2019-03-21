#ifndef __DEV_STATUS_H_
#define __DEV_STATUS_H_

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (1)
#endif

#ifndef NULL
#define NULL (0)
#endif

#define DEV_OK                      (0)
#define DEV_NO_INSTANCE             (-1)
#define DEV_CLOSED                  (-2)
#define DEV_NO_CONNECT              (-3)
#define DEV_OPENED                  (-4)
#define DEV_HAND_NULL               (-5)
#define DEV_NO_EXIST                (-6)
#define DEV_ENTER_RESET_FAIL        (-7)
#define DEV_REC_FIFO_EMPTY          (-8)
#define DEV_SEBD_TYPE_ERROR         (-9)
#define DEV_SEND_BUSY               (-10)
#define DEV_SEND_TIME_OUT           (-11)
#define DEV_WORK_MODE_NONSUPPORT    (-12)
#define DEV_BAND_RATE_NONSUPPORT    (-13)
#define DEV_EXIT_RESET_FAIL         (-14)
#define DEV_FILTER_PARAM_ERROR      (-15)


#endif
