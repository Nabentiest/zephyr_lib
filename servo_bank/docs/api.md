# servo_bank API reference

このドキュメントは、`lib/servo_bank/include/servo_bank/servo.h` で公開している API の仕様をまとめたもの。

## 方針

`servo_bank` は、複数のサーボを1つの PWM デバイス上のチャンネル配列として扱うための薄いライブラリ。

ライブラリは I2C や PCA9685 のレジスタを直接触らない。
依存しているのは Zephyr の PWM API、つまり `pwm_set()`。

PCA9685 が I2C デバイスであることは、Zephyr の PCA9685 PWM ドライバと board overlay 側に閉じ込める。

## Include

```c
#include <servo_bank/servo.h>
```

## Data structures

### struct servo_config

1本のサーボ設定を表す。

```c
struct servo_config {
	const char *name;
	uint8_t channel;
	uint16_t min_pulse_us;
	uint16_t max_pulse_us;
	uint16_t min_angle_deg;
	uint16_t max_angle_deg;
	bool inverted;
};
```

| フィールド | 意味 |
| --- | --- |
| `name` | ログ表示用の名前 |
| `channel` | PWM デバイス上の出力チャンネル番号 |
| `min_pulse_us` | 最小角度に対応するパルス幅 |
| `max_pulse_us` | 最大角度に対応するパルス幅 |
| `min_angle_deg` | 扱う最小角度 |
| `max_angle_deg` | 扱う最大角度 |
| `inverted` | PWM 極性を反転するなら `true` |

### struct servo_bank

1つの PWM デバイスと、その上に並ぶ複数サーボの設定配列をまとめる。

```c
struct servo_bank {
	const struct device *pwm_dev;
	const struct servo_config *servos;
	size_t count;
	uint32_t period_us;
};
```

## Macros

### SERVO_CONFIG

```c
SERVO_CONFIG(name, channel, min_pulse_us, max_pulse_us, min_angle_deg, max_angle_deg, inverted)
```

`struct servo_config` の初期化用マクロ。

使用例:

```c
static const struct servo_config servo_configs[] = {
	SERVO_CONFIG("servo0", 0, 500, 2500, 0, 180, false),
};
```

### SERVO_BANK_DEFINE

```c
SERVO_BANK_DEFINE(pwm_node_id, servo_configs)
```

PWM デバイス、サーボ設定配列、サーボ数、PWM 周期をまとめた `struct servo_bank` を初期化する。
周期は現在 `20000` us、つまり 50 Hz。

使用例:

```c
#define SERVO_PWM_NODE DT_ALIAS(servo_pwm)

static const struct servo_bank servo_bank =
	SERVO_BANK_DEFINE(SERVO_PWM_NODE, servo_configs);
```

## Functions

### servo_bank_init

```c
int servo_bank_init(const struct servo_bank *bank);
```

PWM デバイスが使える状態か確認し、全サーボをそれぞれの `min_angle_deg` に初期化する。

戻り値:

| 値 | 意味 |
| --- | --- |
| `0` | 成功 |
| `-ENODEV` | PWM デバイスが ready ではない |
| 負の値 | `pwm_set()` など下位処理のエラー |

### servo_bank_set_angle

```c
int servo_bank_set_angle(const struct servo_bank *bank, size_t index, uint16_t angle_deg);
```

`index` 番目のサーボを指定角度へ動かす。
角度は `min_angle_deg` から `max_angle_deg` の範囲に丸められる。
その後、角度をパルス幅へ線形変換して `pwm_set()` を呼ぶ。

戻り値:

| 値 | 意味 |
| --- | --- |
| `0` | 成功 |
| `-EINVAL` | `index` が範囲外、または角度範囲が不正 |
| 負の値 | `pwm_set()` のエラー |

### servo_bank_set_angles

```c
int servo_bank_set_angles(const struct servo_bank *bank, const uint16_t *angles_deg, size_t count);
```

角度配列をまとめて適用する。
`angles_deg[0]` が `servos[0]`、`angles_deg[1]` が `servos[1]` に対応する。

`count` がサーボ数より多い場合、余った値は無視される。
`count` がサーボ数より少ない場合、足りないサーボは更新されない。

TFLite や IK の出力が角度配列になっている場合は、この関数に渡す形にすると扱いやすい。

戻り値:

| 値 | 意味 |
| --- | --- |
| `0` | 成功 |
| 負の値 | 最初に失敗したサーボ操作のエラー |

### servo_bank_set_pulse_us

```c
int servo_bank_set_pulse_us(const struct servo_bank *bank, size_t index, uint32_t pulse_us);
```

`index` 番目のサーボへ、角度ではなくパルス幅を直接指定する。
指定値は `min_pulse_us` から `max_pulse_us` の範囲に丸められる。

通常は `servo_bank_set_angle()` を使い、キャリブレーションやデバッグ時だけこの関数を使う。

戻り値:

| 値 | 意味 |
| --- | --- |
| `0` | 成功 |
| `-EINVAL` | `index` が範囲外 |
| 負の値 | `pwm_set()` のエラー |
