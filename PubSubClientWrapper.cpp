//------------------------------------------------------------------
// Copyright(c) 2024 a2n Technology
// Anwar Minarso (anwar.minarso@gmail.com)
// https://github.com/anwarminarso/PubSubClientWrapper
// This file is part of the a2n PubSubClientWrapper v1.0.0
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
// Lesser General Public License for more details
//------------------------------------------------------------------

#include "PubSubClientWrapper.h"
using namespace a2n::iot;
#ifdef __has_include(<WiFi.h>)
#include <WiFi.h>
#define HAS_WIFI
String getMACAddress() {
	String macAddress = WiFi.macAddress();
	macAddress.remove(14, 1);
	macAddress.remove(11, 1);
	macAddress.remove(8, 1);
	macAddress.remove(5, 1);
	macAddress.remove(2, 1);
	macAddress.toUpperCase();
	return macAddress;
}
#else
String generateRandomClientId(int len) {
	//48-57  (0-9)
	//65-90  (A-Z)
	//97-122 (a-z)
	//10 => 1-10 
	//26 => 11-36
	//26 => 37-62
	String output = "";
	for (int i = 0; i < len; i++) {
		uint8_t random_index = random(1, 62);
		if (random_index <= 10) {
			output += (char)(random_index + 47);
		}
		else if (random_index <= 36) {
			output += (char)(random_index + 54);
		}
		else {
			output += (char)(random_index + 60);
		}
	}
	return output;
}
#endif

PubSubClientWrapper::PubSubClientWrapper() :
	_subscribehandlers(ListOf<SubscribeHandler*>([](SubscribeHandler* h) { delete h; })),
	_publishHandlers(ListOf<PublishHandler*>([](PublishHandler* h) { delete h; }))
{
}
PubSubClientWrapper::PubSubClientWrapper(Client& client) :
	_subscribehandlers(ListOf<SubscribeHandler*>([](SubscribeHandler* h) { delete h; })),
	_publishHandlers(ListOf<PublishHandler*>([](PublishHandler* h) { delete h; }))
{
	this->setClient(client);
}
void PubSubClientWrapper::setClient(Client& client) {
	this->_client = &client;
	this->_mqttClient.setClient(client);
}
void PubSubClientWrapper::setMqttClientId(const char* ClientId) {
	this->_mqttClientId = ClientId;
}

void PubSubClientWrapper::setMqttServer(const char* mqttServer) {
	this->_mqttServer = mqttServer;
	this->setMqttServer();
}
void PubSubClientWrapper::setMqttServer(const char* mqttServer, uint16_t mqttPort) {
	this->_mqttServer = mqttServer;
	this->_mqttPort = mqttPort;
	this->setMqttServer();
}
void PubSubClientWrapper::setMqttServer(const char* mqttServer, uint16_t mqttPort, const char* UserName, const char* Password) {
	this->_mqttServer = mqttServer;
	this->_mqttPort = mqttPort;
	this->_mqttUsername = UserName;
	this->_mqttPassword = Password;
	this->setMqttServer();
}
void PubSubClientWrapper::setMqttServer(const char* mqttServer, const char* UserName, const char* Password) {
	this->_mqttServer = mqttServer;
	this->_mqttUsername = UserName;
	this->_mqttPassword = Password;
	this->setMqttServer();
}
void PubSubClientWrapper::setMqttServer(const char* UserName, const char* Password) {
	this->_mqttUsername = UserName;
	this->_mqttPassword = Password;
	this->setMqttServer();
}
void PubSubClientWrapper::setMqttServer() {
	this->_mqttClient.setServer(_mqttServer, _mqttPort);
}

SubscribeHandler& PubSubClientWrapper::setSubscription(const char* topicFilter, ArSubscribeHandlerFunction func) {
	SubscribeHandler* handler = new SubscribeHandler();
	handler->setTopicFilter(topicFilter);
	handler->setFunction(func);
	this->addSubscribeHandler(handler);
	return *handler;
}
SubscribeHandler& PubSubClientWrapper::setSubscription(const char* topicFilter, ArSubscribeMessageHandlerFunction func) {
	SubscribeHandler* handler = new SubscribeHandler();
	handler->setTopicFilter(topicFilter);
	handler->setFunction(func);
	this->addSubscribeHandler(handler);
	return *handler;
}
PublishHandler& PubSubClientWrapper::setPublisher(const char* topic, int interval, ArPublishHandlerFunction func) {
	PublishHandler* handler = new PublishHandler();
	handler->setTopic(topic);
	handler->setInterval(interval);
	handler->setFunction(func);
	this->addPublishHandler(handler);
	return *handler;
}
PublishHandler& PubSubClientWrapper::setPublisher(const char* topic, int interval, int startDelay, ArPublishHandlerFunction func) {
	PublishHandler* handler = new PublishHandler();
	handler->setTopic(topic);
	handler->setInterval(interval);
	handler->setStartDelay(startDelay);
	handler->setFunction(func);
	this->addPublishHandler(handler);
	return *handler;
}
void PubSubClientWrapper::removePublisher(const char* topic) {
	this->_publishHandlers.remove_first([topic](PublishHandler* h) {
		return h->isTopicEqual(topic);
		});
}
void PubSubClientWrapper::removeSubscription(const char* topicFilter) {
	this->_subscribehandlers.remove_first([topicFilter](SubscribeHandler* h) {
		return h->canHandle(topicFilter);
		});
}
void PubSubClientWrapper::initMqtt() {
	if (this->_mqttClientId && !this->_mqttClientId[0]) {
		String clientId = "";
#if defined(ESP8266)
		clientId += "ESP8266-";
#elif defined(ESP32)
		clientId += "ESP32-";
#else
		clientId += "Arduino-";
#endif
#if defined(HAS_WIFI)
		clientId += getMACAddress();
		this->print("Generate ");
#else
		clientId += generateRandomClientId(8);
		this->print("Generate random ");
#endif
		int str_len = clientId.length() + 1;
		char* clientIdChars = (char*)malloc(str_len);
		clientId.toCharArray(clientIdChars, str_len);
		this->_mqttClientId = clientIdChars;
	}
	this->print("MQTT Client Id: ");
	this->println(this->_mqttClientId);
	this->_mqttClient.setCallback([&](char* topic, uint8_t* payload, unsigned int length) {
		for (const auto& h : this->_subscribehandlers) {
			if (h->canHandle(topic)) {
				h->handleFunction(topic, payload, length);
			}
		}
	});
}
bool PubSubClientWrapper::connectMqtt() {
	bool result = false;
	if (!_mqttClient.connected()) {
		if (millis() - _lastReconnect < 1000)
			return result;
		this->_reconnectMqttCount++;
		this->_lastReconnect = millis();
#if defined(ESP8266) || defined(ESP32)
		if (this->_reconnectMqttCount > this->_maxReconnect) {
			this->println("Restart ESP...");
			ESP.restart();
		}
#endif
		this->print("Attempting MQTT connection: ");
		// Attempt to connect
		if (_mqttClient.connect(_mqttClientId, _mqttUsername, _mqttPassword)) {
			this->println("connected");
			for (const auto& h : _subscribehandlers) {
				this->print("Subscribing to ");
				this->println(h->getTopicFilter());
				this->_mqttClient.subscribe(h->getTopicFilter());
			}
			this->_reconnectMqttCount = 0;
			result = true;
		}
		else {
			this->print("Failed, Reason Code=");
			this->print(_mqttClient.state());
			this->println();
		}
	}
	else
		result = true;
	return result;
}
bool PubSubClientWrapper::loop() {
	if (!connectMqtt())
		return false;
	this->_mqttClient.loop();
	for (const auto& h : this->_publishHandlers) {
		this->now = millis();
		if (h->canHandle(this->now)) {
			const char* message = h->handleFunction();
			if (message != nullptr) {
				this->_mqttClient.publish(h->getTopic(), message);
			}
		}
	}
	return true;
}