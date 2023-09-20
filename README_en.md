# modbus_rt：纯C实现面向工业物联网的modbus协议库

该项目的演示视频，详见bilibi视频：

### 一、介绍
​		一款基于agile_modbus移植的可以运行在windows, linux,  macos， RTOS(计划移植rt-thread和freeRTOS，其他系统请自行移植)的modbus库，采用纯C编写。可以同时支持slave模式和master模式，支持多例模式。支持几乎市面上常用的所有modbus协议支持。包括modbus RTU，modbus ASCII，modbus TCP，modbus TCP over UDP，modbus RTU over TCP/UDP,modbus ASCII over TCP/UDP，以覆盖更多特殊的的modbus协议。并且提供基于pikapython的python接口实现，可以无缝兼容到pikapython环境中。（后续考虑提供基于micro python和Cpython的接口实现，本人对micro python和Cpython的底层封装不是很熟悉，看情况，不一定有时间，感兴趣的可以自己移植）。  另外由于代码完全用C编写，可以很方便的封装成dll供C#等其他编程语言调用。

其主要的特性和优势如下：

1. 可以运行在市面上绝大部分平台，包括：windows，Linux，嵌入式RTOS。
2. 同时支持modbus slave模式和modbus master模式。几乎市面上最常用的modbus协议，包括：modbus RTU，modbus ASCII，modbus TCP，modbus TCP over UDP。以及modbus RTU over TCP/UDP, modbus ASCII over TCP/UDP。理论上可以快速移植modbus RTU 和modbus ASCII在其他通信接口的实现。
3. 基于面向对象的思维方式，采用句柄模式，非常方便实现modbus的多例模式。理论上只要性能，内存和空间足够，可以创建无数个modbus示例。
4. 在实现modbus slave的协议时，增加了set_pre_ans_callback和set_done_callback两个回调函数调用接口，方便快速实现modbus协议与其他协议的转换。比如modbus RTU，ASCII，TCP之间的相互转化， modbus转mqtt，modbus转sql， modbus转OPC Client， modbus转profibus，modbus转canOpen等协议（第三方协议需要自己实现）。可以快速实现类似DTU的应用，以适应各种工业物联网的应用需求。
5. 支持modbus协议的寄存器到各种数据的转换接口函数，支持各种大小端模式(市面上4种大小端模式均支持)。快速实现不同的大小端设备之间的数据通信。
6. modbus_rt库除了本身支持modbus常用协议之外，扩展支持P2P模式，可以通过MODBUS_P2P_ENABLE宏定义开启，该功能可以用于文件传输功能（需要有文件系统支持，或者自己移植修改modbus_p2p相关代码），可以自定义文件传输的modbus命令。
7. 在modbus_rt的modbus TCP over UDP模式下提供了网络设备查找和发现功能。即：modbus_rt设备端与PC客户端在同一个路由器或者交换机下面，即使没有设置设备端与客户端的IP地址在同一个网段下，可以通过往255.255.255.255的广播地址广播modbus命令，modbus_rt设备端收到广播信息后会对广播设备的ip地址进行检测，如果与设备不在同一个网段，会默认往255.255.255.255广播应答信息。这个时候，客户端就可以获取到modbus_rt设备端的ip地址。这样就可以实现设备发现。另外基于此基础上可以扩展实现修改设备的IP地址的功能（该功能的上层接口需要自己实现，例如定义特定的寄存器为修改设备IP地址功能，通过UDP写入指定的IP地址到该寄存器，设备端收到之后修改自己的IP为指定的静态IP地址功能）。
8. 提供基于pikapython的接口API，可以快速的融入到支持pikapython的应用当中去（目前windows，linux，嵌入式环境均采用pika的脚本功能来验证程序），更多pikapython相关功能，参考皮卡python的git：[https://gitee.com/Lyon1998/pikapython](https://gitee.com/Lyon1998/pikapython)

这里对modbus_rt的实现做几点说明：

1. modbus_rt库在windows, linux,  macos操作系统上基于pthread多线程（windows平台默认不支持phread，需要自己安装pthread库），并且把线程操作接口进行重定义，在RTOS上，可以自行采用系统的线程进行移植，可以参考rt-thread的移植。
2. 串口通信部分：在windows, linux,  macos操作系统上采用开源的libserialport作为串口通信底层(由于时间)，操作接口也进行重定义，在RTOS上自行移植，可以参考rt-thread的移植。
3. 网络通信部分：基于BSD Socket通信，并且需要支持IO多路复用接口（select机制），windows, linux,  macos操作系统都是支持的，在rt-thread上本身可以通过使能posix文件系统，开启BSD Socket支持即可，其他系统需要BSD Socket的支持。如果无需用到TCP/UDP网络部分功能，可以通过宏定义屏蔽网络的支持。

### 二、测试与使用

​	测试视频详见bilibili视频：

##### 1. windows平台

​		windows平台我们这里我们采用QT+MSVC来编译进行测试（理论上用Visual Studio—本身就是基于MSVC，或者MiniGW—基于GCC都可以），我们编译好的可以运行程序在example/windows目录下，源码工程可以见另外一个仓库：



​		我们只需要终端进入到example/windows目录下，输入```.\pikapython main.py```便可以运行main.py的代码，代码运行完毕之后，可以进入到REPL交互界面，在这个界面下我们可以通过脚本对modbus_rt进行测试。



​		如果需要编译到自己的应用程序种，只需要把src目录的代码复制到工程当中去，安装pthread库，自己添加好头文件即可，可以参考QT+MSVC的源码工程。

##### 2. Linux平台

​		Linux平台硬件理论上只要时标准的linux系统都可以使用。这里我采用了野火的鲁班猫1作为测试平台，手头刚好又这个开发板，而且目前市面上很多ROS机器人系统都采用了这块开发板，具有一定的代表性，modbus_rt时上层的应用开发API，所以实际上与硬件无关。



​		这里我们刷了野火提供的最新的Ubuntu桌面版本的固件。确保安装了git, cmake和gcc程序，用git下载代码。

终端进入example/linux目录，输入```sh make.sh```进行编译，此时会在example/linux/build目录下生成pikapython的可执行程序（运行需要用到的库已经复制到该目录下了），用cd进入build目录，便可以运行程序了



##### 3. RT-Thread平台

​		RT-thread平台采用了我之前自己DIY的一块PLC开发板，外壳完全兼容西门子的S7-200的PLC外壳，主控我采用国产的珠海半导体的APM32E103VET6芯片（基本兼容STM32F103VET6，引脚Pin2Pin兼容，但是RAM时STM32的2倍，STM32只有64K，比较小）。外挂一篇SPI Flash，运行little FS文件系统，网络部分采用W5500网络芯片。运行rt-thread系统。详细的测试代码可以见相应的工程：



​		该硬件有两个RS485接口，我们代码默认靠近网卡的RS485作为终端测试使用，我们连上网线，RS485线缆(一个用于modbus通信，一个用于终端调试)，上电，编译下载到硬件平台之后。我们可以通过modbus poll控制和读取IO的状态，也可以用终端运行modbus master示例，与其他设备通信。

##### 4. FreeRTOS平台

​	FreeRTOS平台，我们用了PikaPython的开源硬件 PikaPython-OpenHardware，基于ESP32-S3平台，本身又配置RS485接口，另外网络部分可以用wifi进行测试。



PikaPython-OpenHardware的开源地址为：[https://gitee.com/SenyLee/pikapython_openhardware](https://gitee.com/SenyLee/pikapython_openhardware)

平台的测试代码仓库（基于idf V5.1开发环境）：

##### 5. mosbus_rt与西门子PLC进行通信

​		因为我们以上4个平台的接口部分都是采用PikaPython封装的API，采用脚本进行通信，所以用那个平台无所谓，西门子PLC我们采用比较常用的S7-1214C的PLC平台，这里仅仅测试modbus TCP，我们用S7-1214C运行一个modbus TCP slave和一个modbus TCP master。modbus_rt设备端也运行一个modbus TCP slave和一个modbus TCP master。他们之间相互通信。

​		PLC工程详见example/PLC目录，工程采用博图V15.1创建工程

##### 6. modbus_rt与威纶通组态屏进行通信

​		威纶通组态屏程序，我们用威纶通的MT8071IP组态屏，结合我们的RT-Thread平台硬件，进行DO控制和DI检测示例。威纶通屏幕的工程详解example/WEINVIEW，采用EBproV6.08打开下载。理论上其他的组态屏操作也基本一致，这里就不一一测试了（主要手头没有相应的组态屏）。



##### 7. modbus_rt与大彩串口屏进行通信

​		考虑到很多工业应用并不会使用组态屏（主要价格贵）。所以这里我们也采用国产的大彩串口屏做了一个demo示例，国产的串口屏性价比就高很多了。大彩串口屏，很多型号都支持modbus协议，价格甚至只需要几十元就可以搞定，这里我们选择了一款2.8寸的串口屏，型号：DC24320M028_1110_0T。通过电脑的modbus_rt来控制串口屏上的显示功能。

​		大彩串口屏的工程见example/DACAI，软件采用VisualTFT_3.0

##### 8. mobus DTU相关示例

​	这里做一个简单的示例，实现modbus TCP转modbus RTU。这里只提供了运行在windows上的mian.py的脚本代码。理论上这个代码也可以运行在嵌入式平台上，实现网络转串口的DTU功能。利用此代码可以很方便的实现dtu相关的功能。

​		

​         这个演示种，我们两个设备，一个为windowsPC（运行modbus_master的代码），一个用Linux系统的鲁班猫作为DTU（运行modbus_dtu的代码），鲁班猫通过一个USB转RS485连接到我们的RT-Thread平台开发板上，鲁班猫设置strict为0，不对地址做检测，理论上这里我们可以用RS485总线或者多个串口连接多个串口或者RS485设备，我们只需要把设备的slave的地址设置不一样即可。这时候，我们便可以通过PC通过modbus TCP远程控制多个设备了。



### 三、接口说明

##### 1. 移植和使用

1. 通过modbus_tcp(mode), modbus_rtu(mode)或者modbus_ascii(mode)创建示例，mode：0表示modbus slave; 1表示modbus Master。该函数主要用于创建设备的数据块内容，设备块内容，初始化设备需要用到的互斥量和信号量。

2. 通过modbus_xxx_set_net，modbus_xxx_set_serial等函数初始化相应的参数,详情见API手册
3. 通过modbus_xxx_open开启运行设备，该函数主要用于 建立设备的通信端口，创建通信相关的线程。
4. 如果暂时不需要使用设备，可以使用modbus_xxx_close函数关闭设备，该函数主要用于结束设备通信相关线程，关闭设备的通信接口
5. 如果彻底不适用对应设备了，可以使用modbus_xxx_destroy销魂该设备，该函数主要用于销毁设备的互斥量和信号量相关数，销毁设备的数据信息和设备信。此时设备指针将指向空指针。
6. slave添加寄存器数据块函数为modbus_xxx_add_block函数。参数分别为寄存器类型(默认0, 1, 3, 4四种)，寄存器其实地址，存储数据的空间地址，寄存器的长度（不是数据长度）。
7. 获取或者修改寄存器的值用modbus_xxx_excuse函数，针对slave模式为读取或者写入自己的寄存器的值，针对master模式为读取或者写入连接slave设备的寄存器的值。
8. 如果要实现dtu或者modbus与其他协议的转换内容，可以通过modbus_xxx_set_pre_ans_callback和modbus_xxx_set_done_callback设置两个回调函数，分别再slave设备收到master端的数据时调用，和modbus通信结束后调用，可以再回调函数种完成协议转换相关的内容（注意回调函数不能为阻塞函数，否则会导致modbus通信超时）。
9. modbus_xxx_set_strict用在slave模式下，把strict设置为0则通信不会对master发送的数据的设备地址做匹配检测，即任何地址都可以与该slave设备通信，主要目的在于实现dtu和协议转换相关的功能。
10. modbus_data_xxx相关函数用于对寄存器与各种数据进行相互转化，以满足不同场景下通信的需求，设备默认为小端模式，由于不同设备的大小端模式不相同。可以采用不同的大小端模式对数据进行转换。

##### 2. 四种大小端模式示例

1. 小端模式（Little-endian byte swap）：

   > 寄存器0：0x0001，寄存器1：0x0002
   >
   > 表示的数字：0x20001（131073）

2.  大端模式（Big-endian byte swap）：

   > 寄存器0：0x0001，寄存器1：0x0002
   >
   > 表示的数字：0x10002000（16,777,728‬）

3. 内部大段，外部小端（Little-endian）：

   > 寄存器0：0x0001，寄存器1：0x0002
   >
   > 表示的数字：0x2000100（33,554,688‬）

4. 内部小端，外部大端（Big-endian）：

   > 寄存器0：0x0001，寄存器1：0x0002
   >
   > 表示的数字：0x10002（65538）

### 四、支持

​		欢迎右上角点击start给我一个支持，欢迎fork。



### 五、 联系方式

- 维护：SenySunny
- 主页：
- 邮箱：[senysunny@163.com](mailto:senysunny@163.com)

### 六、 感谢以下项目作者

​	本项目参考或者使用了如下开源项目的内容，再此对以下项目的创作者表示感谢，由于本项目使用的libserialport项目采用LGPL V3开源协议，所以该项目默认采用LGPL V3开源协议，后续有时间会把串口部分的接口自己独立实现，再考虑更换开源协议。

1.  [agile_modbus(一款开源的支持跨平台的轻量型 modbus 协议栈): https://github.com/loogg/agile_modbus](https://github.com/loogg/agile_modbus)
2.  [libserialport(一款跨平台的串口库): https://github.com/sigrokproject/libserialport](https://github.com/sigrokproject/libserialport)
3.  [PikaPython(一个完全重写的超轻量级 python 引擎): https://github.com/pikasTech/PikaPython](https://github.com/pikasTech/PikaPython)
