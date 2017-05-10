#include "randombytes.h"
#include <assert.h>

#ifdef __linux__
# define _GNU_SOURCE
# include <linux/random.h>
# include <sys/syscall.h>
# include <unistd.h>
# include <errno.h>
# include <sys/ioctl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <poll.h>
#endif


#if defined (__linux__) && !defined (SYS_getrandom)
static int randombytes_linux_get_entropy_avail(int fd)
{
	return ioctl(fd, RNDGETENTCNT);
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
#endif /* __linux__ && !SYS_getrandom */


void randombytes(void *buf, size_t n)
{
#ifdef __linux__
# ifdef SYS_getrandom
#  pragma message "Using getrandom system call"
	/* Use getrandom system call */
	int tmp = syscall(SYS_getrandom, buf, n, 0);
	assert(tmp == n); /* Failure indicates a bug in the code */
# else
#  pragma message "Using /dev/urandom device"
	/* When we have enough entropy, we can read from /dev/urandom */
	int fd;
	ssize_t tmp;
	do {
		fd = open("/dev/urandom", O_RDONLY);
	} while (fd == -1 && errno == EINTR);
	assert(randombytes_linux_wait_for_entropy(fd) != -1);

	while (n > 0) {
		tmp = read(fd, buf, n);
		if (tmp == -1 && (errno == EAGAIN || errno == EINTR)) {
			continue;
		}
		assert(tmp != -1); /* Unrecoverable IO error */
	}
# endif
#else
# error "randombytes(...) is not supported on this platform"
#endif
}
