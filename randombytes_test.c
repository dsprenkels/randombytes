#include "randombytes.c"
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static void *current_test = NULL;

// ======== Helper macros and functions ========

#define RUN_TEST(name) \
	printf("%s ... ", #name); \
	padto(' ', sizeof(#name) + sizeof(" ... "), 24); \
	current_test = name; \
	name(); \
	printf("ok\n");
#define SKIP_TEST(name) \
	printf("%s ... ", #name); \
	padto(' ', sizeof(#name) + sizeof(" ... "), 24); \
	printf("skipped\n");

static void padto(const char c, const size_t curlen, const size_t len) {
	for (size_t i = curlen; i < len; i++) {
		putchar(c);
	}
}

// ======== Forward declarations needed for mocked functions ========

#if defined(__linux__) && !defined(SYS_getrandom)
int __wrap_ioctl(int fd, int code, int* ret);
int __real_ioctl(int fd, int code, int* ret);
#endif /* defined(__linux__) && !defined(SYS_getrandom) */

// ======== Test definitions ========

static void test_functional(void) {
	uint8_t buf1[20] = {}, buf2[sizeof(buf1)] = {};
	const int ret1 = randombytes(buf1, sizeof(buf1));
	const int ret2 = randombytes(buf2, sizeof(buf2));
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

static void test_issue_17(void) {
	uint8_t buf[20] = {};
	const int ret = randombytes(buf, sizeof(buf));
	assert(ret == -1);
	assert(errno = ENOTTY);
}

// ======== Mock OS functions to simulate uncommon behavior ========

#if defined(__linux__) && !defined(SYS_getrandom)
int __wrap_ioctl(int fd, int code, int* ret) {
	if (current_test == test_issue_17) {
		errno = ENOTTY;
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
#if defined(__linux__) && !defined(SYS_getrandom)
	RUN_TEST(test_issue_17)
#else
	SKIP_TEST(test_issue_17)
#endif /* defined(__linux__) && !defined(SYS_getrandom) */
	return 0;
}
