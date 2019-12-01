
#include "pt.h"

#include "wifiPublisher.h"
#include "hardwareConsts.h"
#include "deadSimpleTimer.h"
#include "log4arduino.h"
#include "inMemoryStore.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"
#include "PubSubClient.h"
#include "wifiPublisherAwsSecrets.h"

pt WifiPublisher::pt;
pt WifiPublisher::pt2;
long WifiPublisher::currentUnpublishedMilliseconds;
WiFiClientSecure WifiPublisher::net;
PubSubClient WifiPublisher::client(net);
char *WifiPublisher::MQTT_SUB_TOPIC;

const char MQTT_TOPIC_PREFIX[] = "$aws/things/";
const char MQTT_TOPIC_SUFFIX[] = "/shadow/update";
const char MQTT_PUB_TOPIC_NAME[] = "CatFeederMetrics/encodedRaw";
const int MQTT_PORT = 8883;

void WifiPublisher::messageReceived(char *topic, byte *payload, unsigned int length)
{
  LOG("Received on topic: [%s]:", topic);
  LOG("=>%.*s", length, (char*)payload);
}

void pubSubErr(int8_t MQTTErr)
{
  if (MQTTErr == MQTT_CONNECTION_TIMEOUT)
    LOG("Connection timeout");
  else if (MQTTErr == MQTT_CONNECTION_LOST)
    LOG("Connection lost");
  else if (MQTTErr == MQTT_CONNECT_FAILED)
    LOG("Connect failed");
  else if (MQTTErr == MQTT_DISCONNECTED)
    LOG("Disconnected");
  else if (MQTTErr == MQTT_CONNECTED)
    LOG("Connected");
  else if (MQTTErr == MQTT_CONNECT_BAD_PROTOCOL)
    LOG("Connect bad protocol");
  else if (MQTTErr == MQTT_CONNECT_BAD_CLIENT_ID)
    LOG("Connect bad Client-ID");
  else if (MQTTErr == MQTT_CONNECT_UNAVAILABLE)
    LOG("Connect unavailable");
  else if (MQTTErr == MQTT_CONNECT_BAD_CREDENTIALS)
    LOG("Connect bad credentials");
  else if (MQTTErr == MQTT_CONNECT_UNAUTHORIZED)
    LOG("Connect unauthorized");
}

void WifiPublisher::connectToMqtt(bool nonBlocking)
{
  LOG("MQTT connecting...");
  while (!client.connected())
  {
    if (client.connect(InMemoryStore::sdConfig.awsClientId))
    {
      LOG("MQTT connected!");
      if (!client.subscribe(MQTT_SUB_TOPIC))
        pubSubErr(client.state());
    }
    else
    {
      LOG("MQTT connecting... failed, reason -> ");
      pubSubErr(client.state());
      if (!nonBlocking)
      {
        LOG(" < MQTT will try again in 5 seconds");
        delay(5000);
      }
      else
      {
        LOG(" < MQTT connect attempt was non-blocking. Try again later.");
      }
    }
    if (nonBlocking)
      break;
  }
}

void WifiPublisher::setupAndConnectBlockinglyToAWSIOT()
{
    LOG("Entering WifiPublisher setup.");
    PT_INIT(&pt);

    net.setCACert(cacert);
    net.setCertificate(client_cert);
    net.setPrivateKey(privkey);
    client.setServer(InMemoryStore::sdConfig.awsHostAddress, MQTT_PORT);
    client.setCallback(WifiPublisher::messageReceived);

    MQTT_SUB_TOPIC = new char[strlen(MQTT_TOPIC_PREFIX) + strlen(InMemoryStore::sdConfig.awsClientId) + strlen(MQTT_TOPIC_SUFFIX) + 1];
    strcpy(MQTT_SUB_TOPIC, MQTT_TOPIC_PREFIX);
    strcat(MQTT_SUB_TOPIC, InMemoryStore::sdConfig.awsClientId);
    strcat(MQTT_SUB_TOPIC, MQTT_TOPIC_SUFFIX);

    LOG("WifiPublisher variable setup done. Connecting to AWS IOT...");
    connectToMqtt(false);

    LOG("Finished WifiPublisher setup");
}

bool WifiPublisher::publishNow()
{
    LOG("PUBLISHING metrics!! :)");
    InMemoryStore::putMessageBufferUtilization();
    InMemoryStore::putAWSIoTClientState(client.state());
    if (!client.publish(MQTT_PUB_TOPIC_NAME, InMemoryStore::getPublishPayload(), false)) {
        LOG("ERROR! Error occurred while publishing.");
        InMemoryStore::putMqttPublishError();
        pubSubErr(client.state());
        return false;
    } else {
        LOG("Published metrics! Cleaning InMemoryStore buffer.");
        InMemoryStore::initMessagePayload();
        InMemoryStore::putMqttPublishedSuccessfully();
        currentUnpublishedMilliseconds = 0;
    }
    return true;
}

int WifiPublisher::protothread()
{
    static DeadSimpleTimer::deadTimer helperTimer;

    PT_BEGIN(&pt);
    while (1)
    {
        DeadSimpleTimer::setMs(&helperTimer, 5000);
        PT_WAIT_UNTIL(&pt, DeadSimpleTimer::expired(&helperTimer));
        currentUnpublishedMilliseconds += 5000;

        if (!client.connected()) {
            connectToMqtt(true);
        } else {
            //if 23min of unpublished data or there is significant amount of data in memory
            if (currentUnpublishedMilliseconds > 23*60*1000 || InMemoryStore::isTimeToPublish())
            {
                if (publishNow())
                {
                    //if published successful - reset counter
                    currentUnpublishedMilliseconds = 0;
                } else {
                    //apply backpressure - wait 2min if possible - but it won't suppress it when the buffer has data
                    currentUnpublishedMilliseconds = 21*60*1000;
                }
            }
        }
    }

    PT_END(&pt);
}

int WifiPublisher::protothreadKeepAliveAndReceive()
{
    static DeadSimpleTimer::deadTimer helperTimer;

    PT_BEGIN(&pt2);
    while (1)
    {
        DeadSimpleTimer::setMs(&helperTimer, 100);
        PT_WAIT_UNTIL(&pt2, DeadSimpleTimer::expired(&helperTimer));

        client.loop();
    }

    PT_END(&pt2);
}