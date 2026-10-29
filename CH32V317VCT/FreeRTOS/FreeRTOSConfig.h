#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "debug.h"   // 为了使用 printf

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1
#define configCPU_CLOCK_HZ                      (SystemCoreClock)   // 144MHz
#define configTICK_RATE_HZ                      (1000)              // 1ms
#define configMAX_PRIORITIES                    (5)
#define configMINIMAL_STACK_SIZE                (128)   // 注意：RISC-V下单位是字(4字节)，128=512字节
#define configTOTAL_HEAP_SIZE                   (24 * 1024)   // 3 个任务 + 队列，96KB SRAM 下留足余量
#define configMAX_TASK_NAME_LEN                 (16)
#define configUSE_TRACE_FACILITY                0
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE               8
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1

/* 软件中断用于上下文切换，青稞V4F支持 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

/* 运行时统计 */
#define configGENERATE_RUN_TIME_STATS           0

/* 协程 */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         (2)

/* 定时器任务 */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (2)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

/* 中断优先级（青稞V4F用PFIC，这里用默认即可） */
#define configKERNEL_INTERRUPT_PRIORITY         0
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    1

/* 断言 */
#define configASSERT(x) if((x)==0) { printf("ASSERT: %s:%d\n", __FILE__, __LINE__); taskDISABLE_INTERRUPTS(); for(;;); }

/* 钩子函数 */
#define vAssertCalled(char, int)    printf("Error: %s, %d\n", char, int)
#define configUSE_MALLOC_FAILED_HOOK    1

/* RISC-V 相关 */
#define configMTIME_BASE_ADDRESS    (0xE000E010UL)   // 青稞V4F的SysTick类似物，如果FreeRTOS用软件定时器可能不需要
#define configMTIMECMP_BASE_ADDRESS (0xE000E014UL)

/* 包含 API */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1

#endif /* FREERTOS_CONFIG_H */