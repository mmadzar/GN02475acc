#include "CanBus.h"

StaticJsonDocument<512> docJ;
CAN_FRAME frames[3];
long displayValue = 0;
int consumptionCounter = 0; // 0 - 65535(0xFFFF)
MqttPubSub *mqttClientCan;

CanBus::CanBus()
{
  init();
}

void CanBus::init()
{
  CAN0.setCANPins(settings.can0_rx, settings.can0_tx);
}

void CanBus::setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport)
{
  mqttClientCan = &mqtt_client;
  b2w = &wifiport;

  CAN0.begin(500000);
  CAN0.watchFor(); // watch all - performance OK until connected clients!
  // // BMW IKE
  // CAN0.watchFor(0x613);
  // // BMW DSC
  // CAN0.watchFor(0x153); // speed  km/h
  // CAN0.watchFor(0x1F0); // wheel speed km/h

  // // CAN0.watchFor(0x545);
  // // CAN0.watchFor(0x316);
  // // CAN0.watchFor(0x329);

  // init frames for sending
  CAN_FRAME txFrame;
  txFrame.rtr = 0;
  txFrame.extended = false;
  txFrame.length = 8;

  // lamps off
  txFrame.id = 0x545;
  txFrame.data.uint8[0] = 0x00; // no engine, cruise or EML
  txFrame.data.uint8[1] = 0x00; // hfc.slice(0, 2)
  txFrame.data.uint8[2] = 0x00; // hfc.slice(2, 4)
  txFrame.data.uint8[3] = 0x00; // no overheat light
  txFrame.data.uint8[4] = 0x7E;
  txFrame.data.uint8[5] = 0x10;
  txFrame.data.uint8[6] = 0x00;
  txFrame.data.uint8[7] = 0x18;
  frames[0] = txFrame;

  // RPM - test 3k rpm * 6.4 = 4b00
  txFrame.id = 0x316;
  txFrame.data.uint8[0] = 0x05;
  txFrame.data.uint8[1] = 0x62;
  txFrame.data.uint8[2] = 0x00; // rpm 0,2
  txFrame.data.uint8[3] = 0x00; // rpm 2,4
  txFrame.data.uint8[4] = 0x65;
  txFrame.data.uint8[5] = 0x12;
  txFrame.data.uint8[6] = 0x00;
  txFrame.data.uint8[7] = 0x62;
  frames[1] = txFrame;

  // Temp - temp needle to beginning of red section [0xE6] - celsius*2
  txFrame.id = 0x329;
  txFrame.data.uint8[0] = 0xD9;
  txFrame.data.uint8[1] = 0xE6;
  txFrame.data.uint8[2] = 0xB2;
  txFrame.data.uint8[3] = 0x19;
  txFrame.data.uint8[4] = 0x00;
  txFrame.data.uint8[5] = 0xEE;
  txFrame.data.uint8[6] = 0x00;
  txFrame.data.uint8[7] = 0x00;
  frames[2] = txFrame;
}

void CanBus::handle()
{
  CAN_FRAME frame;
  if (CAN0.read(frame))
  {
    switch (frame.id)
    {
    case 0x613:
      handle613(frame);
      break;
    default:
      break;
    }

    // store message to buffer
    b2w->addBuffer(0xf1);
    b2w->addBuffer(0x00); // 0 = canbus frame sending
    uint32_t now = micros();
    b2w->addBuffer(now & 0xFF);
    b2w->addBuffer(now >> 8);
    b2w->addBuffer(now >> 16);
    b2w->addBuffer(now >> 24);
    b2w->addBuffer(frame.id & 0xFF);
    b2w->addBuffer(frame.id >> 8);
    b2w->addBuffer(frame.id >> 16);
    b2w->addBuffer(frame.id >> 24);
    b2w->addBuffer(frame.length + (uint8_t)(((int)1) << 4)); // 2 ibus address
    for (int c = 0; c < frame.length; c++)
      b2w->addBuffer(frame.data.uint8[c]);
    b2w->addBuffer(0x0a); // new line in serial monitor
  }
}

int CanBus::handle613(CAN_FRAME frame)
{
  const int v = (int)(frame.data.bytes[2]);
  if (status.ikeFuelLevel != v)
  {
    mqttClientCan->sendMessage(String(v), String(wifiSettings.hostname) + "/out/IKE/fuel_level");
    status.ikeFuelLevel = v;
  }
  return status.ikeFuelLevel;
}

void CanBus::sendMessageSet()
{
  // create and send can message in intervals
  if (status.currentMillis - previousMillis >= intervals.CANsend)
  {
    previousMillis = status.currentMillis;

    // update rpm and coolant temp values
    displayValue = status.rpm * 6.4;
    frames[1].data.uint8[2] = (int)((displayValue & 0X000000FF));      // rpm 0,2
    frames[1].data.uint8[3] = (int)((displayValue & 0x0000FF00) >> 8); // rpm 2,4
    displayValue = status.coolant_temp * 2;
    frames[2].data.uint8[1] = (int)((displayValue & 0X000000FF));

    CAN0.sendFrame(frames[0]);
    CAN0.sendFrame(frames[1]);
    CAN0.sendFrame(frames[2]);
  }
}
