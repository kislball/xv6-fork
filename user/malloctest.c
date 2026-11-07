#include "kernel/types.h"
#include "user/user.h"

static void
check(int ok, const char *message)
{
  if (!ok) {
    fprintf(2, "malloctest: FAIL: %s\n", message);
    exit(1);
  }
}

struct stringbuilder {
  char *data;
  uint len;
  uint capacity;
};

static void
append(struct stringbuilder *s, const char *text)
{
  uint len = strlen(text);
  if (s->len + len + 1 > s->capacity) {
    uint capacity = s->capacity ? s->capacity : 16;
    while (capacity < s->len + len + 1)
      capacity *= 2;
    char *data = malloc(capacity);
    check(data != 0, "allocation failed");
    if (s->len)
      memmove(data, s->data, s->len);
    free(s->data);
    s->data = data;
    s->capacity = capacity;
  }
  memmove(s->data + s->len, text, len + 1);
  s->len += len;
}

int
main(void)
{
  char *baseline = sbrk(0);
  struct stringbuilder s = {0};
  append(&s, "");
  check(strcmp(s.data, "") == 0, "empty string corrupted");
  append(
    &s,
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut");
  append(
    &s,
    " aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.");
  append(
    &s,
    " Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
  append(&s, "");
  const char *expected =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
  check(strcmp(s.data, expected) == 0, "built string corrupted");
  check(s.len == strlen(expected), "incorrect string length");
  printf("%s\n", s.data);
  free(s.data);
  check(sbrk(0) == baseline, "empty slabs not reclaimed");
  printf("malloctest: OK\n");
  exit(0);
}
