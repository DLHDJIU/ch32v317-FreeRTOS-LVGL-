/******************************************************************************
 * CH32V317VCT6 + LVGL 9 移植示例（FreeRTOS 可选）
 *
 * 本文件只保留两件事：
 *   1) ST7798 屏 (320x240, 软件 SPI) 的 LVGL 显示接口 disp_flush()
 *   2) 主循环 / 任务框架 + 一个演示界面
 *
 * 没有网络 / PHY / TDR / 业务逻辑。把它当作新项目的起点即可。
 *
 * 关于 FreeRTOS：
 *   FreeRTOS 源码和 RISC-V 移植层仍在 FreeRTOS/ 目录下，但本仓库自带的
 *   portable/GCC/RISC-V/port.c 是 FreeRTOS 官方版本（依赖 CLINT / MTIME），
 *   与 CH32V317 的 PFIC + SysTick(0xE000F000) 并不匹配，直接启动调度器
 *   无法产生节拍。因此默认 USE_FREERTOS = 0，走已经验证过的裸机循环。
 *   要启用请先按 README 的说明替换 port 层。
 *****************************************************************************/

#include "debug.h"
#include "lvgl.h"
#include "lcd_st7798.h"
#include "ui/ui_anyui5.h"

/* 0 = 裸机主循环（默认，已验证）
 * 1 = FreeRTOS 调度器（需先修好 port 层的 SysTick 支持） */
#define USE_FREERTOS        0

#if USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

/* ============================ 显示参数 ============================ */
#define DISP_HOR_RES        320u
#define DISP_VER_RES        240u
#define DISP_BUF_LINES      8u      /* 局部刷新缓冲：320*8*2 = 5 KB */

#define LOOP_TICK_MS        5u      /* 主循环步进 */
#define GUI_REFRESH_MS      10u     /* lv_timer_handler 调用周期 */
#define DEMO_PERIOD_MS      1000u   /* 演示数据刷新周期 */

/* ============================ 任务参数 ============================ */
#define GUI_TASK_STACK      1024u   /* 单位：字 (4 字节) = 4 KB */
#define GUI_TASK_PRIO       (tskIDLE_PRIORITY + 2u)
#define DEMO_TASK_STACK     512u
#define DEMO_TASK_PRIO      (tskIDLE_PRIORITY + 1u)

/* ============================ 显示缓冲 ============================ */
static uint16_t disp_buf[DISP_HOR_RES * DISP_BUF_LINES];
static lv_display_t *g_disp;

/* ============================ LVGL 显示接口 ============================ */
/* 把 LVGL 渲染好的区域通过 ST7798 写进屏，写完必须通知 LVGL 可以继续。 */
static void disp_flush(lv_display_t *disp, const lv_area_t *area,
                       uint8_t *px_map)
{
    uint32_t count = (uint32_t)(area->x2 - area->x1 + 1) *
                     (uint32_t)(area->y2 - area->y1 + 1);

    LCD_ST7798_SetWindow((uint16_t)area->x1, (uint16_t)area->y1,
                         (uint16_t)area->x2, (uint16_t)area->y2);
    LCD_ST7798_WritePixels((const uint16_t *)px_map, count);

    lv_display_flush_ready(disp);
}

static void lcd_lvgl_init(void)
{
    lv_display_t *disp;

    LCD_ST7798_Init();

    lv_init();

    disp = lv_display_create((int32_t)DISP_HOR_RES, (int32_t)DISP_VER_RES);
    g_disp = disp;
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(disp, disp_buf, NULL, sizeof(disp_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, disp_flush);

    lv_screen_load(ui_environment_main_create());
    lv_refr_now(disp);

    printf("[LCD] ST7798 + LVGL ready (%ux%u)\r\n",
           (unsigned)DISP_HOR_RES, (unsigned)DISP_VER_RES);
}

/* ============================ 演示数据 ============================ */
/* 每秒刷一组假数据，验证"改数据 -> 局部刷新"的链路。 */
static void demo_update(uint32_t second)
{
    ui_environment_set_readings((int32_t)(230 + (second % 20u)),
                                (uint32_t)(45u + (second % 20u)),
                                (uint32_t)(600u + (second % 200u)),
                                (uint32_t)(12u + (second % 30u)));
    ui_dynamic_data_update(second, second, 0u);
}

#if USE_FREERTOS
/* ============================ GUI 任务 ============================ */
/* LVGL 不是线程安全的：所有 lv_* 调用都集中在这一个任务里。 */
static void gui_task(void *arg)
{
    (void)arg;

    for (;;)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(GUI_REFRESH_MS));
    }
}

static void demo_task(void *arg)
{
    uint32_t second = 0u;
    (void)arg;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(DEMO_PERIOD_MS));
        second++;
        demo_update(second);
    }
}

/* ============================ FreeRTOS 钩子 ============================ */
void vApplicationMallocFailedHook(void)
{
    printf("[RTOS] Malloc Failed!\r\n");
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    printf("[RTOS] Stack Overflow: %s\r\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
#endif /* USE_FREERTOS */

/* ============================ 入口 ============================ */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    printf("\r\n====================================\r\n");
    printf(" CH32V317VCT6  LVGL demo (%s)\r\n",
           USE_FREERTOS ? "FreeRTOS" : "bare-metal");
    printf(" SystemClk: %lu Hz\r\n", (unsigned long)SystemCoreClock);
    printf("====================================\r\n");

    lcd_lvgl_init();

#if USE_FREERTOS
    xTaskCreate(gui_task,  "gui",  GUI_TASK_STACK,  NULL, GUI_TASK_PRIO,  NULL);
    xTaskCreate(demo_task, "demo", DEMO_TASK_STACK, NULL, DEMO_TASK_PRIO, NULL);

    vTaskStartScheduler();

    /* 调度器启动失败（通常是堆不够）才会执行到这里。 */
    printf("[RTOS] scheduler start failed: heap exhausted?\r\n");
    for (;;) { }
#else
    {
        uint32_t elapsed_ms = 0u;
        uint32_t gui_elapsed_ms = 0u;
        uint32_t demo_elapsed_ms = 0u;
        uint32_t second = 0u;

        for (;;)
        {
            Delay_Ms(LOOP_TICK_MS);
            elapsed_ms += LOOP_TICK_MS;
            gui_elapsed_ms += LOOP_TICK_MS;
            demo_elapsed_ms += LOOP_TICK_MS;

            /* LVGL 需要一个毫秒时基。 */
            lv_tick_inc(LOOP_TICK_MS);

            /* 20~100 FPS 都可以，按屏的实际刷屏速度调。 */
            if (gui_elapsed_ms >= GUI_REFRESH_MS)
            {
                gui_elapsed_ms = 0u;
                lv_timer_handler();
            }

            /* 每 1 秒更新一次演示数据。 */
            if (demo_elapsed_ms >= DEMO_PERIOD_MS)
            {
                demo_elapsed_ms = 0u;
                second++;
                demo_update(second);
            }

            (void)elapsed_ms;
        }
    }
#endif
}
