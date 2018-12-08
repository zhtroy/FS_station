/*
 * Watch_fun.c
 *
 *  Created on: 2018年12月5日
 *      Author: zhtro
 */


#include "Watch_fun.h"
#include "stdio.h"


void initTime(timedate_t * pt)
{
	pt->sec = 0;
	pt->min = 0;
	pt->hour = 0;
	pt->day = 1;
	pt->month =1;
}
void tickTime(timedate_t * pt){
  static int const month[] = {
     31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
  };
  if (++pt->sec == 60) {
    pt->sec = 0;
    if (++pt->min == 60) {
      pt->min = 0;
      if (++pt->hour == 24) {
        pt->hour = 0;
        if (++pt->day == month[pt->month-1]+1) {
          pt->day = 1;
          if (pt->month == 13)
           pt->month = 1;
        }
      }
    }
  }
}

void showTime(timedate_t * pt)
{

	printf("Time\t%d:%d:%d\n",pt->hour, pt->min, pt->sec);
}

void showDate(timedate_t * pt)
{

	printf("Date\t%d/%d\n",pt->month, pt->day);
}


