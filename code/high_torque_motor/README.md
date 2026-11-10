# 大电机首次测试：Mega 2560 + CL86Y-V20

参考 [22_stepper_acceleration_ramp](https://github.com/yuxuan-518/atmega2560-embedded-foundations/tree/main/code/22_stepper_acceleration_ramp)。沿用 AVR C、定时器、串口命令、加减速状态机的结构；本项目已包含所有依赖源码，无需仓库里的共享 `code/lib`，也不使用 Arduino 框架或 AccelStepper。

## 当前接线和拨码

| Mega 2560 | 驱动器 |
|---|---|
| D23 / PA1 | PU+ |
| D22 / PA0 | DR+ / DIR+ |
| GND | PU−、DR− / DIR− |
| USB | Mega 自身供电 |

Mega 的 5V 不接驱动器。电机的 A+/A−/B+/B− 和编码器线按此前确认的对应关系接驱动器。驱动器独立供电；MF、PEND、ALM 和 RS232 调试口暂时留空。

| 设置 | 值 |
|---|---|
| 逻辑电平选择 | 5V |
| SW1 | OFF：脉冲 + 方向 |
| SW2 | OFF：默认方向 |
| SW3 / SW4 / SW5 / SW6 | ON / OFF / ON / ON |
| 每圈脉冲数 | 1600 |

DIR 显示的是引脚电平 0/1，实际正反方向以实物为准。若修改细分，转速换算也必须随之修改。

## 编译、上传和串口

用 VS Code + PlatformIO 打开本文件夹，选择 `megaatmega2560` 环境，先 Build，再自行 Upload。也可在该文件夹的终端执行：

```sh
pio run
pio run -t upload
pio device monitor -b 9600
```

串口为 **9600、8N1**，命令全部小写，按回车发送。支持 LF、CR 和 CRLF。若终端未配置 `pio` 命令，可使用 PlatformIO 按钮，或本机的 `~/.platformio/penv/bin/pio`。本项目是 AVR C 工程，不是可直接在 Arduino IDE 打开的 `.ino` 工程。

程序上电/复位后**不自动转动**，应看到：

```text
BOOT CL86Y-V20; STOPPED; STEP=D23 DIR=D22; type help
```

## 第一次实物测试

先固定好电机，空载测试。驱动器电源型号、电压和限流值尚未在本次对话中确认，因此本文不预设可调电源数值；先确认供电，再打开驱动器输出。接线、修改拨码、插拔电机线均在断电后进行。

打开串口后，依次发送，每行回车：

```text
status
speed 200
accel 100
demo
```

`demo` 只运行一次：从 50 Hz 缓慢加速至 200 Hz，保持约 3 秒，再减速停止。1600 脉冲/圈时，200 Hz 约为 **7.5 RPM**。整个过程约 6 秒。串口显示 `DEMO DONE` 表示脉冲程序已完成；并不证明电机实际到位。

观察方向、转动是否平稳、电源是否进入限流、驱动器红灯是否报警；然后再逐步提高速度。需要持续转动时使用 `start`，结束使用 `stop`。

## 命令

| 命令 | 行为 |
|---|---|
| `help` | 显示命令 |
| `status` | 显示状态、当前指令频率、目标频率、加速度、方向电平 |
| `start` | 从低速启动，持续运行至收到停止命令 |
| `stop` | 减速至 50 Hz 后停止发脉冲，并取消待执行换向 |
| `estop` | 立即停止发脉冲，取消演示和换向；驱动器仍可能保持锁轴 |
| `reverse` | 停止时切换方向；运行时先减速、停止脉冲，再换向并加速 |
| `speed 400` | 目标 400 Hz；允许 50–2000 Hz；停止时设置不会自动启动 |
| `accel 200` | 加/减速度 200 Hz/s；允许 1–2000；初测不提供 `accel 0` 跳变模式 |
| `accel low` / `accel high` | 100 / 500 Hz/s |
| `demo` | 按当前目标和加速度运行一次，加速、保持 3 秒、减速停止 |
| `log on` / `log off` | 每 200 ms 输出 `LOG,毫秒,指令Hz` |

`start/stop/estop/reverse` 或有效的速度/加速度修改会取消演示流程。取消演示本身不会自动停止持续运动，停机需发送 `stop` 或 `estop`。

默认目标 400 Hz（15 RPM），启动 50 Hz（1.875 RPM），默认加速度 200 Hz/s，上限 2000 Hz（75 RPM）。这些是首次测试限制，不是电机额定能力。提高速度并不增加扭矩。换算：`RPM = Hz × 60 ÷ 1600`。

**反馈限制：** 编码器只连接驱动器，Mega 未连接 ALM/PEND，所以 `HZ`、状态和日志仅反映程序发出的指令，不是实测转速或故障反馈。`estop` 是串口软件停脉冲命令，不能切断驱动器电源；机械停下所需时间取决于惯性和负载。

## 文件和实现

- `src/main.c`：主循环，开机停止，状态输出。
- `src/motor.c`：参考项目的加减速和换向状态机。
- `src/motor_io.c`：Timer1 中断驱动 D23，D22 控制方向；方向切换留出建立/保持时间。
- `src/timer.c`：Timer3 提供 1 ms 时钟。
- `src/usart.c`、`src/command/`：USB 串口命令，拒绝超长、非数字、溢出及超范围参数。
- `src/demo.c`：一次性演示。
- `include/motor.h`：集中修改频率、加速度和每圈脉冲数。

不使用参考代码的 D11/OC1A，因为当前 PU+ 已接 D23。Timer1 自由运行，以比较中断翻转 PA1，输出约 50% 占空比脉冲；修改频率在中断周期边界生效。串口收发不会直接阻塞脉冲中断。长串口输出后，加减速单次追赶时间限制为 50 ms，避免频率突然跳变。整个项目独占 Timer1、Timer3、USART0。

有本机 AVR 工具链时，也可 `make` 编译并生成 `build/firmware.hex`；`make test` 运行主机上的状态机和命令测试。主机测试模拟 GPIO/UART，不代替实物波形、实际扭矩、驱动器报警和供电测试。代码由本次任务编译验证后交付；未连接或上传到用户硬件。
