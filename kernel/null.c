#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

enum nulldevice_kind_t {
    NULLDEVICE_NULL,
    NULLDEVICE_ZERO,
    NULLDEVICE_NULLSTAT,
    
    // add new types here, if needed

    NULLDEVICE_TOTAL
};

struct nullfilehandler {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

struct nullfilehandler handlers[NULLDEVICE_TOTAL] = {
    [NULLDEVICE_NULL] = {},
    [NULLDEVICE_ZERO] = {},
    [NULLDEVICE_NULLSTAT] = {}
};

int
nullwrite(int user_src, uint64 src, int n, short minor)
{
    return handlers[minor].write(user_src, src, n);
}

int
nullread(int user_dst, uint64 dst, int n, short minor)
{
    return handlers[minor].read(user_dst, dst, n);
}

void
nullinit(void)
{
    devsw[NULLDEVICES].write = nullwrite;
    devsw[NULLDEVICES].write = nullread;
}