/* Get recv router csi

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "nvs_flash.h"

#include "esp_mac.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_now.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "ping/ping_sock.h"

#include "protocol_examples_common.h"

#define CONFIG_SEND_FREQUENCY      10

static const char *TAG = "csi_recv_router";

#define TAG "BASE_MAC"

#ifdef CONFIG_BASE_MAC_STORED_OTHER_EXTERNAL_STORAGE

static esp_err_t external_storage_mac_get(uint8_t *mac)
{
    /* This function simulates getting a base MAC address from external storage
     * by simply setting the base MAC to an arbitrary address. Users should
     * re-implement this function to access external storage (e.g. flash, EEPROM) */
    uint8_t external_storage_mac_addr[6] = { 0x62, 0xaf, 0x09, 0x51, 0xc9, 0x69 };
    if (mac == NULL) {
        ESP_LOGE(TAG, "The mac parameter is NULL");
        abort();
    }

    memcpy(mac, external_storage_mac_addr, 6);
    return ESP_OK;
}
#endif//CONFIG_BASE_MAC_STORED_OTHER_EXTERNAL_STORAGE

static void wifi_csi_rx_cb(void *ctx, wifi_csi_info_t *info)
{
    if (!info || !info->buf) {
        ESP_LOGW(TAG, "<%s> wifi_csi_cb", esp_err_to_name(ESP_ERR_INVALID_ARG));
        return;
    }

    if (memcmp(info->mac, ctx, 6)) {
        return;
    }

    static int s_count = 0;
    const wifi_pkt_rx_ctrl_t *rx_ctrl = &info->rx_ctrl;

    if (!s_count) {
        ESP_LOGI(TAG, "================ CSI RECV ================");
        ets_printf("type,seq,mac,rssi,rate,noise_floor,channel,local_timestamp,sig_len,rx_state,len,first_word,data\n");
    }

    /** Only LLTF sub-carriers are selected. */
    info->len = 128;

    printf("CSI_DATA,%d," MACSTR ",%d,%d,%d,%d,%d,%d,%d",
            s_count++, MAC2STR(info->mac), rx_ctrl->rssi, rx_ctrl->rate,
            rx_ctrl->noise_floor, rx_ctrl->channel,
            rx_ctrl->timestamp, rx_ctrl->sig_len, rx_ctrl->rx_state);

    printf(",%d,%d,\"[%d", info->len, info->first_word_invalid, info->buf[0]);

    for (int i = 1; i < info->len; i++) {
        printf(",%d", info->buf[i]);
    }

    printf("]\"\n");

}

static void wifi_csi_init()
{
    /**
     * @brief In order to ensure the compatibility of routers, only LLTF sub-carriers are selected.
     */

    static wifi_ap_record_t s_ap_info = {0};
    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&s_ap_info));
    //ESP_ERROR_CHECK(esp_wifi_set_csi_config(&csi_config));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(wifi_csi_rx_cb, s_ap_info.bssid));
    ESP_ERROR_CHECK(esp_wifi_set_csi(true));
}

static esp_err_t wifi_ping_router_start()
{
    static esp_ping_handle_t ping_handle = NULL;

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.count             = 0;
    ping_config.interval_ms       = 1000 / CONFIG_SEND_FREQUENCY;
    ping_config.task_stack_size   = 3072;
    ping_config.data_size         = 1;

    esp_netif_ip_info_t local_ip;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &local_ip);
    ESP_LOGI(TAG, "got ip:" IPSTR ", gw: " IPSTR, IP2STR(&local_ip.ip), IP2STR(&local_ip.gw));
    ping_config.target_addr.u_addr.ip4.addr = ip4_addr_get_u32(&local_ip.gw);
    ping_config.target_addr.type = ESP_IPADDR_TYPE_V4;

    esp_ping_callbacks_t cbs = { 0 };
    esp_ping_new_session(&ping_config, &cbs, &ping_handle);
    esp_ping_start(ping_handle);

    return ESP_OK;
}

void app_main()
{
    //Get the base MAC address from different sources
    uint8_t base_mac_addr[6] = {0};
    esp_err_t ret = ESP_OK;


    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

// * If you need to set up MAC address, uncomment the following code
/*
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
*/
#ifdef CONFIG_BASE_MAC_STORED_EFUSE_BLK3
    //Get base MAC address from EFUSE BLK3
    ret = esp_read_mac(base_mac_addr, ESP_MAC_EFUSE_CUSTOM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get base MAC address from EFUSE BLK3. (%s)", esp_err_to_name(ret));
#ifdef CONFIG_BASE_MAC_STORED_EFUSE_BLK3_ERROR_ABORT
        ESP_LOGE(TAG, "Aborting");
        abort();
#else
        ESP_LOGI(TAG, "Defaulting to base MAC address in BLK0 of EFUSE");
        ESP_ERROR_CHECK(esp_read_mac(base_mac_addr, ESP_MAC_EFUSE_FACTORY));
        ESP_LOGI(TAG, "Base MAC Address read from EFUSE BLK0");
#endif//CONFIG_BASE_MAC_STORED_EFUSE_BLK3_ERROR_ABORT
    } else {
        ESP_LOGI(TAG, "Base MAC Address read from EFUSE BLK3");
    }
#elif defined(CONFIG_BASE_MAC_STORED_OTHER_EXTERNAL_STORAGE)
    //Get base MAC address from other external storage, or set by software
    ret = external_storage_mac_get(base_mac_addr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get base MAC address from external storage. (%s)", esp_err_to_name(ret));
        ESP_LOGE(TAG, "Aborting");
        abort();
    } else {
        ESP_LOGI(TAG, "Base MAC Address read from external storage");
    }
#else
    //Get base MAC address from EFUSE BLK0(default option)
    ret = esp_read_mac(base_mac_addr, ESP_MAC_EFUSE_FACTORY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get base MAC address from EFUSE BLK0. (%s)", esp_err_to_name(ret));
        ESP_LOGE(TAG, "Aborting");
        abort();
    } else {
        ESP_LOGI(TAG, "Base MAC Address read from EFUSE BLK0");
    }
#endif

    //Set the base MAC address using the retrieved MAC address
    ESP_LOGI(TAG, "Using \"0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\" as base MAC address",
             base_mac_addr[0], base_mac_addr[1], base_mac_addr[2], base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);
    esp_iface_mac_addr_set(base_mac_addr, ESP_MAC_BASE);

    //Get the derived MAC address for each network interface
    uint8_t derived_mac_addr[6] = {0};

#ifdef CONFIG_ESP_WIFI_ENABLED
    //Get MAC address for WiFi Station interface
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_STA));
    ESP_LOGI("WIFI_STA MAC", "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);

    //Get MAC address for SoftAp interface
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_SOFTAP));
    ESP_LOGI("SoftAP MAC", "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
#endif // CONFIG_ESP_WIFI_ENABLED

#ifdef CONFIG_ESP_MAC_ADDR_UNIVERSE_BT
    //Get MAC address for Bluetooth
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_BT));
    ESP_LOGI("BT MAC", "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
#endif // CONFIG_ESP_MAC_ADDR_UNIVERSE_BT

    //Get MAC address for Ethernet
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_ETH));
    ESP_LOGI("Ethernet MAC", "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);

    ESP_LOGI("Ethernet MAC", "Overwrite Ethernet MAC");
    base_mac_addr[5] += 6;
    ESP_ERROR_CHECK(esp_iface_mac_addr_set(base_mac_addr, ESP_MAC_ETH));

    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_ETH));
    ESP_LOGI("New Ethernet MAC", "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);

#if CONFIG_SOC_IEEE802154_SUPPORTED
    uint8_t mac_ext[2] = {0};
    ESP_ERROR_CHECK(esp_read_mac(mac_ext, ESP_MAC_EFUSE_EXT));
    ESP_LOGI("MAC_EXT", "0x%x, 0x%x", mac_ext[0], mac_ext[1]);
    uint8_t eui64[8] = {0};
    ESP_ERROR_CHECK(esp_read_mac(eui64, ESP_MAC_IEEE802154));
    ESP_LOGI("IEEE802154", "0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
             eui64[0], eui64[1], eui64[2], eui64[3], eui64[4], eui64[5], eui64[6], eui64[7]);
#endif

    /**
     * @brief This helper function configures Wi-Fi, as selected in menuconfig.
     *        Read "Establishing Wi-Fi Connection" section in esp-idf/examples/protocols/README.md
     *        for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    wifi_csi_init();
    wifi_ping_router_start();
}
