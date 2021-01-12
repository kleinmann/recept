#include <cstdint>
#include <array>
#include <limits>

#ifndef ALPHA_2K
#define ALPHA_2K 1
#endif

static uint32_t ema_x;
static uint32_t ema_y;

void filter(char* buf) {

	const uint8_t type = (uint8_t)buf[8];
	const uint16_t code = (
			(uint16_t)buf[10]      |
			(uint16_t)buf[11] << 8);
	uint32_t value = (
			(uint32_t)buf[12]       |
			(uint32_t)buf[13] << 8  |
			(uint32_t)buf[14] << 16 |
			(uint32_t)buf[15] << 24);

	// type == 1 && code == 320 && value == 1 -> pen in
	// type == 1 && code == 320 && value == 0 -> pen out
	// type == 1 && code == 321 && value == 1 -> eraser in
	// type == 1 && code == 321 && value == 0 -> eraser out
	//
	// type == 1 && code == 330 && value == 1 -> pen/eraser down
	// type == 1 && code == 330 && value == 0 -> pen/eraser up
	//
	// type == 3 && code == 0 -> value == x
	// type == 3 && code == 1 -> value == y
	// type == 3 && code == 24 -> value == pressure
	// type == 3 && code == 25 -> value == distance
	// type == 3 && code == 26 -> value == tilt x
	// type == 3 && code == 27 -> value == tilt y


	// pen/eraser in
	if (type == 1 && ((code == 320 && value == 1) || (code == 321 && value == 1))) {
		ema_x = value; // Reset EMA when pen/eraser is touching
		ema_y = value; // Reset EMA when pen/eraser is touching
	}

	if (type == 3 && code < 2) {
		if (code == 0) {
			ema_x += value;
			value = ema_x >> ALPHA_2K;
			ema_x -= value;
		} else if (code == 1) {
			ema_y += value;
			value = ema_y >> ALPHA_2K;
			ema_y -= value;
		}

		// copy value back to buffer
		buf[12] = (uint8_t)value;
		buf[13] = (uint8_t)(value >> 8);
		buf[14] = (uint8_t)(value >> 16);
		buf[15] = (uint8_t)(value >> 24);
	}
}

extern "C" {

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

typedef int (*t_open)(const char*, int);
typedef ssize_t (*t_read)(int, void*, size_t);

static int event_fd = 0;

int open(const char* filename, int flags) {

	static t_open real_open = NULL;
	if (!real_open)
		real_open = (t_open)dlsym(RTLD_NEXT, "open");

	int fd = real_open(filename, flags);

	if (strcmp(filename, "/dev/input/event1") == 0) {

		printf(">| someone is trying to open event1, let's remember the fd!\n");
		printf(">| (psssst: it is %d)\n", fd);
		event_fd = fd;
	}

	return fd;
}

ssize_t read(int fd, void* buf, size_t count) {

	static t_read real_read = NULL;
	if (!real_read)
		real_read = (t_read)dlsym(RTLD_NEXT, "read");

	ssize_t ret = real_read(fd, buf, count);

	if (fd != 0 && fd == event_fd && ret == 16)
		filter((char*)buf);

	return ret;
}

}
