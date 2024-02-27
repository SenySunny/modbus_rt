#include "_aht20_Aht20.h"
#include "aht20_driver.h"
#include <FreeRtos.h>
#include <stdint.h>

void _aht20_Aht20___init__(PikaObj *self) { aht20_init(); }

pika_float _aht20_Aht20_readHumidity(PikaObj *self) {
  AHT20_Data data;
  aht20_read(&data);
  return data.humidity;
}

pika_float _aht20_Aht20_readTemp(PikaObj *self) {
  AHT20_Data data;
  aht20_read(&data);
  return data.temperature;
}

PikaEventListener *g_pika_aht20_listener = NULL;

#define AHT20_EVENT_ID_READ_TEMP 0x01
#define AHT20_EVENT_ID_READ_HUMI 0x02
#define AHT20_EVENT_ID_READ_ALL 0x03

#define AHT20_EVENT_SIGNAL_READ_COMPLETE 0x01

void task_aht20_read(void *arg) {
  AHT20_Data data;
  aht20_read(&data);
  /* 发送事件回调的 signal */

  /* 用 New_pikaLIstFrom 创建一个两个成员的 list，
  然后用 arg_newObj 将其转换为 * arg */
  pika_eventListener_send(
      g_pika_aht20_listener, AHT20_EVENT_ID_READ_ALL,
      arg_newObj(New_pikaListFrom(arg_newFloat(data.temperature),
                                  arg_newFloat(data.humidity))));
  vTaskDelete(NULL); // 删除这个线程
}

void task_aht20_read_data(void *arg) {
  uint32_t event_id = *(uint32_t *)arg;
  AHT20_Data data;
  aht20_read(&data);
  float result = 0;
  if (event_id == AHT20_EVENT_ID_READ_TEMP) {
    result = data.temperature;
  } else if (event_id == AHT20_EVENT_ID_READ_HUMI) {
    result = data.humidity;
  } else {
    result = data.temperature;
  }
  /* 发送事件回调，变量是一个arg */
  pika_eventListener_send(g_pika_aht20_listener, event_id,
                          arg_newFloat(result));
  vTaskDelete(NULL); // 删除这个线程
}

void _aht20_Aht20_startRead(PikaObj *self, Arg *callback) {
  /* 可以在注册时再初始化事件监听器 */
  if (NULL == g_pika_aht20_listener) {
    pika_eventListener_init(&g_pika_aht20_listener);
  }
  /* 你需要通过某种方式来生成这个事件的 ID，后面要使用相同的 ID 来触发这个事件
   */
  uint32_t eventId = AHT20_EVENT_ID_READ_ALL;
  /* 将这个事件注册进监听器 */
  pika_eventListener_registEventCallback(g_pika_aht20_listener, eventId,
                                         callback);
  /* 创建一个线程来读取数据 */
  xTaskCreate(task_aht20_read, "aht20_read", 8192, NULL, 1, NULL);
}

void _aht20_Aht20_startReadHumidity(PikaObj *self, Arg *callback) {
  /* 可以在注册时再初始化事件监听器 */
  if (NULL == g_pika_aht20_listener) {
    pika_eventListener_init(&g_pika_aht20_listener);
  }
  /* 你需要通过某种方式来生成这个事件的 ID，后面要使用相同的 ID 来触发这个事件
   */
  uint32_t eventId = AHT20_EVENT_ID_READ_HUMI;
  /* 将这个事件注册进监听器 */
  pika_eventListener_registEventCallback(g_pika_aht20_listener, eventId,
                                         callback);
  /* 创建一个线程来读取数据 */
  xTaskCreate(task_aht20_read_data, "aht20_read", 8192, &eventId, 1, NULL);
}

void _aht20_Aht20_startReadTemp(PikaObj *self, Arg *callback) {
  /* 可以在注册时再初始化事件监听器 */
  if (NULL == g_pika_aht20_listener) {
    pika_eventListener_init(&g_pika_aht20_listener);
  }
  /* 你需要通过某种方式来生成这个事件的 ID，后面要使用相同的 ID 来触发这个事件
   */
  uint32_t eventId = AHT20_EVENT_ID_READ_TEMP;
  /* 将这个事件注册进监听器 */
  pika_eventListener_registEventCallback(g_pika_aht20_listener, eventId,
                                         callback);
  /* 创建一个线程来读取数据 */
  xTaskCreate(task_aht20_read_data, "aht20_read", 8192, &eventId, 1, NULL);
}
