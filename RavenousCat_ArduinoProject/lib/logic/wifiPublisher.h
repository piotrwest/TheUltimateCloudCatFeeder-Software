#pragma once

#include "inMemoryStore.h"
#include "WiFiClientSecure.h"
#include "PubSubClient.h"

class WifiPublisher
{

public:
	static void setupAndConnectBlockinglyToAWSIOT();
	static int protothread();
	static int protothreadKeepAliveAndReceive();
	static bool publishNow();

private:
	static void messageReceived(char *topic, byte *payload, unsigned int length);
	static void connectToMqtt(bool nonBlocking = false);

	static struct pt pt;
	static struct pt pt2;
    static long currentUnpublishedMilliseconds;
	static WiFiClientSecure net;
	static PubSubClient client;
	static char *MQTT_SUB_TOPIC;
};