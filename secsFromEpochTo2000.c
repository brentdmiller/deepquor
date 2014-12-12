#include <stdio.h>
#include <time.h>

main() {
  struct tm t;
  time_t rval;


  t.tm_sec = 0;         /* seconds */
  t.tm_min = 0;         /* minutes */
  t.tm_hour = 0;        /* hours */
  t.tm_mday = 1;        /* day of the month */
  t.tm_mon  = 0;         /* month */
  t.tm_year = 2000 - 1900;        /* year */
  t.tm_wday = 0;        /* day of the week */
  t.tm_yday = 0;        /* day in the year */
  t.tm_isdst= -1;       /* daylight saving time */

  rval = mktime(&t);

  printf("seconds between epoch & 1/1/2000: %d\n", rval);
}
