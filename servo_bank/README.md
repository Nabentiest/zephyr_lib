# servo_bank

`servo_bank` is a small Zephyr-facing servo control library.

The library depends on Zephyr's PWM API, not on PCA9685 registers, I2C transactions, or a specific board.
Any device that is exposed as a Zephyr PWM device can be used as the servo output device.

## Documents

- [API reference](docs/api.md)
- [Devicetree contract](docs/devicetree.md)
- [Application integration](docs/integration.md)

## Application contract

The application provides:

- A PWM device node from Devicetree.
- A `struct servo_config` array describing each servo channel.
- A `struct servo_bank` that combines the PWM device and the servo array.

Example:

```c
#include <servo_bank/servo.h>

#define SERVO_PWM_NODE DT_ALIAS(servo_pwm)

static const struct servo_config servo_configs[] = {
	SERVO_CONFIG("servo0", 0, 500, 2500, 0, 180, false),
};

static const struct servo_bank servo_bank = SERVO_BANK_DEFINE(SERVO_PWM_NODE, servo_configs);
```

## Devicetree contract

The board overlay must provide a `servo-pwm` alias that points to an enabled PWM device.

For PCA9685:

```dts
/ {
	aliases {
		servo-pwm = &pca9685;
	};
};

&i2c0 {
	status = "okay";

	pca9685: pca9685@40 {
		compatible = "nxp,pca9685-pwm";
		reg = <0x40>;
		status = "okay";
		#pwm-cells = <3>;
	};
};
```

For another board, keep the `servo-pwm` alias and change the PWM provider node as needed.
If the new device supports Zephyr's `pwm_set()` API, `servo_bank` does not need to change.
