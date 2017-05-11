#include "randombytes.h"

#if defined(__linux__)
/* Linux */
# define _GNU_SOURCE
# include <assert.h>
# include <errno.h>
# include <fcntl.h>
# include <linux/random.h>
# include <poll.h>
# include <sys/ioctl.h>
# include <sys/stat.h>
# include <sys/syscall.h>
# include <sys/types.h>
# include <unistd.h>
#endif /* defined(__linux__) */


#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
/* Dragonfly, FreeBSD, NetBSD, OpenBSD (has arc4random) */
# include <sys/param.h>
# if defined(BSD)
#  include <stdlib.h>
# endif
#endif


#if defined(__linux__) && defined(SYS_getrandom)
static void randombytes_linux_randombytes_getrandom(void *buf, size_t n)
{
	/* I have thought about using a separate PRF, seeded by getrandom, but
	 * it turns out that the performance of getrandom is good enough
	 * (250 MB/s on my laptop).
	 */
	size_t offset = 0, chunk;
	int ret;
	while (n > 0) {
		/* getrandom does not allow chunks larger than 33554431 */
		chunk = n <= 33554431 ? n : 33554431;
		do {
			ret = syscall(SYS_getrandom, buf + offset, chunk, 0);
		} while (ret == -1 && errno == EINTR);
		assert(ret != -1);
		offset += ret;
		n -= ret;
	}
	assert(n == 0);
}
#endif /* defined(__linux__) && defined(SYS_getrandom) */


#if defined(__linux__) && !defined(SYS_getrandom)
static int randombytes_linux_get_entropy_avail(int fd)
{
	int ret;
	ioctl(fd, RNDGETENTCNT, &ret);
	return ret;
}


static int randombytes_linux_wait_for_entropy(int device)
{
	/* We will block on /dev/random, because any increase in the OS' entropy
	 * level will unblock the request. I use poll here (as does libsodium),
	 * because we don't *actually* want to read from the device. */
	const int bits = 128;
	struct pollfd pfd;
	int fd, retcode; /* Used as file descriptor *and* poll() return code */

	/* If the device has enough entropy already, we will want to return early */
	if (randombytes_linux_get_entropy_avail(device) >= bits) {
		return 0;
	}

	do {
		fd = open("/dev/random", O_RDONLY);
	} while (fd == -1 && errno == EINTR); /* EAGAIN will not occur */
	if (fd == -1) {
		/* Impossible to recover without returning something other than
		 * void and while not compromising any security. If opening
		 * `/dev/random` is impossible, this probably indicates a bug in
		 * the code (or you might * have bigger problems on your hands). */
		return -1;
	}

	pfd.fd = fd;
	pfd.events = POLLIN;
	do {
		retcode = poll(&pfd, 1, -1);
	} while ((retcode == -1 && (errno == EINTR || errno == EAGAIN)) ||
	         randombytes_linux_get_entropy_avail(device) < bits);
	if (retcode != 1) {
		do {
			retcode = close(fd);
		} while (retcode == -1 && errno == EINTR);
		return -1;
	}
	return close(fd);
}


static void randombytes_linux_randombytes_urandom(void *buf, size_t n)
{
	int fd;
	size_t offset = 0;
	ssize_t tmp;
	do {
		fd = open("/dev/urandom", O_RDONLY);
	} while (fd == -1 && errno == EINTR);
	assert(randombytes_linux_wait_for_entropy(fd) != -1);

	assert(n <= SSIZE_MAX);
	while (n > 0) {
		tmp = read(fd, buf + offset, n);
		if (tmp == -1 && (errno == EAGAIN || errno == EINTR)) {
			continue;
		}
		assert(tmp != -1); /* Unrecoverable IO error */
		offset += tmp;
		n -= tmp;
	}
	assert(n == 0);
}
#endif /* defined(__linux__) && !defined(SYS_getrandom) */


#if defined(BSD)
static void randombytes_bsd_randombytes(void *buf, size_t n)
{
	arc4random_buf(buf, n);
}
#endif /* defined(BSD) */


void randombytes(void *buf, size_t n)
{
#if defined(__linux__)
# if defined(SYS_getrandom)
#  pragma message "Using getrandom system call"
	/* Use getrandom system call */
	randombytes_linux_randombytes_getrandom(buf, n);
# else
#  pragma message "Using /dev/urandom device"
	/* When we have enough entropy, we can read from /dev/urandom */
	randombytes_linux_randombytes_urandom(buf, n);
# endif
#elif defined(BSD)
# pragma message "Using arc4random system call"
	/* Use arc4random system call */
	randombytes_bsd_randombytes(buf, n);
#else
# error "randombytes(...) is not supported on this platform"
#endif
}
