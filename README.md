# Recv router CSI example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

The device triggers the router to send packets through the Ping command to obtain the CSI data between the device and the router

## How to use example
Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

### Hardware Required

* A development board with ESP32-C6 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for power supply and programming

### Configure the project

```
idf.py menuconfig
```
Open the project configuration menu (`idf.py menuconfig`) to configure Wi-Fi or Ethernet. See "Establishing Wi-Fi or Ethernet Connection" section in [examples/protocols/README.md](https://github.com/espressif/esp-idf/tree/master/examples/protocols#establishing-wi-fi-or-ethernet-connection) for more details.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

```shell
I (0) cpu_start: App cpu up.
I (288) cpu_start: Pro cpu start user code
I (288) cpu_start: cpu freq: 240000000
I (288) cpu_start: Application information:
I (289) cpu_start: Project name:     csi_recv_router
I (289) cpu_start: App version:      e1dd673-dirty
I (290) cpu_start: Compile time:     Jul 14 2022 18:55:08
I (291) cpu_start: ELF file SHA256:  81680e58e5d0e53c...
I (292) cpu_start: ESP-IDF:          v4.4.1-dirty
I (293) heap_init: Initializing. RAM available for dynamic allocation:
I (293) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (294) heap_init: At 3FFB7310 len 00028CF0 (163 KiB): DRAM
I (295) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (296) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (296) heap_init: At 40093AA4 len 0000C55C (49 KiB): IRAM
I (298) spi_flash: detected chip: generic
I (299) spi_flash: flash io: dio
W (299) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (301) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (331) wifi:wifi driver task: 3ffbfa74, prio:23, stack:6656, core=0
I (332) system_api: Base MAC address is not set
I (332) system_api: read default base MAC address from EFUSE
I (335) wifi:wifi firmware version: 63017e0
I (335) wifi:wifi certification version: v7.0
I (335) wifi:config NVS flash: enabled
I (335) wifi:config nano formating: disabled
I (336) wifi:Init data frame dynamic rx buffer num: 128
I (336) wifi:Init management frame dynamic rx buffer num: 128
I (337) wifi:Init management short buffer num: 32
I (337) wifi:Init dynamic tx buffer num: 32
I (338) wifi:Init static rx buffer size: 2212
I (338) wifi:Init static rx buffer num: 10
I (339) wifi:Init dynamic rx buffer num: 128
I (340) wifi_init: tcpip mbox: 32
I (340) wifi_init: udp mbox: 6
I (340) wifi_init: tcp mbox: 6
I (341) wifi_init: tcp tx win: 5744
I (341) wifi_init: tcp rx win: 5744
I (342) wifi_init: tcp mss: 1440
I (342) wifi_init: WiFi IRAM OP enabled
I (343) wifi_init: WiFi RX IRAM OP enabled
I (344) example_connect: Connecting to TP-LINK_8C81...
I (345) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (433) wifi:mode : sta (ac:67:b2:53:78:d0)
I (434) wifi:enable tsf
I (437) example_connect: Waiting for IP(s)
I (2483) wifi:new:<13,2>, old:<1,0>, ap:<255,255>, sta:<13,2>, prof:1
I (2485) wifi:state: init -> auth (b0)
I (2503) wifi:state: auth -> assoc (0)
I (2509) wifi:state: assoc -> run (10)
I (2510) wifi:connected with TP-LINK_8C81, aid = 1, channel 13, 40D, bssid = 94:d9:b3:80:8c:81
I (2511) wifi:security: Open Auth, phy: bgn, rssi: -28
I (2511) wifi:pm start, type: 1

I (2535) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (3331) esp_netif_handlers: example_connect: sta ip: 192.168.0.107, mask: 255.255.255.0, gw: 192.168.0.1
I (3332) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.0.107
I (4330) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:ae67:b2ff:fe53:78d0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4332) example_connect: Connected to example_connect: sta
I (4332) example_connect: - IPv4 address: 192.168.0.107
I (4333) example_connect: - IPv6 address: fe80:0000:0000:0000:ae67:b2ff:fe53:78d0, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4335) csi_recv_router: got ip:192.168.0.107, gw: 192.168.0.1
I (4346) csi_recv_router: ================ CSI RECV ================
C
```
