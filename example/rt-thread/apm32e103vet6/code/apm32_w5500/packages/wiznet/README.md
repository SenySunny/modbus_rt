# WIZnet

[中文页](README_ZH.md) | English

## 1. Introduction

The WIZnet software package is a porting implementation of RT-Thread based on the WIZnet official website [ioLibrary_Driver](https://github.com/Wiznet/ioLibrary_Driver) code base, and currently only supports W5500 devices. On the basis of the original code library function, this software package docks with the RT-Thread SAL socket abstraction layer, realizes the support for standard BSD Socket APIs, is perfectly compatible with a variety of software packages and network functions, and improves the compatibility of WIZnet devices.

### 1.1 Directory structure

The WIZnet software package directory structure is as follows:

```
wiznet
├───inc           // RT_Thread transplant header file
├───iolibrary     // WIZnet official library file
│ └───Ethernet    // WIZnet official Socket APIs and WIZCHIP driver
│ │ └───W5500     // WIZCHIP driver
│ │ wizchip_conf.c // Socket configuration file
│ │ wizchip_socket.c // Socket APIs file
│ └───Internet // WIZnet official network function realization
│ │ └───DHCP // DHCP function implementation
│ └───────DNS // DNS function realization
├───src // RT_Thread transplant source code file
│ └───wiz_af_inet.c // WIZnet BSD Socket registered to SAL
│ │ wiz_device.c // WIZnet device initialization
│ │ wiz_ping.c // WIZnet device Ping command realization
│ │ wiz_socket.c // WIZnet BSD Socket APIs implementation
│ └───wiz.c // WIZnet initialization (device initialization, network initialization)
│ LICENSE // package license
│ README.md // Software package instructions
└───SConscript // RT-Thread default build script
```


### 1.2 License

The WIZnet software package complies with the Apache-2.0 license, see the LICENSE file for details.

### 1.3 Dependency

- RT-Thread 4.0.1+
- SAL component
- netdev component
- SPI driver: WIZnet devices use SPI for data communication, which requires the support of the system SPI driver framework;
- PIN driver: used to handle device reset and interrupt pins;

## 2. Get the software package

To use the WIZnet software package, you need to select it in the RT-Thread package management. The specific path is as follows:

```shell
WIZnet: WIZnet TCP/IP chips SAL framework implement
        WIZnet device type (W5500) --->
        WIZnet device configure --->
            (spi30) SPI device name
            (10) Reset PIN number
            (11) IRQ PIN number
  [] Enable alloc IP address through DHCP
            WIZnet network configure --->
                (192.168.1.10) IPv4: IP address
                (192.168.1.1) IPv4: Gateway address
                (255.255.255.0) IPv4: Mask address
  [] Enable Ping utility
  [] Enable debug log output
        Version (latest) --->
```

**WIZnet device type**: Configure the supported device type (currently only supports W5500 devices)

**WIZnet device configure**: configure the parameters of the device used

- **SPI device name**: Configure the name of the device using SPI (note that it needs to be set to **non-SPI bus device**)

- **Reset PIN number**: Configure the reset pin number connected to the device (modified according to the actual pin number used)

- **IRQ PIN number**: Configure the interrupt pin number of the device connection (same as above)

**Enable alloc IP address through DHCP**: Configure whether to use DHCP to allocate IP addresses (enabled by default)

**WIZnet network configure**: If you do not enable the DHCP function, you need to configure the statically connected IP address, gateway and subnet mask

**Enable Ping utility**: Configure to enable Ping command (enabled by default)

**Enable debug log output**: Configure to enable debug log display

**Version**: software package version selection

## 3. Use the software package

The initialization function of WIZnet software package is as follows:

```c
int wiz_init(void);
```

This function supports component initialization. If the automatic component initialization function is enabled, the application layer does not need to call this function. The main functions of the function are:

- Set the default MAC address;
- Device configuration and initialization (configure SPI device, configure reset and interrupt pins);
- Network configuration and initialization (DHCP allocation of IP address, configuration of socket parameters);
- Register the implemented BSD Socket APIs to the SAL socket abstraction layer to complete WIZnet device adaptation;

Each WIZnet device needs a unique MAC address. The user can call the following function in the application layer program to set the MAC address of the WIZnet device. If this function is not called, the device will use the default MAC address. The default MAC address is `00-E0-81 -DC-53-1A` (Note: If there are devices with the same MAC address in the same LAN, it may cause the device network to be abnormal).

```c
int wiz_set_mac(const char *mac);
```

After the device is powered on and initialized, the device's MAC address is successfully set, and then you can enter the command `wiz_ifconfig` in FinSH to view the device's IP address, MAC address and other network information, as shown below:

```shell
msh />ifconfig
network interface device: W5500 (Default)    ## Device name
MTU: 1472                                    ## Network maximum transmission unit
MAC: 00 e0 81 dc 53 1a                       ## Device MAC address
FLAGS: UP LINK_UP INTERNET_UP                ## Device flag
ip address: 192.168.12.26                    ## Device IP address
gw address: 192.168.10.1                     ## Device gateway address
net mask: 255.255.0.0                        ## Device subnet mask
dns server #0: 192.168.10.1                  ## DNS server address 0
dns server #1: 0.0.0.0                       ## DNS server address 1
```

After obtaining the IP address successfully, if the Ping command function is enabled, you can enter the command `ping + domain name address` in FinSH to test the network connection status, as shown below:

```shell
msh />wiz_ping baidu.com
32 bytes from 220.181.57.216 icmp_seq=0 ttl=128 time=31 ticks
32 bytes from 220.181.57.216 icmp_seq=1 ttl=128 time=31 ticks
32 bytes from 220.181.57.216 icmp_seq=2 ttl=128 time=32 ticks
32 bytes from 220.181.57.216 icmp_seq=3 ttl=128 time=32 ticks
```

The normal test of the `ping` command indicates that the WIZnet device is successfully connected to the network, and then you can use the standard BSD Socket APIs abstracted by SAL (Socket Abstraction Layer) for network development (MQTT, HTTP, MbedTLS, NTP, Iperf, etc.), WIZnet software package The supported protocol cluster types are: the primary protocol cluster is **AF_WIZ**, and the secondary protocol cluster is **AF_INET** (for specific differences and usage, please refer to [SAL Programming Guide](https://www.rt-thread.org/document/site/submodules/rtthread-manual-doc/zh/1chapters/13-chapter_sal/) ).

## 4. Common problems

- Assertion problem during SPI device initialization

  ```shell
  (wiz_device->parent.type == RT_Device_Class_SPIDevice) assertion failed at function:wiz_spi_init, line number:126
  ```

  The above assertion problem occurs. The possible reason is that the name of the SPI device used by WIZnet in ENV is incorrectly filled. Please distinguish the relationship between SPI DEVICE and SPI BUS. If there is no SPI device or only SPI bus device in the BSP project, you need to manually mount the SPI device to the SPI bus in the driver and correctly configure the SPI device name used in the WIZnet software package.

- The latest version of WIZnet software package has been supported as a server server mode (not supported before V1.1.0).

- WIZNet software package initialization error ```[E/wiz.dev] You should attach [wiznet] into SPI bus firstly.``` error is caused by not mounting the winzet device to the SPI bus; please refer to the wiz_init function Note to solve the problem of package initialization failure.

- When using the previous code in the RT-Thread repository, please compare ```[components/net/sal_socket/src/sal_socket.c]```, especially about the content of [PR](https://github.com/RT-Thread/rt-thread/pull/3534/files), pay attention to the content of sal_closesocket . When you always fail to apply for socket(-1), please make sure that the RT-Thread code you are using is the same as the [PR](https://github.com/RT-Thread/rt-thread/pull/3534 /files).

- When applying for a socket, the error is ```0x22```. Note that the development branch of wiznet is in the master version or a version greater than V1.1.0. Please pay attention to the execution order of ```wiz_socket_init()```, because the ```sal_check_netdev_internet_up``` networking detection function will actively apply for a socket to determine whether the w5500 has network capabilities, and network status changes will cause ```sal_check_netdev_internet_up``` was called, causing ```0x22``` error.


## 5. Matters needing attention

- When obtaining the software package, you need to pay attention to the correct configuration of the SPI device name, reset pin number and interrupt pin number used;
- After the initialization is complete, it is recommended to use the `wiz_set_mac()` function to set the device MAC address to prevent conflicts with the default MAC address;

## 6. Contact & Thanks

- Maintenance: RT-Thread development team
- Homepage: https://github.com/RT-Thread-packages/wiznet
