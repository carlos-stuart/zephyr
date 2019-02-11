#include <sys/time.h>
#include <zephyr.h>

int _gettimeofday(struct timeval *tv, void *tzvp) {
	u64_t t = k_uptime_get();
	tv->tv_sec = t / 1000000000;
	tv->tv_usec = (t % 1000000000) / 1000;
	return 0;
}
