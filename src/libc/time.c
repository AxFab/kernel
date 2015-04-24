#include <time.h>
#include <smkos/assert.h>
#include <smkos/compiler.h>
// #include <smkos/_spec.h>

static const char *const weekDayStrings[] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};


static const char *const shWeekDayStrings[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};


static const char *const shMonthStrings[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


static long long __yearToSecs(long long year, int *is_leap)
{
  if (year - 2ULL <= 136) {
    int y = year;
    int leaps = (y - 68) >> 2;

    if (!((y - 68) & 3)) {
      leaps--;

      if (is_leap) *is_leap = 1;
    } else if (is_leap) *is_leap = 0;

    return 31536000 * (y - 70) + 86400 * leaps;
  }

  int cycles, centuries, leaps, rem;

  if (!is_leap) is_leap = &(int) {
    0
  };

  cycles = (year - 100) / 400;

  rem = (year - 100) % 400;

  if (rem < 0) {
    cycles--;
    rem += 400;
  }

  if (!rem) {
    *is_leap = 1;
    centuries = 0;
    leaps = 0;
  } else {
    if (rem >= 200) {
      if (rem >= 300) centuries = 3, rem -= 300;
      else centuries = 2, rem -= 200;
    } else {
      if (rem >= 100) centuries = 1, rem -= 100;
      else centuries = 0;
    }

    if (!rem) {
      *is_leap = 0;
      leaps = 0;
    } else {
      leaps = rem / 4U;
      rem %= 4U;
      *is_leap = !rem;
    }
  }

  leaps += 97 * cycles + 24 * centuries - *is_leap;

  return (year - 100) * 31536000LL + leaps * 86400LL + 946684800 + 86400;
}


static int __monthToSecs(int month, int is_leap)
{
  static const int secs_through_month[] = {
    0, 31 * 86400, 59 * 86400, 90 * 86400,
    120 * 86400, 151 * 86400, 181 * 86400, 212 * 86400,
    243 * 86400, 273 * 86400, 304 * 86400, 334 * 86400
  };
  int t = secs_through_month[month];

  if (is_leap && month >= 2) t += 86400;

  return t;
}


static long long __tmToSecs(const struct tm *tm)
{
  int is_leap;
  long long year = tm->tm_year;
  int month = tm->tm_mon;

  if (month >= 12 || month < 0) {
    int adj = month / 12;
    month %= 12;

    if (month < 0) {
      adj--;
      month += 12;
    }

    year += adj;
  }

  long long t = __yearToSecs(year, &is_leap);
  t += __monthToSecs(month, is_leap);
  t += 86400LL * (tm->tm_mday - 1);
  t += 3600LL * tm->tm_hour;
  t += 60LL * tm->tm_min;
  t += tm->tm_sec;
  return t;
}


// FIXME use local
char *asctime_r(const struct tm *restrict date, char *restrict str)
{
  assert(date->tm_wday >= 0 && date->tm_wday < 7);
  assert(date->tm_mon >= 0 && date->tm_mon < 12);
  snprintf(str, 26, "%.3s %.3s %3d %.2d:%.2d:%.2d %d\n",
           shWeekDayStrings[date->tm_wday], shMonthStrings[date->tm_mon],
           date->tm_mday, date->tm_hour,
           date->tm_min, date->tm_sec,
           1900 + date->tm_year);
  return str;
}


char *asctime(const struct tm *restrict date)
{
  static char buf[30];
  return asctime_r(date, buf);
}

