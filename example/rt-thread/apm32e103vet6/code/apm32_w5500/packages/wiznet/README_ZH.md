# WIZnet

中文页 | [English](README.md)

## 1、介绍

WIZnet 软件包是 RT-Thread 基于 WIZnet 官网 [ioLibrary_Driver](https://github.com/Wiznet/ioLibrary_Driver) 代码库的移植实现，目前只支持 W5500 设备。该软件包在原代码库功能的基础上，对接 RT-Thread SAL 套接字抽象层，实现对标准 BSD Socket APIs 的支持，完美的兼容多种软件包和网络功能实现，提高 WIZnet 设备兼容性。

### 1.1 目录结构

WIZnet 软件包目录结构如下所示：

```
wiznet
├───inc                             // RT_Thread 移植头文件
├───iolibrary                       // WIZnet 官方库文件
│   └───Ethernet                    // WIZnet 官方 Socket APIs 和 WIZCHIP 驱动
│   │	└───W5500                   // WIZCHIP 驱动
│   │   wizchip_conf.c              // Socket 配置文件
│   │   wizchip_socket.c            // Socket APIs 文件
│   └───Internet                    // WIZnet 官方网络功能实现
│   │	└───DHCP                    // DHCP 功能实现
│   └───────DNS                     // DNS 功能实现
├───src                             // RT_Thread 移植源码文件
│   └───wiz_af_inet.c               // WIZnet BSD Socket 注册到 SAL
│   │	wiz_device.c                // WIZnet 设备初始化
│   │   wiz_ping.c                  // WIZnet 设备 Ping 命令实现
│   │	wiz_socket.c                // WIZnet BSD Socket APIs 实现
│   └───wiz.c                       // WIZnet 初始化（设备初始化、网络初始化）
│   LICENSE                         // 软件包许可证
│   README.md                       // 软件包使用说明
└───SConscript                      // RT-Thread 默认的构建脚本
```


### 1.2 许可证

WIZnet 软件包遵循 Apache-2.0 许可，详见 LICENSE 文件。

### 1.3 依赖

- RT-Thread 4.0.1+
- SAL 组件
- netdev 组件
- SPI 驱动：WIZnet 设备使用  SPI 进行数据通讯，需要系统 SPI 驱动框架支持；
- PIN 驱动：用于处理设备复位和中断引脚；

## 2、获取软件包

使用 WIZnet 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```shell
WIZnet: WIZnet TCP/IP chips SAL framework implement
        WIZnet device type (W5500)  --->
        WIZnet device configure  --->
            (spi30) SPI device name
            (10) Reset PIN number
            (11) IRQ PIN number
  [ ]   Enable alloc IP address through DHCP
            WIZnet network configure  --->
                (192.168.1.10) IPv4: IP address
                (192.168.1.1) IPv4: Gateway address
                (255.255.255.0) IPv4: Mask address
  [ ]   Enable Ping utility
  [ ]   Enable debug log output
        Version (latest)  --->
```

**WIZnet device type** ：配置支持的设备类型（目前只支持 W5500 设备 ）

**WIZnet device configure** ：配置使用设备的参数

- **SPI device name**：配置使用 SPI 的设备名称（注意需设置为**非 SPI 总线设备**）

- **Reset PIN number**：配置设备连接的复位引脚号（根据实际使用引脚号修改）

- **IRQ PIN number**：配置设备连接的中断引脚号（同上）

**Enable alloc IP address through DHCP**： 配置是否使用 DHCP 分配 IP 地址（默认开启）

**WIZnet network configure**：如果不开启 DHCP 功能，需要配置静态连接的 IP 地址、网关和子网掩码

**Enable Ping utility**： 配置开启 Ping 命令 （默认开启）

**Enable debug log output**：配置开启调试日志显示

**Version**：软件包版本选择

## 3、使用软件包

WIZnet 软件包初始化函数如下所示：

```c
int wiz_init（void）；
```

该函数支持组件初始化，如果开启组件自动初始化功能，则应用层无需在调用该函数 ，函数主要完成功能有，

- 设置默认 MAC 地址；

- 设备配置和初始化（配置 SPI 设备，配置复位和中断引脚）；

- 网络配置和初始化（DHCP 分配 IP 地址，配置 socket 参数 ）；

- 注册实现的 BSD Socket APIs 到 SAL 套接字抽象层中，完成 WIZnet 设备适配；

每个 WIZnet 设备需要唯一的 MAC 地址，用户可以在应用层程序中调用如下函数设置 WIZnet 设备 MAC 地址，如果不调用该函数，设备将使用默认的 MAC 地址，默认 MAC 地址为 `00-E0-81-DC-53-1A`（注意：同一个局域网中如果存在相同 MAC 地址的设备，可能导致设备网络异常） 。

```c
int wiz_set_mac(const char *mac);
```

设备上电初始化完成，设置设备 MAC 地址成功，然后可以在 FinSH 中输入命令 `wiz_ifconfig` 查看设备 IP 地址、MAC 地址等网络信息，如下所示：

```shell
msh />ifconfig
network interface device: W5500 (Default)        ## 设备名称
MTU: 1472                                        ## 网络最大传输单元
MAC: 00 e0 81 dc 53 1a                           ## 设备 MAC 地址
FLAGS: UP LINK_UP INTERNET_UP                    ## 设备标志
ip address: 192.168.12.26                        ## 设备 IP 地址
gw address: 192.168.10.1                         ## 设备网关地址
net mask  : 255.255.0.0                          ## 设备子网掩码
dns server #0: 192.168.10.1                      ## 域名解析服务器地址0
dns server #1: 0.0.0.0                           ## 域名解析服务器地址1
```

获取 IP 地址成功之后，如果开启 Ping 命令功能，可以在 FinSH 中输入命令 `ping + 域名地址` 测试网络连接状态， 如下所示：

```shell
msh />wiz_ping baidu.com
32 bytes from 220.181.57.216 icmp_seq=0 ttl=128 time=31 ticks
32 bytes from 220.181.57.216 icmp_seq=1 ttl=128 time=31 ticks
32 bytes from 220.181.57.216 icmp_seq=2 ttl=128 time=32 ticks
32 bytes from 220.181.57.216 icmp_seq=3 ttl=128 time=32 ticks
```

`ping` 命令测试正常说明 WIZnet 设备网络连接成功，之后可以使用 SAL（套接字抽象层） 抽象出来的标准 BSD Socket APIs 进行网络开发（MQTT、HTTP、MbedTLS、NTP、Iperf 等），WIZnet 软件包支持的协议簇类型为：主协议簇为 **AF_WIZ**、次协议簇为 **AF_INET**（具体区别和使用方式可查看  [SAL 编程指南](https://www.rt-thread.org/document/site/submodules/rtthread-manual-doc/zh/1chapters/13-chapter_sal/) ）。

## 4、常见问题

- SPI 设备初始化时断言问题

  ```shell
  (wiz_device->parent.type == RT_Device_Class_SPIDevice) assertion failed at function:wiz_spi_init, line number:126
  ```

  出现上述断言问题，可能原因是 ENV 中配置 WIZnet 使用的 SPI 设备名称填写不正确，请区分 SPI DEVICE 与 SPI BUS 的关系。如果 BSP 工程中没有 SPI 设备或者只有 SPI 总线设备，需要手动在驱动中挂载 SPI 设备到 SPI 总线，并正确配置 WIZnet 软件包中使用的 SPI 设备名称。

- WIZnet 软件包最新版本已支持作为 server 服务器模式（V1.1.0 版本之前不支持）。

- WIZNet 软件包初始化出现 ```[E/wiz.dev] You should attach [wiznet] into SPI bus firstly.```错误，是因为没有挂载 winzet 设备到 SPI 总线导致的；请参考 wiz_init 函数中的注释，解决软件包初始化失败的问题。

- 在使用 RT-Thread 仓库的既往代码时，请比对 ```[components/net/sal_socket/src/sal_socket.c]```的内容，尤其是关于此处 [PR](https://github.com/RT-Thread/rt-thread/pull/3534/files) 的内容，注意 sal_closesocket 的内容。当你总是申请 socket(-1) 失败时，请确保你所使用的 RT-Thread 的代码是与该 [PR](https://github.com/RT-Thread/rt-thread/pull/3534/files) 的意图相符合的。

- 当出现申请 socket 时错误为  ```0x22``` 错误，注意 wiznet 的开发分支处于 master 版本或者大于 V1.1.0 的版本。请留意  ```wiz_socket_init()```  的执行顺序，因为 ```sal_check_netdev_internet_up``` 联网检测函数，会主动申请 socket 以判断 w5500 是否具有网络能力，而网络状态变更会导致  ```sal_check_netdev_internet_up```  被调用，造成 ```0x22``` 错误。


## 5、注意事项

- 获取软件包时，需要注意正确配置使用的 SPI 设备名称、复位引脚号和中断引脚号；
- 初始化完成之后，建议使用 `wiz_set_mac()` 函数设置设备 MAC 地址，防止使用默认 MAC 地址产生冲突；

## 6、联系方式 & 感谢

- 维护：RT-Thread 开发团队
- 主页：https://github.com/RT-Thread-packages/wiznet
