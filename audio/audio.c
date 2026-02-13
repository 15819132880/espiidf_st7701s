#include "audio.h"

// 嵌入式PCM音频声明
// extern const uint8_t music_pcm_start[] asm("_binary_canon_pcm_start");
// extern const uint8_t music_pcm_end[]   asm("_binary_canon_pcm_end");
// static const char err_reason[][30] = {"input param is invalid",
//                                     "operation timeout"
//                                     };
static const char *TAG = "AUDIO_STREAM";





#define REMOTE_IP "192.168.40.181"
#define UDP_PORT 12345
#define AUDIO_BUFFER_SIZE 1024





/************************* 网络音频流 *************************/






// 🎧 播放任务
// void udp_receive_and_play_task(void *arg)
// {
//     int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
//     struct sockaddr_in local_addr = {
//         .sin_family = AF_INET,
//         .sin_port = htons(UDP_PORT),
//         .sin_addr.s_addr = htonl(INADDR_ANY),
//     };
//     bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));

//     int16_t buffer[AUDIO_BUFFER_SIZE];
//     size_t bytes_written;
//     bool i2s_started = false;

//     ESP_LOGI(TAG, "UDP playback task started, waiting for data...");

//     while (1) {
//         int len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
//         if (len > 0) {
//             if (!i2s_started) {
//                 i2s_channel_enable(i2s_tx_chan);  // ✅ 第一次收到数据后才启用
//                 i2s_started = true;
//                 ESP_LOGI(TAG, "UDP audio started");
//             }
//             i2s_channel_write(i2s_tx_chan, buffer, len, &bytes_written, portMAX_DELAY);
//         }
//     }

//     close(sock);
//     vTaskDelete(NULL);
// }


void udp_pull_and_play_task(void *arg)
{
    int sock = -1;
    struct sockaddr_in server_addr;
    int16_t buffer[AUDIO_BUFFER_SIZE];
    size_t bytes_written;
    bool i2s_started = false;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(REMOTE_IP);

    ESP_LOGI(TAG, "UDP pull-play task started...");

    while (1) {
        // 如果 socket 无效，重新创建
        if (sock < 0) {
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
            if (sock < 0) {
                ESP_LOGE(TAG, "socket create failed, errno=%d", errno);
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // 设置接收超时 2 秒
            struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

            ESP_LOGI(TAG, "socket created, ready to pull data");
        }

        // 发送 PULL 请求
        const char *pull_cmd = "PULL";
        int ret = sendto(sock, pull_cmd, strlen(pull_cmd), 0,
                        (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret < 0) {
            ESP_LOGW(TAG, "sendto failed, errno=%d, retrying...", errno);
            close(sock);
            sock = -1;
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // 接收数据
        int len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (len > 0) {
            // 确保是偶数字节，避免半个采样点
            if (len % 2 != 0) len--;

            if (!i2s_started) {
                i2s_channel_enable(i2s_tx_chan);
                i2s_started = true;
                ESP_LOGI(TAG, "UDP audio playback started");
            }

            if (len > 0) {
                i2s_channel_write(i2s_tx_chan, buffer, len, &bytes_written, portMAX_DELAY);
            }
        } else {
            // 超时或出错，重建 socket
            ESP_LOGW(TAG, "recvfrom timeout or failed, closing socket, errno=%d", errno);
            close(sock);
            sock = -1;
            i2s_started = false;
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}





// 🎙️ 采集发送任务
void udp_record_and_send_task(void *arg)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    struct sockaddr_in remote_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(UDP_PORT),
        .sin_addr.s_addr = inet_addr(REMOTE_IP),
    };

    int16_t buffer[AUDIO_BUFFER_SIZE];
    size_t bytes_read;

    i2s_channel_enable(i2s_rx_chan);
    ESP_LOGI(TAG, "UDP record task started");

    while (1) {
        esp_err_t ret = i2s_channel_read(i2s_rx_chan, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
        if (ret == ESP_OK && bytes_read > 0) {
            sendto(sock, buffer, bytes_read, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
        }
    }
    close(sock);
    vTaskDelete(NULL);
}



/************************* 主逻辑 *************************/
void app_audio() {


    
    // 初始化硬件

    ESP_ERROR_CHECK(bsp_codec_init());
    
    

    bsp_codec_mute_set(false);       // 取消静音
    // bsp_codec_volume_set(100, NULL);  // 设置音量为80（0~100）
    ESP_LOGI(TAG, "Waiting for client...");



    // 启动双向通信任务
    xTaskCreate(udp_pull_and_play_task, "udp_play", 8192, NULL, 5, NULL);
    //xTaskCreate(udp_record_and_send_task, "udp_send", 8192, NULL, 5, NULL);


}
