#include "marvell_api.h"

void time(time_t *c_time)
{
	*c_time = wmtime_time_get_posix();
}

/*ssize_t writev(int d, const struct iovec *iov, int iovcnt)
{
    ssize_t ret;
    size_t tot = 0;
    int i;
    char *buf, *p;

    for(i = 0; i < iovcnt; ++i)
	tot += iov[i].iov_len;
    buf = malloc(tot);
    if (tot != 0 && buf == NULL) {
	errno = ENOMEM;
	return -1;
    }
    p = buf;
    for (i = 0; i < iovcnt; ++i) {
	memcpy (p, iov[i].iov_base, iov[i].iov_len);
	p += iov[i].iov_len;
    }
    ret = write (d, buf, tot);
    free (buf);
    return ret;
}*/


