import PikaStdLib
import machine, os
import _ctest
import aht20
import unittest
import network, socket, json
import _thread, eventloop, fsm
import PikaDebug
import time
import modbus_rt
import modbus_rt_defines as cst

pdb = PikaDebug.Debuger()

print('hello PikaPython')
mem = PikaStdLib.MemChecker()
print('mem used max:')
mem.max()
print('mem used now:')
mem.now()

class AHT20UnitTest(unittest.TestCase):
    def test_read(self):
        aht = aht20.AHT20()
        temp, humi = aht.read()
        print('temp: ', temp, ' humi: ', humi)
        self.assertTrue(temp > -40 and temp < 80)
        self.assertTrue(humi > 0 and humi < 100)
    
    def test_readTemp(self):
        aht = aht20.AHT20()
        temp = aht.readTemp()
        print('temp: ', temp)
        self.assertTrue(temp > -40 and temp < 80)
    
    def test_readHumidity(self):
        aht = aht20.AHT20()
        humi = aht.readHumidity()
        print('humi: ', humi)
        self.assertTrue(humi > 0 and humi < 100)


def utest():
    suit = unittest.TestSuite("test1")
    suit.addTest(AHT20UnitTest())
    runner = unittest.TextTestRunner()
    res = runner.run(suit)

def wifi_init():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect('CU_ugJz', '16643685017')

    for i in range(10):
        if wlan.isconnected():
            print('wifi connected')
            return
        time.sleep(1)
        print('wifi connecting...', i)

def socket_connect():
    wifi_init()
    pdb.set_trace()
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('192.168.1.5', 8080))
    print('socket connected')
    client.send('hello'.encode())
    return client


def aht20_upload():
    client = socket_connect()
    aht = aht20.AHT20()
    temp, humi = aht.read()
    message_dict = {'temp': temp, 'humi': humi}
    message_json = json.dumps(message_dict)
    client.send(message_json.encode())

def aht20_wait_request():
    client = socket_connect()
    while True:
        recv_str = client.recv(1024).decode()
        pdb.set_trace()
        print('recv: ', recv_str)
        if recv_str == 'exit':
            break
        recv_dict = json.loads(recv_str)

        if 'fun' not in recv_dict:
            print('fun not in recv_dict')
            continue
        
        if recv_dict['fun'] == "request_aht20":
            aht = aht20.AHT20()
            temp, humi = aht.read()
            message_dict = {
                "fun": "response_aht20",
                "ret":  {
                        "temp": temp,
                        "humi": humi
                    }
                }
            message_json = json.dumps(message_dict)
            client.send(message_json.encode())

def request_aht20():
    aht = aht20.AHT20()
    temp, humi = aht.read()
    message_dict = {
            "temp": temp,
            "humi": humi
        }
    return message_dict

def request_temp():
    aht = aht20.AHT20()
    temp, humi = aht.read()
    message_dict = {
            "temp": temp
        }
    return message_dict

def request_humi():
    aht = aht20.AHT20()
    temp, humi = aht.read()
    message_dict = {
            "humi": humi
        }
    return message_dict

def aht20_wait_request_eval():
    client = socket_connect()
    while True:
        recv_str = client.recv(1024).decode()
        pdb.set_trace()
        print('recv: ', recv_str)
        if recv_str == 'exit':
            break
        recv_dict = json.loads(recv_str)

        if 'fun' not in recv_dict:
            print('fun not in recv_dict')
            continue

        ret = eval(recv_dict['fun'] + '()')
        message_dict = {
            'fun': recv_dict['fun'],
            'ret': ret
        }
        message_json = json.dumps(message_dict)
        client.send(message_json.encode())
