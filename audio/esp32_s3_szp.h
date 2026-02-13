#pragma once

#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
//#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "driver/spi_master.h"
#include "driver/i2s_std.h"

#include "driver/sdmmc_host.h"

/******************************************************************************/
/***************************  I2C ↓ *******************************************/
// #define BSP_I2C_SDA           (GPIO_NUM_19)   // SDA引脚
// #define BSP_I2C_SCL           (GPIO_NUM_45)   // SCL引脚

#define BSP_I2C_NUM           (0)            // I2C外设
// #define BSP_I2C_FREQ_HZ       200000         // 200kHz

// esp_err_t bsp_i2c_init(void);   // 初始化I2C接口
void bsp_i2c_init(void);
/***************************  I2C ↑  *******************************************/
/*******************************************************************************/

/***********************************************************/
/***************    IO扩展芯片 ↓   *************************/
#define PCA9557_INPUT_PORT              0x00
#define PCA9557_OUTPUT_PORT             0x01
#define PCA9557_POLARITY_INVERSION_PORT 0x02
#define PCA9557_CONFIGURATION_PORT      0x03

#define PA_EN_GPIO (1U << 6)   // GPIO6
#define PCA9557_SENSOR_ADDR             0x19        /*!< Slave address of the MPU9250 sensor */

#define SET_BITS(_m, _s, _v)  ((_v) ? (_m)|((_s)) : (_m)&~((_s)))
esp_err_t pca9557_set_output_state(uint8_t gpio_bit, uint8_t level);

void pca9557_init(void);
void pa_en(uint8_t level);
/***************    IO扩展芯片 ↑   *************************/
/***********************************************************/

/******************************************************************************/
/***************************   I2S  ↓    **************************************/

/* Example configurations */
// #define EXAMPLE_RECV_BUF_SIZE   (2400)
// #define EXAMPLE_SAMPLE_RATE     (16000)
// #define EXAMPLE_MCLK_MULTIPLE   (384) // If not using 24-bit data width, 256 should be enough
// #define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
// #define EXAMPLE_VOICE_VOLUME    (70)

// /* I2S port and GPIOs */
// #define I2S_NUM         (0)
// #define I2S_MCK_IO      (GPIO_NUM_38)
// #define I2S_BCK_IO      (GPIO_NUM_14)
// #define I2S_WS_IO       (GPIO_NUM_13)
// #define I2S_DO_IO       (GPIO_NUM_45)
// #define I2S_DI_IO       (-1)


/***********************************************************/
/*********************    音频 ↓   *************************/
#define VOLUME_DEFAULT    60
#define ADC_I2S_CHANNEL 3
#define CODEC_DEFAULT_SAMPLE_RATE          (16000)
#define CODEC_DEFAULT_BIT_WIDTH            (16)
#define CODEC_DEFAULT_ADC_VOLUME           (24.0)
#define CODEC_DEFAULT_CHANNEL              (2)

#define BSP_I2S_NUM                  I2S_NUM_1


#define GPIO_I2S_LRCK       (GPIO_NUM_2)
#define GPIO_I2S_MCLK       (GPIO_NUM_41)
#define GPIO_I2S_SCLK       (GPIO_NUM_1)
#define GPIO_I2S_SDIN       (GPIO_NUM_42)
#define GPIO_I2S_DOUT       (GPIO_NUM_40)
#define GPIO_PWR_CTRL       (GPIO_NUM_NC)
extern i2s_chan_handle_t i2s_tx_chan;
extern i2s_chan_handle_t i2s_rx_chan;
esp_err_t bsp_codec_init(void);
esp_err_t bsp_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms);
esp_err_t bsp_codec_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch);
esp_err_t bsp_codec_mute_set(bool enable);
esp_err_t bsp_codec_volume_set(int volume, int *volume_set);
esp_err_t bsp_codec_microphone_volume_set(int volume);
esp_err_t bsp_get_feed_data(bool is_get_raw_channel, int16_t *buffer, int buffer_len);
int bsp_get_feed_channel(void);
/*********************    音频 ↑   *************************/
/***********************************************************/
