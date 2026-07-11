# Devicetree contract

`servo_bank` は Devicetree の細部を直接知らない。
ただし、アプリ側はサーボ出力に使う PWM デバイスを Devicetree から取得する必要がある。

このライブラリでは、アプリが `servo-pwm` alias を使う前提にしている。

## Contract

board overlay は次の条件を満たすこと。

- `servo-pwm` alias が存在する。
- `servo-pwm` alias が `status = "okay"` の PWM デバイスを指している。
- その PWM デバイスが Zephyr の `pwm_set()` API で操作できる。

アプリ側では次のように受ける。

```c
#define SERVO_PWM_NODE DT_ALIAS(servo_pwm)

BUILD_ASSERT(DT_NODE_HAS_STATUS(SERVO_PWM_NODE, okay),
	     "servo-pwm alias must point to an okay PWM device");
```

## PCA9685 example

PCA9685 を使う場合の overlay 例。

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

別ボードでは `&i2c0` の部分を、そのボードで PCA9685 をつないだ I2C バスに変える。
必要に応じて、そのボードの pinctrl や I2C clock 設定を追加する。

## Porting to another PWM device

PCA9685 以外でも、Zephyr PWM デバイスとして公開されているなら使える。

移植時に守ることは、`servo-pwm` alias が有効な PWM デバイスを指すようにすること。
アプリと `lib/servo_bank` のコードは基本的に変更しない。

## Hardware notes

- PCA9685 の OE は Low で出力有効。High につなぐと PWM が出ない。
- サーボ電源と MCU/PWM デバイスの GND は共通にする。
- サーボ電源は MCU の 3.3 V から取らない。サーボ用に十分な電源を用意する。
- 最初は狭いパルス幅で動作確認し、機械的にぶつからない範囲を確認してから広げる。
