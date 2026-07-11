#ifndef SERVO_BANK_SERVO_H
#define SERVO_BANK_SERVO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/util.h>

struct servo_config {
	const char *name;
	uint8_t channel;
	uint16_t min_pulse_us;
	uint16_t max_pulse_us;
	uint16_t min_angle_deg;
	uint16_t max_angle_deg;
	bool inverted;
};

struct servo_bank {
	const struct device *pwm_dev;
	const struct servo_config *servos;
	size_t count;
	uint32_t period_us;
};

#define SERVO_CONFIG(servo_name, pwm_channel, min_pulse, max_pulse, min_angle, max_angle, invert) \
	{                                                                                       \
		.name = servo_name,                                                             \
		.channel = pwm_channel,                                                         \
		.min_pulse_us = min_pulse,                                                      \
		.max_pulse_us = max_pulse,                                                      \
		.min_angle_deg = min_angle,                                                     \
		.max_angle_deg = max_angle,                                                     \
		.inverted = invert,                                                             \
	}

#define SERVO_BANK_DEFINE(pwm_node_id, servo_configs) \
	{                                             \
		.pwm_dev = DEVICE_DT_GET(pwm_node_id),    \
		.servos = servo_configs,                  \
		.count = ARRAY_SIZE(servo_configs),       \
		.period_us = 20000U,                      \
	}

int servo_bank_init(const struct servo_bank *bank);
int servo_bank_set_pulse_us(const struct servo_bank *bank, size_t index, uint32_t pulse_us);
int servo_bank_set_angle(const struct servo_bank *bank, size_t index, uint16_t angle_deg);
int servo_bank_set_angles(const struct servo_bank *bank, const uint16_t *angles_deg, size_t count);

#endif
