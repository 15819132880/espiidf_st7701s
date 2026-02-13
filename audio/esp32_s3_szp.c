#include <stdio.h>
#include "es7210_adc.h" // 包含ES7210 ADC音频编解码器驱动头文件
#include "esp32_s3_szp.h" // 包含ESP32-S3 SZP音频模块头文件


static const char *TAG = "esp32_s3_szp"; // 定义日志标签
// I2S发送通道句柄
// I2S接收通道句柄
i2s_chan_handle_t i2s_tx_chan = NULL;
i2s_chan_handle_t i2s_rx_chan = NULL;
static i2c_master_dev_handle_t pca9557_dev = NULL;
/******************************************************************************/
/***************************  I2C ↓ *******************************************/


// esp_err_t bsp_i2c_init(void)
// {
//     i2c_config_t i2c_conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_io_num = BSP_I2C_SDA,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_io_num = BSP_I2C_SCL,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master.clk_speed = BSP_I2C_FREQ_HZ
//     } // 结束错误检查


//     i2c_param_config(BSP_I2C_NUM, &i2c_conf);

//     return i2c_driver_install(BSP_I2C_NUM, i2c_conf.mode, 0, 0, 0);
// }
i2c_master_bus_handle_t i2c_bus = NULL; // I2C主总线句柄


void bsp_i2c_init(void) { // I2C总线初始化函数

    i2c_master_bus_config_t bus_config = { // I2C主总线配置结构体

        .clk_source = I2C_CLK_SRC_DEFAULT, // 时钟源：默认

        .i2c_port = BSP_I2C_NUM, // I2C端口号

        .scl_io_num = 45, // SCL引脚号

        .sda_io_num = 19, // SDA引脚号

        .glitch_ignore_cnt = 7, // 毛刺抑制计数

        .flags.enable_internal_pullup = true, // 启用内部上拉电阻

     }; // 结束ES7210编解码器配置
 // 结束音频编解码器I2C配置
 // 结束ES8311编解码器配置



    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus); // 创建新的I2C主总线

    if (ret != ESP_OK) { // 检查I2C总线创建是否成功

        ESP_LOGE(TAG, "Failed to create new I2C bus: %s", esp_err_to_name(ret)); // 记录错误日志

    }
}
/***************************  I2C ↑  *******************************************/
/*******************************************************************************/




/***********************************************************/
/***************    IO扩展芯片 ↓   *************************/

esp_err_t pca9557_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    if (pca9557_dev == NULL) return ESP_ERR_INVALID_STATE;
    return i2c_master_transmit_receive(pca9557_dev, &reg_addr, 1, data, len, 1000);
}

esp_err_t pca9557_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    if (pca9557_dev == NULL) return ESP_ERR_INVALID_STATE;
    uint8_t write_buf[2] = { reg_addr, data };
    return i2c_master_transmit(pca9557_dev, write_buf, sizeof(write_buf), 1000);
}


// 初始化PCA9557 IO扩展芯片


void pca9557_init(void)
{
    if (pca9557_dev == NULL) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,   // 7位地址
            .device_address  = PCA9557_SENSOR_ADDR,  // 0x19
            .scl_speed_hz    = 400000,               // 根据总线速率设置
        };
        esp_err_t err = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &pca9557_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add PCA9557 device: %s", esp_err_to_name(err));
            return;
        }
    }

    // 初始化默认输出
    pca9557_set_output_state(PA_EN_GPIO, 1);
}



// 设置PCA9557芯片的某个IO引脚输出高低电平
esp_err_t pca9557_set_output_state(uint8_t gpio_bit, uint8_t level)
{
    uint8_t data;
    esp_err_t res = ESP_FAIL;

    pca9557_register_read(PCA9557_OUTPUT_PORT, &data, 1);
    res = pca9557_register_write_byte(PCA9557_OUTPUT_PORT, SET_BITS(data, gpio_bit, level));

    return res;
}



// 控制 PCA9557_PA_EN 引脚输出高低电平 参数0输出低电平 参数1输出高电平 
void pa_en(uint8_t level)
{
    pca9557_set_output_state(PA_EN_GPIO, level);
}



/***************    IO扩展芯片 ↑   *************************/
/***********************************************************/




/***********************************************************/
/*********************    音频 ↓   *************************/
static esp_codec_dev_handle_t play_dev_handle;    // 扬声器设备句柄
static esp_codec_dev_handle_t record_dev_handle;  // 麦克风设备句柄



static const audio_codec_data_if_t *i2s_data_if = NULL;  /* 编解码器数据接口 */



// I2S总线初始化函数
esp_err_t bsp_audio_init(void)

{
    esp_err_t ret = ESP_FAIL; // 初始化返回状态为失败

    if (i2s_tx_chan && i2s_rx_chan) { // 如果I2S发送和接收通道已存在

        /* 音频已初始化 */

        return ESP_OK; // 直接返回成功

    }

    /* 设置I2S外设 */

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(BSP_I2S_NUM, I2S_ROLE_MASTER); // I2S通道配置，设置为BSP I2S端口和主模式

    chan_cfg.auto_clear = true; // 自动清除DMA缓冲区中的旧数据
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &i2s_tx_chan, &i2s_rx_chan)); // 创建新的I2S通道


    /* Setup I2S channels */
    // const i2s_std_config_t std_cfg_default = {
    //     .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),   // 采样率16000
    //     .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(32, I2S_SLOT_MODE_STEREO),  // 32位 2通道
    //     .gpio_cfg = { 
    //         .mclk = GPIO_I2S_MCLK, 
    //         .bclk = GPIO_I2S_SCLK, 
    //         .ws   = GPIO_I2S_LRCK, 
    //         .dout = GPIO_I2S_DOUT, 
    //         .din  = GPIO_I2S_SDIN,
    //         .invert_flags = {
    //             .mclk_inv = false,
    //             .bclk_inv = false,
    //             .ws_inv = false,
    //         } // 结束GPIO配置

    //     },


    i2s_std_config_t std_cfg_default = { // I2S标准模式配置结构体

        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000), // 时钟配置：默认16000Hz采样率

        .slot_cfg = { // 时隙配置

            .data_bit_width = 32, // 数据位宽：32位

            .slot_bit_width = 32, // 时隙位宽：32位

            .slot_mode = I2S_COMM_MODE_TDM,   // 时隙模式：TDM模式

            .slot_mask = I2S_TDM_SLOT0 | I2S_TDM_SLOT1 | I2S_TDM_SLOT2,  // 时隙掩码：启用0、1、2三个时隙

            .ws_width = 32, // 字选择信号宽度：32位

            .bit_shift = true,  // 位移：启用（替代msb_right）

            .ws_pol = false // 字选择信号极性：低电平有效

        },
        .gpio_cfg = { // GPIO配置

            .mclk = GPIO_I2S_MCLK, // MCLK引脚

            .bclk = GPIO_I2S_SCLK, // BCLK引脚

            .ws = GPIO_I2S_LRCK, // WS（LRCK）引脚

            .dout = GPIO_I2S_DOUT, // DOUT引脚

            .din = GPIO_I2S_SDIN, // DIN引脚

            .invert_flags = { // 反转标志

                .mclk_inv = false, // MCLK不反转

                .bclk_inv = false, // BCLK不反转

                .ws_inv = false, // WS不反转

            } // 结束反转标志配置

        }

    };

    if (i2s_tx_chan != NULL) { // 如果I2S发送通道不为空

        ESP_GOTO_ON_ERROR(i2s_channel_init_std_mode(i2s_tx_chan, &std_cfg_default), err, TAG, "I2S channel initialization failed"); // 初始化I2S发送通道为标准模式

        ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_tx_chan), err, TAG, "I2S enabling failed"); // 使能I2S发送通道

    }
    if (i2s_rx_chan != NULL) { // 如果I2S接收通道不为空

        ESP_GOTO_ON_ERROR(i2s_channel_init_std_mode(i2s_rx_chan, &std_cfg_default), err, TAG, "I2S channel initialization failed"); // 初始化I2S接收通道为标准模式

        ESP_GOTO_ON_ERROR(i2s_channel_enable(i2s_rx_chan), err, TAG, "I2S enabling failed"); // 使能I2S接收通道

    }

    audio_codec_i2s_cfg_t i2s_cfg = { // 音频编解码器I2S配置结构体

        .port = BSP_I2S_NUM, // I2S端口号

        .rx_handle = i2s_rx_chan, // 接收通道句柄

        .tx_handle = i2s_tx_chan, // 发送通道句柄

    };
    i2s_data_if = audio_codec_new_i2s_data(&i2s_cfg); // 创建新的I2S数据接口

    if (i2s_data_if == NULL) { // 如果I2S数据接口未初始化

        goto err; // 跳转到错误处理标签

    } // 结束I2S数据接口创建检查


    return ESP_OK;

err: // 错误处理标签

    if (i2s_tx_chan) { // 如果发送通道存在

        i2s_del_channel(i2s_tx_chan); // 删除发送通道

    }
    if (i2s_rx_chan) { // 如果接收通道存在

        i2s_del_channel(i2s_rx_chan); // 删除接收通道

    }

    return ret;
}

// 初始化音频输出芯片（扬声器）
esp_codec_dev_handle_t bsp_audio_codec_speaker_init(void)

{
    if (i2s_data_if == NULL) {
        /* 配置I2S外设和功率放大器 */

        ESP_ERROR_CHECK(bsp_audio_init()); // 调用音频初始化函数

    }
    assert(i2s_data_if); // 断言I2S数据接口已初始化


    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio(); // 创建GPIO接口


    audio_codec_i2c_cfg_t i2c_cfg = { // 音频编解码器I2C配置结构体

        .port = BSP_I2C_NUM, // I2C端口号

        .addr = 0x18, // I2C设备地址

         .bus_handle = i2c_bus, // I2C总线句柄
 // I2C总线句柄

    };
    const audio_codec_ctrl_if_t *i2c_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg); // 创建I2C控制接口 // 创建I2C控制接口
     assert(i2c_ctrl_if); // 断言I2C控制接口已初始化
 // 断言I2C控制接口已初始化



    esp_codec_dev_hw_gain_t gain = { // 硬件增益配置结构体
         .pa_voltage = 5.0, // 功率放大器电压

        .codec_dac_voltage = 3.3, // 编解码器DAC电压
        .pa_gain = 0.0f  // 功率放大器增益（例如放大10dB，按需调整）
    };

    es8311_codec_cfg_t es8311_cfg = { // ES8311编解码器配置结构体
         .ctrl_if = i2c_ctrl_if, // 控制接口
 // 控制接口

         .gpio_if = gpio_if, // GPIO接口

        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC, // 编解码器工作模式：DAC模式
         .pa_pin = GPIO_PWR_CTRL, // 功率放大器控制引脚

         .pa_reverted = false, // 功率放大器是否反转

         .master_mode = false, // 主模式

         .use_mclk = true, // 使用MCLK

         .digital_mic = false, // 数字麦克风

         .invert_mclk = false, // 不反转MCLK

         .invert_sclk = false, // 不反转SCLK

         .hw_gain = gain, // 硬件增益

    };
    const audio_codec_if_t *es8311_dev = es8311_codec_new(&es8311_cfg); // 创建ES8311编解码器句柄

    if (es8311_dev == NULL) { // 如果扬声器句柄创建失败
        ESP_LOGE(TAG, "create ES8311 codec speaker failed"); // 记录错误日志
        return NULL; // 返回空
    } // 结束扬声器句柄创建检查


    esp_codec_dev_cfg_t codec_dev_cfg = { // 编解码器设备配置结构体
        .dev_type = ESP_CODEC_DEV_TYPE_OUT, // 设备类型：输出设备
        .codec_if = es8311_dev, // 编解码器接口
         .data_if = i2s_data_if, // 数据接口

    };
    play_dev_handle = esp_codec_dev_new(&codec_dev_cfg); // 创建播放设备句柄
    if (play_dev_handle == NULL) { // 如果播放设备句柄创建失败
        ESP_LOGE(TAG, "create play device failed"); // 记录错误日志
        return NULL; // 返回空
    } // 结束播放设备句柄创建检查
    return play_dev_handle; // 返回播放设备句柄
}

// 初始化音频输入芯片（麦克风）
esp_codec_dev_handle_t bsp_audio_codec_microphone_init(void)

{
    if (i2s_data_if == NULL) {
        /* 配置I2S外设和功率放大器 */

        ESP_ERROR_CHECK(bsp_audio_init());
    }
    assert(i2s_data_if);

    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = BSP_I2C_NUM,
        .addr = 0x41, // I2C设备地址
         .bus_handle = i2c_bus,
    };
    const audio_codec_ctrl_if_t *i2c_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    assert(i2c_ctrl_if);

    es7210_codec_cfg_t es7210_cfg = { // ES7210编解码器配置结构体
        .ctrl_if = i2c_ctrl_if,
        .mic_selected = ES7210_SEL_MIC1 | ES7210_SEL_MIC2 | ES7210_SEL_MIC3 |ES7210_SEL_MIC4, // 选择麦克风：MIC1、MIC2、MIC3、MIC4
    };
    const audio_codec_if_t *es7210_dev = es7210_codec_new(&es7210_cfg); // 创建ES7210编解码器句柄
     assert(es7210_dev); // 断言ES7210编解码器句柄已初始化


    esp_codec_dev_cfg_t codec_es7210_dev_cfg = { // 编解码器设备配置结构体
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .codec_if = es7210_dev,
        .data_if = i2s_data_if,
    };
    return esp_codec_dev_new(&codec_es7210_dev_cfg);
}


// 设置采样率
esp_err_t bsp_codec_set_fs(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    esp_err_t ret = ESP_OK;

    esp_codec_dev_sample_info_t fs = {
        .sample_rate = rate,
        .channel = ch,
        .bits_per_sample = bits_cfg,
    };
    
    if (play_dev_handle) {
        ret = esp_codec_dev_close(play_dev_handle);
    }
    if (record_dev_handle) {
        ret |= esp_codec_dev_close(record_dev_handle);
        ret |= esp_codec_dev_set_in_gain(record_dev_handle, CODEC_DEFAULT_ADC_VOLUME);
    }

    if (play_dev_handle) {
        ret |= esp_codec_dev_open(play_dev_handle, &fs);
    }
    if (record_dev_handle) {
        ret |= esp_codec_dev_open(record_dev_handle, &fs);
    }
    return ret;
}

// 音频芯片初始化
esp_err_t bsp_codec_init(void)
{
    play_dev_handle = bsp_audio_codec_speaker_init();
    assert((play_dev_handle) && "play_dev_handle not initialized");

    record_dev_handle = bsp_audio_codec_microphone_init();
    assert((record_dev_handle) && "record_dev_handle not initialized");

    bsp_codec_set_fs(CODEC_DEFAULT_SAMPLE_RATE, CODEC_DEFAULT_BIT_WIDTH, CODEC_DEFAULT_CHANNEL);
    esp_codec_dev_set_out_vol(play_dev_handle, VOLUME_DEFAULT);

    return ESP_OK;
}

// 播放音乐
esp_err_t bsp_i2s_write(void *audio_buffer, size_t len, size_t *bytes_written, uint32_t timeout_ms)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_write(play_dev_handle, audio_buffer, len);
    ESP_LOGI(TAG, "Writing %d bytes to I2S", len);
    *bytes_written = len;
    return ret;
}

// 设置静音与否
// 设置静音与否
esp_err_t bsp_codec_mute_set(bool enable)
{
    esp_err_t ret = ESP_OK;

    // 先控制编解码器的静音
    ret = esp_codec_dev_set_out_mute(play_dev_handle, enable);

    // // 再控制功放芯片的使能（PA_EN）
    if (enable) {
        // enable = true 表示静音 -> 关掉功放
        pa_en(0);   // 或者根据你的硬件逻辑，0 关，1 开
    } else {
        // enable = false 表示取消静音 -> 打开功放
        pa_en(1);
    }

    return ret;
}

// 设置喇叭音量
esp_err_t bsp_codec_volume_set(int volume, int *volume_set)
{
    esp_err_t ret = ESP_OK;
    float v = volume;
    ret = esp_codec_dev_set_out_vol(play_dev_handle, (int)v);
    return ret;
}
int bsp_get_feed_channel(void)
{
    return ADC_I2S_CHANNEL;
}

esp_err_t bsp_codec_microphone_volume_set(int volume)
{
    esp_err_t ret = ESP_OK;
    ret = esp_codec_dev_set_in_gain(record_dev_handle, (float)volume);
    return ret;
}

esp_err_t bsp_get_feed_data(bool is_get_raw_channel, int16_t *buffer, int buffer_len)
{
    esp_err_t ret = ESP_OK;

    int audio_chunksize = buffer_len / (sizeof(int16_t) * ADC_I2S_CHANNEL);

    ret = esp_codec_dev_read(record_dev_handle, (void *)buffer, buffer_len);



    if (!is_get_raw_channel) {
        for (int i = 0; i < audio_chunksize; i++) {
            int16_t ref = buffer[4 * i + 0];              // MIC1
            int16_t ch2 = buffer[4 * i + 1];              // MIC2
            int16_t ch4 = buffer[4 * i + 3];              // MIC4（接地，可能用于消噪）
            buffer[3 * i + 0] = ch2;                       // MIC2
            buffer[3 * i + 1] = ref;                       // MIC4
            buffer[3 * i + 2] = ch4;                       // MIC1作为第3通道
        }
        // 按麦克风地址打印数据，示例打印前三个样本
        ESP_LOGI("MIC_TEST", "MIC2 sample data: %d, %d, %d", buffer[1], buffer[5], buffer[9]);
        ESP_LOGI("MIC_TEST", "MIC4 sample data: %d, %d, %d", buffer[3], buffer[7], buffer[11]);
        ESP_LOGI("MIC_TEST", "MIC1 sample data: %d, %d, %d", buffer[0], buffer[4], buffer[8]);
    }


    return ret;
}
/*********************    音频 ↑   *************************/
/***********************************************************/



