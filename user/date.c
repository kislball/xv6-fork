#include "kernel/types.h"
#include "user/user.h"

#define BUF_SIZE 128
#define OUT_SIZE 16

int
is_leap(int year)
{
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int
hex_val(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

int
read_hex_from_stdin(int64 *out_ns)
{
  char buf[BUF_SIZE];
  int total = 0;
  int n;

  while (total < sizeof(buf) - 1 &&
         (n = read(0, buf + total, sizeof(buf) - 1 - total)) > 0) {
    total += n;
  }
  buf[total] = '\0';

  int64 ns = 0;
  int byte_idx = 0;
  int i = 0;

  while (buf[i] != '\0' && byte_idx < 8) {
    while (buf[i] == ' ') {
      i++;
    }
    if (buf[i] == '\0') {
      break;
    }

    int hi = hex_val(buf[i]);
    int lo = hex_val(buf[i + 1]);
    if (hi < 0 || lo < 0)
      return -1;

    uint64 byte = (hi << 4) | lo;
    ns |= (byte << (byte_idx * 8));
    byte_idx++;
    i += 2;
  }

  if (byte_idx != 8)
    return -1;

  *out_ns = ns;
  return 0;
}

void
print_padded(int64 val, int width)
{
  char out[OUT_SIZE];
  for (int i = width - 1; i >= 0; i--) {
    out[i] = '0' + (val % 10);
    val /= 10;
  }
  write(1, out, width);
}

void
print_datetime(int64 ns)
{
  int64 total_sec = ns / 1000000000LL;
  int64 rem_ns = ns % 1000000000LL;
  if (rem_ns < 0) {
    rem_ns += 1000000000LL;
    total_sec -= 1;
  }

  int64 days = total_sec / 86400;
  int64 seconds_of_day = total_sec % 86400;
  if (seconds_of_day < 0) {
    seconds_of_day += 86400;
    days -= 1;
  }

  int hour = seconds_of_day / 3600;
  int min = (seconds_of_day % 3600) / 60;
  int sec = seconds_of_day % 60;

  int year = 1970;
  if (days >= 0) {
    while (1) {
      int days_in_year = is_leap(year) ? 366 : 365;
      if (days >= days_in_year) {
        days -= days_in_year;
        year++;
      } else {
        break;
      }
    }
  } else {
    while (days < 0) {
      year--;
      days += is_leap(year) ? 366 : 365;
    }
  }

  static const int days_in_month[] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
  int month = 1;
  for (int m = 0; m < 12; m++) {
    int dim = days_in_month[m];
    if (m == 1 && is_leap(year)) {
      dim = 29;
    }
    if (days >= dim) {
      days -= dim;
      month++;
    } else {
      break;
    }
  }
  int day = days + 1;

  print_padded(day, 2);
  printf(".");
  print_padded(month, 2);
  printf(".");
  print_padded(year, 4);
  printf(" ");
  print_padded(hour, 2);
  printf(":");
  print_padded(min, 2);
  printf(":");
  print_padded(sec, 2);
  printf(".");
  print_padded(rem_ns, 9);
  printf("\n");
}

int
main(int argc, char *argv[])
{
  int64 ns = 0;

  if (read_hex_from_stdin(&ns) < 0) {
    fprintf(2, "date: invalid input\n");
    exit(1);
  }

  print_datetime(ns);
  exit(0);
}
