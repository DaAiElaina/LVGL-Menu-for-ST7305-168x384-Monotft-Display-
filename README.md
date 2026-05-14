# LVGL Menu for ST7305 (168x384 MonotftDisplay)

基于 LVGL 8.3.10 和 ST7305 monotft屏的菜单系统，支持摇杆控制焦点切换和菜单导航。

## 特性

- 摇杆控制（上/下/左/右/按下）
- 主菜单/子菜单切换
- 焦点循环切换
- 弹出提示窗口
- 模块化代码结构

## 硬件连接

| 组件 | 引脚 | 说明 |
|------|------|------|
| DC | 38 | 数据/命令选择 |
| RST | 0 | 复位 |
| CS | 45 | 片选 |
| SCLK | 36 | SPI 时钟 |
| SDIN | 37 | SPI 数据 (MOSI) |
| Joy X | 11 | 摇杆 X 轴（模拟输入） |
| Joy Y | 12 | 摇杆 Y 轴（模拟输入） |
| Joy SW | 48 | 摇杆按键（数字输入） |

## 文件结构
```
src/
├── main.cpp # 主入口
├── lvgl_driver_screen.h/cpp # 屏幕显示驱动
├── lvgl_driver_encoder.h/cpp # 摇杆输入驱动
└── lvgl_menu.h/cpp # 菜单逻辑
```

## 修改配置

### 1. 修改屏幕引脚

**编辑** `lvgl_driver_screen.cpp`：


```
#define PIN_DC      38   // 数据/命令引脚
#define PIN_RST     0    // 复位引脚
#define PIN_CS      45   // 片选引脚
#define PIN_SCLK    36   // SPI 时钟引脚
#define PIN_SDIN    37   // SPI 数据引脚
```
### 2.修改摇杆引脚
**编辑** `lvgl_driver_encoder.cpp`：
```
    const int center = 2048;      // 摇杆中心值 (ADC 0-4095)
    const int threshold = 600;    // 触发阈值，越小越灵敏
```

### 3. 修改摇杆灵敏度
**编辑** `lvgl_driver_encoder.cpp` 中的 `encoder_read()` 函数：

```
const int center = 2048;      // 摇杆中心值 (ADC 0-4095)
const int threshold = 600;    // 触发阈值，越小越灵敏
```

### 4. 修改按键防抖延迟
**编辑** `lvgl_driver_encoder.cpp`：
```
static const unsigned long key_delay = 200;  // 毫秒，防抖延迟
```
## 显示原理

### 屏幕驱动层 (`lvgl_driver_screen.cpp`)

本项目的显示驱动基于 **逐像素绘制** 方式实现。

#### 核心原理

1. **内存布局**：屏幕物理分辨率为 168x384 像素，采用 **4x2 像素打包** 方式存储在 `display_buffer` 中：
   - 每 4 个水平像素和 2 个垂直像素打包成 1 个字节
   - 每个字节对应 4x2 像素区域

2. **像素格式**：
```
BIT7 BIT5 BIT3 BIT1 (第0行)
BIT6 BIT4 BIT2 BIT0 (第1行)
```

3. **显示刷新流程**：
>LVGL 计算 → my_disp_flush() → writePoint() → display_buffer → display() → SPI → 屏幕

4. **回调函数**：`my_disp_flush()` 接收 LVGL 传来的颜色数据（`LV_COLOR_DEPTH=1`，每个像素 0 或 1），然后逐像素调用 `display.writePoint()` 写入缓冲区，最后调用 `display.display()` 通过 SPI 发送到屏幕。

#### 为什么这样实现？

| 原因 | 说明 |
|------|------|
| **硬件限制** | ST7305 驱动芯片使用特殊的 4x2 像素打包格式，不能直接写入帧缓冲区 |
| **兼容性** | 逐像素绘制方式最简单可靠，能确保 LVGL 在任何屏幕驱动上工作 |
| **调试友好** | 出问题时可以快速定位是绘制问题还是坐标计算问题 |

#### 性能说明

- **逐像素绘制**：全屏刷新约 64,512 次 `writePoint()` 调用
- **建议优化**：如需更高帧率，可实现批量绘制函数直接操作 `display_buffer`

## 摇杆控制原理

### 输入类型

使用 `LV_INDEV_TYPE_KEYPAD`，摇杆方向映射为 LVGL 按键事件：

| 摇杆动作 | LVGL 事件 | 效果 |
|----------|-----------|------|
| 上/下/左/右 | `LV_KEY_UP/DOWN/LEFT/RIGHT` | 切换焦点 |
| 按下 | `LV_KEY_ENTER` | 触发当前控件点击 |

### 焦点管理

1. 使用 `lv_group_t` 管理可聚焦控件
2. 所有按钮添加到同一个焦点组
3. 焦点切换时同步更新视觉样式（黑底白字/白底黑字）

## 依赖库

- [lvgl](https://github.com/lvgl/lvgl) v8.3.10 - GUI 图形库
- ST7305_2p9_BW_DisplayDriver - 屏幕驱动（鱼鹰光电资料）

## 编译环境

- PlatformIO / Arduino IDE
- ESP32-S3 开发板

## 操作说明

| 摇杆操作 | 功能 |
|----------|------|
| 上/下/左/右 | 在菜单按钮之间切换焦点 |
| 按下 | 确认选择 |
| Enter 按钮 | 进入子菜单 |
| Settings 按钮 | 显示"Coming Soon"提示 |
| Back 按钮 | 返回主菜单 |

>ps：需要自己下载lvgl库，然后一定一定要把lvgl.conf放入lvgl根目录，否则会报错！

>搭好框架就可以爽玩lvgl辣~

>还有最后，这个项目本质上是我自己刚开始接触lvgl写着玩玩的，所以有些简陋啦，反正都用lvgl了，已经搭好框架可以往上加东西了！
