# modbus_rt的脚本封装层

### 一、PikaPython封装层

​        可以通过pikapython对modbus_rt进行二次封装，最后可以再PC系统或者嵌入式系统中使用python接口或者通过REPL交互来使用modbus_rt。你可以再移植好modbus_rt和pikaPython之后，直接再pikaPython的官方仓库中使用使用pikaPython的包管理工具直接加载和使用modbus_rt库(只需要在requestment.txt中添加一行modbus_rt即可)。也可以自主添加modbus_rt库到pikapython中。方法如下:

1. 移植好modbus_rt和pikapython。

2. 把api/pikapython目录下的_modbus_rt.pyi，modbus_rt.py，modbus_rt_defines.py复制到pikapython代码的更目录下（即包含main.py的目录）。

3. 把api/pikapython目录下的整个modbus_rt文件夹（包含目录下的_modbus_rt_data_trans.c， _modbus_rt_rtu.c， _modbus_rt_tcp.c）复制到pikapython根目录下的pikascript-lib目录下（即pikapython/pikascript-lib目录）。

4. 再main.py中用import导入modbus_rt和modbus_rt_defines文件，如下所示：

   ```python
   import modbus_rt
   import modbus_rt_defines as cst
   ```

5. 重新编译即可（如果编译器没有设置编译前预编译皮卡python，则需要手动运行皮卡python目录下的rust-msc-latest-win10.exe文件进行预编译）。

   ​       这样modbus_rt就被添加到pikapython中。

