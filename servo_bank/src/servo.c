#include <servo_bank/servo.h>

#include <errno.h>

#include <zephyr/sys/printk.h>

static uint32_t clamp_u32(uint32_t value, uint32_t min, uint32_t max)
{
	if (value < min) {
		return min;
	}

	if (value > max) {
		return max;
	}

	return value;
}

static const struct servo_config *servo_at(const struct servo_bank *bank, size_t index)
{
	if (index >= bank->count) {
		return NULL;
	}

	return &bank->servos[index];
}

int servo_bank_init(const struct servo_bank *bank)
{
	int ret;

	if (!device_is_ready(bank->pwm_dev)) {
		printk("PWM not ready: %s\n", bank->pwm_dev->name);
		return -ENODEV;
	}

	for (size_t i = 0; i < bank->count; i++) {
		ret = servo_bank_set_angle(bank, i, bank->servos[i].min_angle_deg);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

int servo_bank_set_pulse_us(const struct servo_bank *bank, size_t index, uint32_t pulse_us)
{
	const struct servo_config *servo = servo_at(bank, index);
	int ret;
	uint32_t clamped_pulse;

	if (servo == NULL) {
		return -EINVAL;
	}

	clamped_pulse = clamp_u32(pulse_us, servo->min_pulse_us, servo->max_pulse_us);

	ret = pwm_set(bank->pwm_dev, servo->channel, PWM_USEC(bank->period_us), PWM_USEC(clamped_pulse),
		      servo->inverted ? PWM_POLARITY_INVERTED : PWM_POLARITY_NORMAL);

	printk("%s ch=%u pulse=%u us ret=%d\n", servo->name, servo->channel, clamped_pulse, ret);

	return ret;
}

int servo_bank_set_angle(const struct servo_bank *bank, size_t index, uint16_t angle_deg)
{
	const struct servo_config *servo = servo_at(bank, index);
	uint32_t angle;
	uint32_t angle_range;
	uint32_t pulse_range;
	uint32_t pulse_us;

	if (servo == NULL) {
		return -EINVAL;
	}

	angle = clamp_u32(angle_deg, servo->min_angle_deg, servo->max_angle_deg);
	angle_range = servo->max_angle_deg - servo->min_angle_deg;
	pulse_range = servo->max_pulse_us - servo->min_pulse_us;

	if (angle_range == 0U) {
		return -EINVAL;
	}

	pulse_us = servo->min_pulse_us +
		   ((angle - servo->min_angle_deg) * pulse_range) / angle_range;

	return servo_bank_set_pulse_us(bank, index, pulse_us);
}

int servo_bank_set_angles(const struct servo_bank *bank, const uint16_t *angles_deg, size_t count)
{
	int ret;
	size_t apply_count = count < bank->count ? count : bank->count;

	for (size_t i = 0; i < apply_count; i++) {
		ret = servo_bank_set_angle(bank, i, angles_deg[i]);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}
