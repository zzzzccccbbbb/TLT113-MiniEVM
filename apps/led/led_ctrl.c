/*
 * led_ctrl.c - T113 MiniEVM user-led0 via sysfs
 *
 * LED is GPIO PG11 (gpio-leds), not hardware PWM.
 * Breathing uses software duty-cycle on brightness 0/1.
 *
 * Usage:
 *   led_ctrl on|off
 *   led_ctrl blink [on_ms] [off_ms]
 *   led_ctrl breath [period_ms]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define LED_TRIG "/sys/class/leds/user-led0/trigger"
#define LED_BRI  "/sys/class/leds/user-led0/brightness"

static int write_str(const char *path, const char *val)
{
	int fd = open(path, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "open %s: %s\n", path, strerror(errno));
		return -1;
	}
	if (write(fd, val, strlen(val)) < 0) {
		fprintf(stderr, "write %s: %s\n", path, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

static int led_prepare(void)
{
	/* stop kernel triggers (heartbeat/mmc/...) */
	return write_str(LED_TRIG, "none");
}

static int led_set(int on)
{
	return write_str(LED_BRI, on ? "1" : "0");
}

static void usage(const char *argv0)
{
	fprintf(stderr,
		"Usage:\n"
		"  %s on|off\n"
		"  %s blink [on_ms=300] [off_ms=300]\n"
		"  %s breath [period_ms=1000]\n",
		argv0, argv0, argv0);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	if (led_prepare() < 0)
		return 1;

	if (!strcmp(argv[1], "on")) {
		return led_set(1) < 0 ? 1 : 0;
	}
	if (!strcmp(argv[1], "off")) {
		return led_set(0) < 0 ? 1 : 0;
	}
	if (!strcmp(argv[1], "blink")) {
		int on_ms = (argc >= 3) ? atoi(argv[2]) : 300;
		int off_ms = (argc >= 4) ? atoi(argv[3]) : 300;
		if (on_ms < 10) on_ms = 10;
		if (off_ms < 10) off_ms = 10;
		for (;;) {
			led_set(1);
			usleep(on_ms * 1000);
			led_set(0);
			usleep(off_ms * 1000);
		}
	}
	if (!strcmp(argv[1], "breath")) {
		/* soft PWM: 20 steps up/down within period_ms */
		int period_ms = (argc >= 3) ? atoi(argv[2]) : 1000;
		int steps = 20;
		int slice_us = (period_ms * 1000) / (steps * 2);
		if (slice_us < 500) slice_us = 500;
		for (;;) {
			int i;
			for (i = 0; i <= steps; i++) {
				int on_us = (slice_us * i) / steps;
				int off_us = slice_us - on_us;
				led_set(1);
				if (on_us) usleep(on_us);
				led_set(0);
				if (off_us) usleep(off_us);
			}
			for (i = steps; i >= 0; i--) {
				int on_us = (slice_us * i) / steps;
				int off_us = slice_us - on_us;
				led_set(1);
				if (on_us) usleep(on_us);
				led_set(0);
				if (off_us) usleep(off_us);
			}
		}
	}

	usage(argv[0]);
	return 1;
}
