from building import *

cwd  = GetCurrentDir()

src = Split('''
src/wiz_af_inet.c
src/wiz_device.c
src/wiz_socket.c
src/wiz.c
''')

if GetDepend(['WIZ_USING_PING']):
    src += Glob('src/wiz_ping.c')

src += Glob('ioLibrary/Ethernet/*.c')
src += Glob('ioLibrary/Internet/DNS/*.c')

if GetDepend(['WIZ_USING_DHCP']):
    src += Glob('ioLibrary/Internet/DHCP/*.c')

if GetDepend(['WIZ_USING_W5500']):
    src += Glob('ioLibrary/Ethernet/W5500/*.c')
    
CPPPATH = [
cwd + '/inc',
cwd + '/ioLibrary',
cwd + '/ioLibrary/Ethernet',
cwd + '/ioLibrary/Internet',
]

group = DefineGroup('WIZnet', src, depend = ['PKG_USING_WIZNET'], CPPPATH = CPPPATH)

Return('group')
