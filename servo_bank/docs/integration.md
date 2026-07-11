# Application integration

このドキュメントは、Zephyr アプリから `lib/servo_bank` を使うための手順をまとめたもの。

## Folder layout

```text
lib/servo_bank/
  CMakeLists.txt
  README.md
  docs/
    api.md
    devicetree.md
    integration.md
  include/servo_bank/servo.h
  src/servo.c

apps/xiao_servo/
  CMakeLists.txt
  boards/
  src/main.c
```

## CMake

アプリの `CMakeLists.txt` でライブラリを追加して `app` にリンクする。

```cmake
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/servo_bank servo_bank)
target_link_libraries(app PRIVATE servo_bank)
```

`lib/servo_bank/CMakeLists.txt` は普通の CMake static library として定義している。

```cmake
add_library(servo_bank STATIC src/servo.c)

target_include_directories(servo_bank PUBLIC include)
target_link_libraries(servo_bank PUBLIC zephyr_interface)
```

## main.c example

```c
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <servo_bank/servo.h>

#define SERVO_PWM_NODE DT_ALIAS(servo_pwm)

BUILD_ASSERT(DT_NODE_HAS_STATUS(SERVO_PWM_NODE, okay),
	     "servo-pwm alias must point to an okay PWM device");

static const struct servo_config servo_configs[] = {
	SERVO_CONFIG("servo0", 0, 500, 2500, 0, 180, false),
};

static const struct servo_bank servo_bank =
	SERVO_BANK_DEFINE(SERVO_PWM_NODE, servo_configs);

int main(void)
{
	int ret;
	static const uint16_t poses[][ARRAY_SIZE(servo_configs)] = {
		{ 0 },
		{ 90 },
		{ 180 },
	};

	printk("servo app start\n");

	ret = servo_bank_init(&servo_bank);
	if (ret < 0) {
		return 0;
	}

	while (1) {
		for (size_t i = 0; i < ARRAY_SIZE(poses); i++) {
			servo_bank_set_angles(&servo_bank, poses[i], ARRAY_SIZE(poses[i]));
			k_sleep(K_SECONDS(1));
		}
	}
}
```

## Adding servos

サーボを増やすときは `servo_configs` を増やす。
`servo_configs` の並び順が角度配列のインデックスになる。

```c
static const struct servo_config servo_configs[] = {
	SERVO_CONFIG("front_left_hip", 0, 600, 2400, 0, 180, false),
	SERVO_CONFIG("front_left_knee", 1, 600, 2400, 0, 180, true),
	SERVO_CONFIG("front_right_hip", 2, 600, 2400, 0, 180, true),
	SERVO_CONFIG("front_right_knee", 3, 600, 2400, 0, 180, false),
};
```

pose 配列はサーボ数と同じ幅にする。

```c
static const uint16_t poses[][ARRAY_SIZE(servo_configs)] = {
	{ 90, 90, 90, 90 },
	{ 60, 120, 120, 60 },
	{ 90, 90, 90, 90 },
};
```

## Calibration

MG90S などの小型サーボは個体差やリンク機構で無理な角度が出ることがある。
最初は `500` から `2500` us より狭い範囲で試すとよい。

例:

```c
SERVO_CONFIG("servo0", 0, 700, 2300, 0, 180, false)
```

機械的にぶつかる場合は、パルス幅か角度範囲を先に制限する。
