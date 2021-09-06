#include "randombytes.c"
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static void *current_test = NULL;
static int syscall_called = 0;
static int glib_getrandom_called = 0;

// ======== Helper macros and functions ========

#define RUN_TEST(name) \
	printf("%s ... ", #name); \
	padto(' ', sizeof(#name) + sizeof(" ... "), 32); \
	current_test = name; \
	name(); \
	printf("ok\n");
#define SKIP_TEST(name) \
	printf("%s ... ", #name); \
	padto(' ', sizeof(#name) + sizeof(" ... "), 32); \
	printf("skipped\n");

static void padto(const char c, const size_t curlen, const size_t len) {
	for (size_t i = curlen; i < len; i++) {
		putchar(c);
	}
}

// ======== Forward declarations needed for mocked functions ========

#if defined(__linux__) && defined(SYS_getrandom)
int __wrap_syscall(int n, char *buf, size_t buflen, int flags);
int __real_syscall(int n, char *buf, size_t buflen, int flags);
#endif /* defined(__linux__) && defined(SYS_getrandom) */

#if defined(__linux__) && !defined(SYS_getrandom)
int __wrap_ioctl(int fd, int code, int* ret);
int __real_ioctl(int fd, int code, int* ret);
#endif /* defined(__linux__) && !defined(SYS_getrandom) */

// ======== Test definitions ========

static void test_functional(void) {
	uint8_t buf1[20] = {}, buf2[sizeof(buf1)] = {};
	const int ret1 = randombytes(buf1, sizeof(buf1));
	const int ret2 = randombytes(buf2, sizeof(buf2));
	if (ret1 != 0 || ret2 != 0) {
		printf("error: %s\n", strerror(errno));
	}
	assert(ret1 == 0);
	assert(ret2 == 0);
	assert(memcmp(buf1, buf2, sizeof(buf1)) != 0);
}

static void test_empty(void) {
	const uint8_t zero[20] = {};
	uint8_t buf[sizeof(zero)] = {};
	const int ret = randombytes(buf, 0);
	assert(ret == 0);
	assert(memcmp(buf, zero, sizeof(zero)) == 0);
}

static void test_getrandom_syscall_partial(void) {
	syscall_called = 0;
	uint8_t buf[100] = {};
	const int ret = randombytes(buf, sizeof(buf));
	assert(ret == 0);
	assert(syscall_called >= 5);
	for (int i = 1; i < 5; i++) {
		assert(memcmp(&buf[0], &buf[20*i], 20) != 0);
	}
}

static void test_getrandom_syscall_interrupted(void) {
	syscall_called = 0;
	uint8_t zero[20] = {};
	uint8_t buf[sizeof(zero)] = {};
	const int ret = randombytes(buf, sizeof(buf));
	assert(ret == 0);
	assert(memcmp(buf, zero, 20) != 0);
}

static void test_getrandom_glib_partial(void) {
	glib_getrandom_called = 0;
	uint8_t buf[100] = {};
	const int ret = randombytes(buf, sizeof(buf));
	assert(ret == 0);
	assert(glib_getrandom_called >= 5);
	for (int i = 1; i < 5; i++) {
		assert(memcmp(&buf[0], &buf[20*i], 20) != 0);
	}
}

static void test_getrandom_glib_interrupted(void) {
	glib_getrandom_called = 0;
	uint8_t zero[20] = {};
	uint8_t buf[sizeof(zero)] = {};
	const int ret = randombytes(buf, sizeof(buf));
	assert(ret == 0);
	assert(memcmp(buf, zero, 20) != 0);
}

static void test_issue_17(void) {
	uint8_t buf1[20] = {}, buf2[sizeof(buf1)] = {};
	const int ret1 = randombytes(buf1, sizeof(buf1));
	const int ret2 = randombytes(buf2, sizeof(buf2));
	assert(ret1 == 0);
	assert(ret2 == 0);
	assert(memcmp(buf1, buf2, sizeof(buf1)) != 0);
}

static void test_issue_22(void) {
	uint8_t buf1[20] = {}, buf2[sizeof(buf1)] = {};
	const int ret1 = randombytes(buf1, sizeof(buf1));
	const int ret2 = randombytes(buf2, sizeof(buf2));
	assert(ret1 == 0);
	assert(ret2 == 0);
	assert(memcmp(buf1, buf2, sizeof(buf1)) != 0);
}

static void test_issue_33(void) {
	for (size_t idx = 0; idx < 100000; idx++) {
		uint8_t buf[20] = {};
		const int ret = randombytes(&buf, sizeof(buf));
		if (ret != 0) {
			printf("error: %s\n", strerror(errno));
		}
		assert(ret == 0);
	}
}

// ======== Mock OS functions to simulate uncommon behavior ========

#if defined(__linux__) && defined(__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC_MINOR__ > 24))
int __wrap_getrandom(char *buf, size_t buflen, int flags) {
	glib_getrandom_called++;
	if (current_test == test_getrandom_glib_partial) {
		// Fill only 16 bytes, the caller should retry
		const size_t current_buflen = buflen <= 16 ? buflen : 16;
		return __real_getrandom(buf, current_buflen, flags);
	} else if (current_test == test_getrandom_glib_interrupted) {
		if (glib_getrandom_called < 5) {
			errno = EINTR;
			return -1;
		}
	}
	return __real_getrandom(buf, buflen, flags);
}

#elif defined(__linux__) && defined(SYS_getrandom)
int __wrap_syscall(int n, char *buf, size_t buflen, int flags) {
	syscall_called++;
	if (current_test == test_getrandom_syscall_partial) {
		// Fill only 16 bytes, the caller should retry
		const size_t current_buflen = buflen <= 16 ? buflen : 16;
		return __real_syscall(n, buf, current_buflen, flags);
	} else if (current_test == test_getrandom_syscall_interrupted) {
		if (syscall_called < 5) {
			errno = EINTR;
			return -1;
		}
	}
	return __real_syscall(n, buf, buflen, flags);
}
#endif /* defined(__linux__) && (defined(SYS_getrandom) or glibc version > 2.24) */

#if defined(__linux__) && !defined(SYS_getrandom)
int __wrap_ioctl(int fd, int code, int* ret) {
	if (current_test == test_issue_17) {
		errno = ENOTTY;
		return -1;
	}
	if (current_test == test_issue_17) {
		errno = ENOSYS;
		return -1;
	}
	return __real_ioctl(fd, code, ret);
}
#endif /* defined(__linux__) && !defined(SYS_getrandom) */

// ======== Main function ========

int main(void) {
	// Use `#if defined()` to enable/disable tests on a platform. If disabled,
	// please still call `SKIP_TEST` to make sure no tests are skipped silently.

	RUN_TEST(test_functional)
	RUN_TEST(test_empty)
#if defined(__linux__) && defined(USE_GLIBC)
	RUN_TEST(test_getrandom_glib_partial)
	RUN_TEST(test_getrandom_glib_interrupted)
	SKIP_TEST(test_getrandom_syscall_partial)
	SKIP_TEST(test_getrandom_syscall_interrupted)
#elif defined(__linux__) && defined(SYS_getrandom)
	SKIP_TEST(test_getrandom_glib_partial)
	SKIP_TEST(test_getrandom_glib_interrupted)
	RUN_TEST(test_getrandom_syscall_partial)
	RUN_TEST(test_getrandom_syscall_interrupted)
#else
	SKIP_TEST(test_getrandom_glib_partial)
	SKIP_TEST(test_getrandom_glib_interrupted)
	SKIP_TEST(test_getrandom_syscall_partial)
	SKIP_TEST(test_getrandom_syscall_interrupted)
#endif /* defined(__linux__) && (defined(SYS_getrandom) or glibc version > 2.24) */
#if defined(__linux__) && !defined(SYS_getrandom)
	RUN_TEST(test_issue_17)
	RUN_TEST(test_issue_22)
	RUN_TEST(test_issue_33)
#else
	SKIP_TEST(test_issue_17)
	SKIP_TEST(test_issue_22)
	SKIP_TEST(test_issue_33)
#endif /* defined(__linux__) && !defined(SYS_getrandom) */
	return 0;
}
