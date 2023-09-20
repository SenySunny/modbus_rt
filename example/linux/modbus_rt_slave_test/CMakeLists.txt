cmake_minimum_required(VERSION 3.10)

project(modbus_rt_linux)

# 添加modbus_rt相关的文件
set(MODBUS_FILE
    ../../../src/agile_modbus/agile_modbus.c
    ../../../src/agile_modbus/agile_modbus_rtu.c
    ../../../src/agile_modbus/agile_modbus_tcp.c 
    ../../../src/modbus_rt/modbus_ascii.c
    ../../../src/modbus_rt/modbus_data_trans.c
    ../../../src/modbus_rt/modbus_rtu.c
    ../../../src/modbus_rt/modbus_slave.c
    ../../../src/modbus_rt/modbus_tcp.c
    ../../../src/platform/linux_win/modbus_rt_platform_memory.c
    ../../../src/platform/linux_win/modbus_rt_platform_net_socket.c
    ../../../src/platform/linux_win/modbus_rt_platform_serial.c
    ../../../src/platform/linux_win/modbus_rt_platform_thread.c
    ../../../src/slave_util/modbus_slave_util.c
)

# 添加port/cmake/main.c文件
set(PORT_FILE main.c)

# 将所有的源文件组合在一起
set(SOURCE_FILES ${MODBUS_FILE} ${PORT_FILE})

# 添加头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/../../../src/agile_modbus)
include_directories(${CMAKE_SOURCE_DIR}/../../../src/modbus_rt)
include_directories(${CMAKE_SOURCE_DIR}/../../../src/platform/linux_win)
include_directories(${CMAKE_SOURCE_DIR}/../../../src/slave_util)

# 当添加PikaMath的时候需要添加连接math.h
LINK_LIBRARIES(m) #括号中添加标准库名称，math的库用m表示

# 设置输出目标
add_executable(modbus_rt_linux ${SOURCE_FILES})

# 添加pthread库链接
find_package(Threads REQUIRED)
target_link_libraries(modbus_rt_linux Threads::Threads)
target_link_libraries(modbus_rt_linux rt nsl resolv)
target_link_libraries(modbus_rt_linux serialport)
