#include "pca10059.h"
#include "pwm.h"
#include "nrf_drv_pwm.h"
#include "log.h"
#include "flash.h"

#define COUNT_1KHz 1000
#define MAX_TIME_PWM_CICLE 20000

static nrf_drv_pwm_t m_pwm = NRF_DRV_PWM_INSTANCE(0);
static uint16_t const              m_led_rgb_top  = MAX_TIME_PWM_CICLE;
static uint8_t                     m_led_rgb_phase;
static nrf_pwm_values_individual_t m_led_rgb_seq_values;
static nrf_pwm_sequence_t const    m_led_rgb_seq =
{
    .values.p_individual = &m_led_rgb_seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(m_led_rgb_seq_values),
    .repeats             = 0,
    .end_delay           = 0
};

uint32_t r = 0;   
uint32_t g = 0;     
uint32_t b = 0;

void rgb_on(uint32_t red, uint32_t green, uint32_t blue)
{
    uint32_t step = MAX_TIME_PWM_CICLE / 255;
    r = red * step;
    g = green * step;
    b = blue * step;
    NRF_LOG_INFO("%d R_on %d",red, r);
    NRF_LOG_INFO("%d G_on %d",green, g);
    NRF_LOG_INFO("%d B_on %d",blue, b);
}

void led_rgb_handler(nrf_drv_pwm_evt_type_t event_type)
{
    if (event_type == NRF_DRV_PWM_EVT_FINISHED)
    {
        m_led_rgb_seq_values.channel_1 = r;
        m_led_rgb_seq_values.channel_2 = g;
        m_led_rgb_seq_values.channel_3 = b;
    }
}

void pwm_rgb_init(void)
{
        nrf_drv_pwm_config_t const config =
    {
        .output_pins =
        {
            NRF_DRV_PWM_PIN_NOT_USED, // channel 0
            BSP_LED_1 | NRF_DRV_PWM_PIN_INVERTED, // channel 1
            BSP_LED_2 | NRF_DRV_PWM_PIN_INVERTED, // channel 2
            BSP_LED_3 | NRF_DRV_PWM_PIN_INVERTED  // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,    //< Приоритет прерываний
        .base_clock   = NRF_PWM_CLK_1MHz,           //< Базовая тактовая частота
        .count_mode   = NRF_PWM_MODE_UP,            //< Режим работы счетчика генератора импульсов.
        .top_value    = m_led_rgb_top,              //< Значение, до которого отсчитывается счетчик генератора импульсов.
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,   //< Режим загрузки данных последовательности из оперативной памяти.
        .step_mode    = NRF_PWM_STEP_AUTO          //< Режим продвижения активной последовательности.
    };
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm, &config, led_rgb_handler));

    m_led_rgb_seq_values.channel_0 = 0;
    m_led_rgb_seq_values.channel_1 = 0;
    m_led_rgb_seq_values.channel_2 = 0;
    m_led_rgb_seq_values.channel_3 = 0;
    m_led_rgb_phase                = 1;

    (void)nrf_drv_pwm_simple_playback(&m_pwm, &m_led_rgb_seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}