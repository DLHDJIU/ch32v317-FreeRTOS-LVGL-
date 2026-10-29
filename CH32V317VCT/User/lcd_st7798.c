#include "lcd_st7798.h"

/* Board wiring (software SPI):
 * SCL=PC10, SDA=PC12, CS=PA8, RST=PC8, DC/RS=PC9, BL=PA15 (TIM2_CH1_1).
 * PA6/SDO is not required for this write-only display.
 */
#define LCD_SCL_0()  (GPIOC->BCR  = GPIO_Pin_10)
#define LCD_SCL_1()  (GPIOC->BSHR = GPIO_Pin_10)
#define LCD_SDA_0()  (GPIOC->BCR  = GPIO_Pin_12)
#define LCD_SDA_1()  (GPIOC->BSHR = GPIO_Pin_12)
#define LCD_CS_0()   (GPIOA->BCR  = GPIO_Pin_8)
#define LCD_CS_1()   (GPIOA->BSHR = GPIO_Pin_8)
#define LCD_RST_0()  (GPIOC->BCR  = GPIO_Pin_8)
#define LCD_RST_1()  (GPIOC->BSHR = GPIO_Pin_8)
#define LCD_DC_0()   (GPIOC->BCR  = GPIO_Pin_9)
#define LCD_DC_1()   (GPIOC->BSHR = GPIO_Pin_9)

/* LVGL uses standard RGB colors. MADCTL bit 3 is the only panel-specific
 * red/blue order switch; set to 1 only if a red test field appears blue. */
#define LCD_PANEL_BGR_ORDER  0u

static inline void lcd_spi_delay(void)
{
    /* 144MHz core + direct GPIO writes: target roughly 10..20MHz software SPI.
     * The exact SCL rate depends on compiler optimization and must be measured. */
    __asm volatile ("nop");
}

/* The known-good spi_lvgl/ST7789 project uses SPI mode 0:
 * clock idles low, data is set before the rising sampling edge. */
static void lcd_write_byte(uint8_t value)
{
    uint8_t mask;
    for (mask = 0x80u; mask != 0u; mask >>= 1)
    {
        if (value & mask) LCD_SDA_1(); else LCD_SDA_0();
        lcd_spi_delay();
        LCD_SCL_1();
        lcd_spi_delay();
        LCD_SCL_0();
        lcd_spi_delay();
    }
}

static void lcd_write_cmd(uint8_t cmd)
{
    LCD_CS_0();
    LCD_DC_0();
    lcd_write_byte(cmd);
    LCD_CS_1();
}

static void lcd_write_data8(uint8_t data)
{
    LCD_CS_0();
    LCD_DC_1();
    lcd_write_byte(data);
    LCD_CS_1();
}

static void lcd_write_data(const uint8_t *data, uint8_t count)
{
    LCD_CS_0();
    LCD_DC_1();
    while (count--) lcd_write_byte(*data++);
    LCD_CS_1();
}

static void lcd_gpio_init(void)
{
    GPIO_InitTypeDef gpio;

    printf("[LCD-DBG] GPIO clk\r\n");
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC, ENABLE);

    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Pin = GPIO_Pin_8;
    printf("[LCD-DBG] GPIO_Init(GPIOA)\r\n");
    GPIO_Init(GPIOA, &gpio);
    printf("[LCD-DBG] GPIO_Init(GPIOC)\r\n");
    gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_12;
    GPIO_Init(GPIOC, &gpio);
    printf("[LCD-DBG] initial levels\r\n");

    LCD_CS_1();
    LCD_SCL_0();
    LCD_SDA_1();
    LCD_DC_1();
    LCD_RST_1();
}

static void lcd_backlight_pwm_init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef base;
    TIM_OCInitTypeDef oc;
    uint32_t prescaler;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO |
                           RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* One direct write: disable JTAG (keep SW-DP so SWD download still
     * works) + TIM2_CH1 partial remap 1 -> PA15.  Doing it in a single
     * register write avoids GPIO_PinRemapConfig(), which forces the
     * reserved SWJ_CFG=111 value into PCFR1 and locks the debug port. */
    AFIO->PCFR1 = (AFIO->PCFR1 &
                   ~(AFIO_PCFR1_SWJ_CFG | AFIO_PCFR1_TIM2_REMAP)) |
                  AFIO_PCFR1_SWJ_CFG_JTAGDISABLE |
                  AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP1;

    gpio.GPIO_Pin = GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpio);
    printf("[LCD-DBG] BL: PCFR1=0x%08lX PA15 AF\r\n",
           (unsigned long)AFIO->PCFR1);

    /* 1MHz timer counter and ARR=99 produce approximately 10kHz PWM. */
    prescaler = SystemCoreClock / 1000000u;
    if (prescaler == 0u) prescaler = 1u;
    TIM_TimeBaseStructInit(&base);
    base.TIM_Prescaler = (uint16_t)(prescaler - 1u);
    base.TIM_Period = 99u;
    base.TIM_CounterMode = TIM_CounterMode_Up;
    base.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &base);

    TIM_OCStructInit(&oc);
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = 0u;
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &oc);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void LCD_ST7798_SetBacklight(uint8_t percent)
{
    if (percent > 100u) percent = 100u;
    /* Measured on the assembled board: increasing TIM2_CH1 duty increases
     * the backlight, so the user percentage maps directly to CCR1. */
    TIM_SetCompare1(TIM2, percent);
}

void LCD_ST7798_Backlight(uint8_t on)
{
    LCD_ST7798_SetBacklight(on ? 100u : 0u);
}

void LCD_ST7798_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t p[4];
    lcd_write_cmd(0x2Au);
    p[0] = (uint8_t)(x1 >> 8); p[1] = (uint8_t)x1;
    p[2] = (uint8_t)(x2 >> 8); p[3] = (uint8_t)x2;
    lcd_write_data(p, 4);

    lcd_write_cmd(0x2Bu);
    p[0] = (uint8_t)(y1 >> 8); p[1] = (uint8_t)y1;
    p[2] = (uint8_t)(y2 >> 8); p[3] = (uint8_t)y2;
    lcd_write_data(p, 4);
    lcd_write_cmd(0x2Cu);
}

void LCD_ST7798_WritePixels(const uint16_t *pixels, uint32_t count)
{
    uint16_t c;
    LCD_CS_0();
    LCD_DC_1();
    while (count--)
    {
        c = *pixels++;
        lcd_write_byte((uint8_t)(c >> 8));
        lcd_write_byte((uint8_t)c);
    }
    LCD_CS_1();
}

void LCD_ST7798_Fill(uint16_t rgb565)
{
    uint32_t count = (uint32_t)LCD_ST7798_WIDTH * LCD_ST7798_HEIGHT;
    LCD_ST7798_SetWindow(0, 0, LCD_ST7798_WIDTH - 1, LCD_ST7798_HEIGHT - 1);
    LCD_CS_0();
    LCD_DC_1();
    while (count--)
    {
        lcd_write_byte((uint8_t)(rgb565 >> 8));
        lcd_write_byte((uint8_t)rgb565);
    }
    LCD_CS_1();
}

void LCD_ST7798_Init(void)
{
    static const uint8_t b2[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    static const uint8_t d0[] = {0xA4, 0xA1};
    static const uint8_t e0[] = {0xD0, 0x00, 0x03, 0x09, 0x13, 0x1C, 0x3A,
                                 0x55, 0x48, 0x18, 0x12, 0x0E, 0x19, 0x1E};
    static const uint8_t e1[] = {0xD0, 0x00, 0x03, 0x09, 0x05, 0x25, 0x3A,
                                 0x55, 0x50, 0x3D, 0x1C, 0x1D, 0x1D, 0x1E};

    printf("[LCD-DBG] enter init\r\n");
    lcd_gpio_init();
    printf("[LCD-DBG] gpio init done\r\n");
    lcd_backlight_pwm_init();
    printf("[LCD-DBG] backlight pwm init done\r\n");
    Delay_Ms(120);
    printf("[LCD-DBG] rst pulse start\r\n");
    LCD_RST_0();
    Delay_Ms(120);
    LCD_RST_1();
    Delay_Ms(120);
    printf("[LCD-DBG] init commands\r\n");

    lcd_write_cmd(0x11); Delay_Ms(120); /* Sleep out */
    /* Known-good 320x240 landscape scan direction for this panel. */
    lcd_write_cmd(0x36);
    lcd_write_data8((uint8_t)(0x60u |
                    (LCD_PANEL_BGR_ORDER ? 0x08u : 0x00u)));
    /* The known-good ESP-IDF ST7789 driver selects 16bpp as COLMOD=0x55. */
    lcd_write_cmd(0x3A); lcd_write_data8(0x55); /* RGB565, 16 bits/pixel */
    lcd_write_cmd(0xB2); lcd_write_data(b2, sizeof(b2));
    lcd_write_cmd(0xB7); lcd_write_data8(0x35);
    lcd_write_cmd(0xBB); lcd_write_data8(0x1C);
    lcd_write_cmd(0xC0); lcd_write_data8(0x2C);
    lcd_write_cmd(0xC2); lcd_write_data8(0x01);
    lcd_write_cmd(0xC3); lcd_write_data8(0x0B);
    lcd_write_cmd(0xC4); lcd_write_data8(0x20);
    lcd_write_cmd(0xC6); lcd_write_data8(0x0F);
    lcd_write_cmd(0xD0); lcd_write_data(d0, sizeof(d0));
    lcd_write_cmd(0xE0); lcd_write_data(e0, sizeof(e0));
    lcd_write_cmd(0xE1); lcd_write_data(e1, sizeof(e1));
    /* White UI mode used by this panel revision. */
    lcd_write_cmd(0x20);                 /* Inversion off */
    lcd_write_cmd(0x29); Delay_Ms(20);   /* Display on */
    LCD_ST7798_SetWindow(0, 0, LCD_ST7798_WIDTH - 1, LCD_ST7798_HEIGHT - 1);
    /* Start at the same minimum level used by the UI brightness state.
     * The previous hard-coded 100% overrode g_lcd_brightness at boot. */
    LCD_ST7798_SetBacklight(20u);
}
