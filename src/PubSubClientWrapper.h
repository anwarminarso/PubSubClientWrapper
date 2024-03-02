#pragma once
#ifndef PubSubClientWrapper_H
#define PubSubClientWrapper_H

#include "Arduino.h"
#include "IPAddress.h"
#include "Client.h"
#include "Stream.h"
#include "PubSubHandler.h"
#include "CommonBase.h"
#include "List.hpp"
#include "CustomNetworkAdapter.h"


#ifdef WRAPPER_USE_WIFI
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#include "ESPWiFiNetworkAdapter.h"
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include "ESPWiFiNetworkAdapter.h"
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#else
#include <SPI.h>
#include <WiFi.h>
#endif
#endif

#ifdef WRAPPER_USE_MODEM
#include "ModemNetworkAdapter.h"
#endif



#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

// MQTT_VERSION : Pick the version
//#define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

// MQTT_MAX_PACKET_SIZE : Maximum packet size. Override with setBufferSize().
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 256
#endif

// MQTT_KEEPALIVE : keepAlive interval in Seconds. Override with setKeepAlive()
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds. Override with setSocketTimeout()
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE 80


// Possible values for client.state()
#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

// Maximum size of fixed header and variable length size header
#define MQTT_MAX_HEADER_SIZE 5

#define CHECK_STRING_LENGTH(l,s) if (l+2+strnlen(s, this->bufferSize) > this->bufferSize) {_client->stop();return false;}



namespace a2n::iot {
	class PubSubClientWrapper : public BaseWrapper {
	private:
#pragma region Fields

#if defined(WRAPPER_USE_SSL)
		ESP_SSLClient _sslClient;
		Client* _proxyClient = NULL;
#endif
		List<SubscribeHandler*> _subscribehandlers;
		List<PublishHandler*> _publishHandlers;
		List<BaseNetworkAdapter*> _networkAdapters;
		//BaseNetworkAdapter _networkAdapters[TOTAL_NETWORK_ADAPTER];
		Client* _client = NULL;
		BaseNetworkAdapter*  _currentNetworkAdapter = NULL;
		BaseNetworkAdapter* _mainNetworkAdapter = NULL;
		unsigned long _lastConnectedMainNetwork = 0;
		unsigned long _lastReconnectMainNetwork = 0;
		unsigned long _reconnectMainNetworkDelay = 180000;
		bool _isReconnectMainNetwork = false;
		
		uint8_t* buffer;
		Print* _logger;
		uint16_t bufferSize;
		uint16_t keepAlive;
		uint16_t socketTimeout;
		uint16_t nextMsgId;
		unsigned long lastOutActivity;
		unsigned long lastInActivity;
		bool pingOutstanding;
		bool hasSubscription = false;
		bool enableLogger = false;

		IPAddress ip;
		const char* domain;
		uint16_t port;
		Stream* stream;
		int _state;

		const char* _mqttUsername = NULL;
		const char* _mqttPassword = NULL;
		const char* _mqttClientId = NULL;
		const char* _mqttwillTopic = NULL;
		uint8_t _mqttwillQos = 0;
		boolean _mqttwillRetain = false;
		const char* _mqttwillMessage = NULL;
		boolean _mqttcleanSession = true;


		int _maxReconnect = 30;
		int _reconnectDelay = 1000;
		int _connectionAttempt = 0;
		unsigned long now = 0;
		unsigned long _lastReconnect = 0;
#pragma endregion
		uint32_t readPacket(uint8_t* lengthLength) {
			uint16_t len = 0;
			if (!readByte(this->buffer, &len)) return 0;
			bool isPublish = (this->buffer[0] & 0xF0) == MQTTPUBLISH;
			uint32_t multiplier = 1;
			uint32_t length = 0;
			uint8_t digit = 0;
			uint16_t skip = 0;
			uint32_t start = 0;

			do {
				if (len == 5) {
					// Invalid remaining length encoding - kill the connection
					_state = MQTT_DISCONNECTED;
					_client->stop();
					return 0;
				}
				if (!readByte(&digit)) return 0;
				this->buffer[len++] = digit;
				length += (digit & 127) * multiplier;
				multiplier <<= 7; //multiplier *= 128
			} while ((digit & 128) != 0);
			*lengthLength = len - 1;

			if (isPublish) {
				// Read in topic length to calculate bytes to skip over for Stream writing
				if (!readByte(this->buffer, &len)) return 0;
				if (!readByte(this->buffer, &len)) return 0;
				skip = (this->buffer[*lengthLength + 1] << 8) + this->buffer[*lengthLength + 2];
				start = 2;
				if (this->buffer[0] & MQTTQOS1) {
					// skip message id
					skip += 2;
				}
			}
			uint32_t idx = len;

			for (uint32_t i = start; i < length; i++) {
				if (!readByte(&digit)) return 0;
				if (this->stream) {
					if (isPublish && idx - *lengthLength - 2 > skip) {
						this->stream->write(digit);
					}
				}

				if (len < this->bufferSize) {
					this->buffer[len] = digit;
					len++;
				}
				idx++;
			}

			if (!this->stream && idx > this->bufferSize) {
				len = 0; // This will cause the packet to be ignored.
			}
			return len;
		}
		// reads a byte into result
		boolean readByte(uint8_t* result) {
			uint32_t previousMillis = millis();
			while (!_client->available()) {
				yield();
				uint32_t currentMillis = millis();
				if (currentMillis - previousMillis >= ((int32_t)this->socketTimeout * 1000)) {
					return false;
				}
			}
			*result = _client->read();
			return true;
		}

		// reads a byte into result[*index] and increments index
		boolean readByte(uint8_t* result, uint16_t* index) {
			uint16_t current_index = *index;
			uint8_t* write_address = &(result[current_index]);
			if (readByte(write_address)) {
				*index = current_index + 1;
				return true;
			}
			return false;
		}

		boolean write(uint8_t header, uint8_t* buf, uint16_t length) {
			uint16_t rc;
			uint8_t hlen = buildHeader(header, buf, length);

	#ifdef MQTT_MAX_TRANSFER_SIZE
			uint8_t* writeBuf = buf + (MQTT_MAX_HEADER_SIZE - hlen);
			uint16_t bytesRemaining = length + hlen;  //Match the length type
			uint8_t bytesToWrite;
			boolean result = true;
			while ((bytesRemaining > 0) && result) {
				bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE) ? MQTT_MAX_TRANSFER_SIZE : bytesRemaining;
				rc = _client->write(writeBuf, bytesToWrite);
				result = (rc == bytesToWrite);
				bytesRemaining -= rc;
				writeBuf += rc;
			}
			return result;
	#else
			rc = _client->write(buf + (MQTT_MAX_HEADER_SIZE - hlen), length + hlen);
			
			lastOutActivity = millis();
			return (rc == hlen + length);
	#endif
		}

		uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos) {
			const char* idp = string;
			uint16_t i = 0;
			pos += 2;
			while (*idp) {
				buf[pos++] = *idp++;
				i++;
			}
			buf[pos - i - 2] = (i >> 8);
			buf[pos - i - 1] = (i & 0xFF);
			return pos;
		}

		// Build up the header ready to send
		// Returns the size of the header
		// Note: the header is built at the end of the first MQTT_MAX_HEADER_SIZE bytes, so will start
		//       (MQTT_MAX_HEADER_SIZE - <returned size>) bytes into the buffer
		size_t buildHeader(uint8_t header, uint8_t* buf, uint16_t length) {
			uint8_t lenBuf[4];
			uint8_t llen = 0;
			uint8_t digit;
			uint8_t pos = 0;
			uint16_t len = length;
			do {

				digit = len & 127; //digit = len %128
				len >>= 7; //len = len / 128
				if (len > 0) {
					digit |= 0x80;
				}
				lenBuf[pos++] = digit;
				llen++;
			} while (len > 0);

			buf[4 - llen] = header;
			for (int i = 0; i < llen; i++) {
				buf[MQTT_MAX_HEADER_SIZE - llen + i] = lenBuf[i];
			}
			return llen + 1; // Full header size is variable length bit plus the 1-byte fixed header
		}

		String getRandomClientId(int len) {
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
			output.toUpperCase();
			return output;
		}

		void subscriptionCallback(char* topic, uint8_t* payload, unsigned int val) {
			int idx = -1;
			for (int i = 0; i < _subscribehandlers.getSize(); i++)
			{
				SubscribeHandler* handler = _subscribehandlers.get(i);
				if (handler->canHandle(topic)) {
					handler->handleFunction(topic, payload, val);
					break;
				}
			}
		}

		SubscribeHandler& addSubscribeHandler(SubscribeHandler* handler) {
			_subscribehandlers.add(handler);
			return *handler;
		};
		bool removeSubscribeHandler(SubscribeHandler* handler) {
			int idx = -1;
			for (int i = 0; i < _subscribehandlers.getSize(); i++)
			{
				if (_subscribehandlers.get(idx) == handler) {
					idx = i;
					break;
				}
			}
			if (idx >= 0) {
				_subscribehandlers.remove(idx);
				delete handler;
				return true;
			}
			else
				return false;
		};
		PublishHandler& addPublishHandler(PublishHandler* handler) {
			_publishHandlers.add(handler);
			return *handler;
		};
		bool removePublishHandler(PublishHandler* handler) {
			int idx = -1;
			for (int i = 0; i < _publishHandlers.getSize(); i++)
			{
				if (_publishHandlers.get(idx) == handler) {
					idx = i;
					break;
				}
			}
			if (idx >= 0) {
				_publishHandlers.remove(idx);
				delete handler;
				return true;
			}
			else
				return false;
		};

		boolean subscribe(const char* topic) {
			return subscribe(topic, 0);
		}

		boolean subscribe(const char* topic, uint8_t qos) {
			size_t topicLength = strnlen(topic, this->bufferSize);
			if (topic == 0) {
				return false;
			}
			if (qos > 1) {
				return false;
			}
			if (this->bufferSize < 9 + topicLength) {
				// Too long
				return false;
			}
			if (connected()) {
				// Leave room in the buffer for header and variable length field
				uint16_t length = MQTT_MAX_HEADER_SIZE;
				nextMsgId++;
				if (nextMsgId == 0) {
					nextMsgId = 1;
				}
				this->buffer[length++] = (nextMsgId >> 8);
				this->buffer[length++] = (nextMsgId & 0xFF);
				length = writeString((char*)topic, this->buffer, length);
				this->buffer[length++] = qos;
				return write(MQTTSUBSCRIBE | MQTTQOS1, this->buffer, length - MQTT_MAX_HEADER_SIZE);
			}
			return false;
		}

		boolean unsubscribe(const char* topic) {
			size_t topicLength = strnlen(topic, this->bufferSize);
			if (topic == 0) {
				return false;
			}
			if (this->bufferSize < 9 + topicLength) {
				// Too long
				return false;
			}
			if (connected()) {
				uint16_t length = MQTT_MAX_HEADER_SIZE;
				nextMsgId++;
				if (nextMsgId == 0) {
					nextMsgId = 1;
				}
				this->buffer[length++] = (nextMsgId >> 8);
				this->buffer[length++] = (nextMsgId & 0xFF);
				length = writeString(topic, this->buffer, length);
				return write(MQTTUNSUBSCRIBE | MQTTQOS1, this->buffer, length - MQTT_MAX_HEADER_SIZE);
			}
			return false;
		}

	public:
		PubSubClientWrapper() {
#if defined(WRAPPER_USE_SSL)
			this->_client = &_sslClient;
			this->_proxyClient = NULL;
			_sslClient.setDebugLevel(1);
#else
			this->_client = NULL;
#endif
			this->_state = MQTT_DISCONNECTED;
			this->stream = NULL;
			this->bufferSize = 0;
			setBufferSize(MQTT_MAX_PACKET_SIZE);
			setKeepAlive(MQTT_KEEPALIVE);
			setSocketTimeout(MQTT_SOCKET_TIMEOUT);
		}
		~PubSubClientWrapper() {
			free(this->buffer);
		}
		void setLogger(Print& logger) {
			enableLogger = true;
			_logger = &logger;
		}

		void addNetworkAdapter(BaseNetworkAdapter * adapter, bool mainNetwork = false) {
			_networkAdapters.add(adapter);
			if (mainNetwork)
				_mainNetworkAdapter = adapter;
		}

		BaseNetworkAdapter* getCurrentNetworkAdapter() {
			return _currentNetworkAdapter;
		}

		void setReconnectMainNetworkDelay(unsigned long reconnectMainNetworkDelay) {
			this->_reconnectMainNetworkDelay = reconnectMainNetworkDelay;
		}

		PubSubClientWrapper& setServer(uint8_t* ip, uint16_t port) {
			IPAddress addr(ip[0], ip[1], ip[2], ip[3]);
			return setServer(addr, port);
		}

		PubSubClientWrapper& setServer(IPAddress ip, uint16_t port) {
			this->ip = ip;
			this->port = port;
			this->domain = NULL;
			return *this;
		}

		PubSubClientWrapper& setServer(const char* domain, uint16_t port) {
			this->domain = domain;
			this->port = port;
			return *this;
		}
#if defined(WRAPPER_USE_SSL)
		bool synchClock() {
			if (_clockSynched)
				return true;
			if (_currentNetworkAdapter == NULL)
			{
				if (_logger)
					_logger->println("ERROR NETWORK NULL");
				return false;
			}
			if (!_currentNetworkAdapter->isConnected())
				return false;

			time_t now = _currentNetworkAdapter->getEpochTime();
			if (now == 0)
				return false;
			struct timeval tv;
			tv.tv_sec = now;
			settimeofday(&tv, NULL);
			setenv("TZ", "UTC", 1);
			tzset();

			//_sslClient.setX509Time(now);
			_clockSynched = true;
			return true;
		}
		void initSSL() override {
			/*if (_rootCA) {
				this->_sslClient.setCACert(_rootCA);
			}
			if (_clientCA && _clientKey) {
				this->_sslClient.setCertificate(_clientCA);
				this->_sslClient.setPrivateKey(_clientKey);
			}*/
			this->_sslClient.setTrustAnchors(this->_rootCA);
			if (_clientCA && _clientKey)
				this->_sslClient.setClientRSACert(this->_clientCA, this->_clientKey);
		}
#endif
		void setCredentials(const char* userName, const char* password) {
			this->_mqttPassword = password;
			this->_mqttUsername = userName;
		}
		void setConnectionAttributes(const char* willTopic = NULL, uint8_t willQos = 0, boolean willRetain = false, const char* willMessage = NULL, boolean cleanSession = true) {
			this->_mqttwillTopic = willTopic;
			this->_mqttwillQos = willQos;
			this->_mqttwillRetain = willRetain;
			this->_mqttwillMessage = willMessage;
			this->_mqttcleanSession = cleanSession;
		}

		PubSubClientWrapper& setStream(Stream& stream) {
			this->stream = &stream;
			return *this;
		}
		PubSubClientWrapper& setKeepAlive(uint16_t keepAlive) {
			this->keepAlive = keepAlive;
			return *this;
		}
		PubSubClientWrapper& setSocketTimeout(uint16_t timeout) {
			this->socketTimeout = timeout;
			return *this;
		}


		SubscribeHandler& setSubscription(const char* topicFilter, ArSubscribeHandlerFunction func) {
			SubscribeHandler* handler = new SubscribeHandler();
			handler->setTopicFilter(topicFilter);
			handler->setFunction(func);
			this->addSubscribeHandler(handler);
			return *handler;
		}
		SubscribeHandler& setSubscription(const char* topicFilter, ArSubscribeMessageHandlerFunction func) {
			SubscribeHandler* handler = new SubscribeHandler();
			handler->setTopicFilter(topicFilter);
			handler->setFunction(func);
			this->addSubscribeHandler(handler);
			return *handler;
		}
		PublishHandler& setPublisher(const char* topic, int interval, ArPublishHandlerFunction func) {
			PublishHandler* handler = new PublishHandler();
			handler->setTopic(topic);
			handler->setInterval(interval);
			handler->setFunction(func);
			this->addPublishHandler(handler);
			return *handler;
		}
		PublishHandler& setPublisher(const char* topic, int interval, int startDelay, ArPublishHandlerFunction func) {
			PublishHandler* handler = new PublishHandler();
			handler->setTopic(topic);
			handler->setInterval(interval);
			handler->setStartDelay(startDelay);
			handler->setFunction(func);
			this->addPublishHandler(handler);
			return *handler;
		}

		bool removePublisher(const char* topic) {
			int idx = -1;
			for (int i = 0; i < this->_publishHandlers.getSize(); i++)
			{
				PublishHandler* handler = this->_publishHandlers.get(idx);
				const char* _topic = handler->getTopic();
				if (strcmp(_topic, topic) == 0) {
					this->_publishHandlers.remove(idx);
					delete handler;
					return true;
				}
			}
			return false;
		}
		bool removeSubscription(const char* topicFilter) {
			int idx = -1;
			for (int i = 0; i < this->_subscribehandlers.getSize(); i++)
			{
				SubscribeHandler* handler = this->_subscribehandlers.get(idx);
				const char* _topicFilter = handler->getTopicFilter();
				if (strcmp(_topicFilter, topicFilter) == 0) {
					this->_subscribehandlers.remove(idx);
					delete handler;
					unsubscribe(_topicFilter);
					return true;
				}
			}
			return false;
		}

		boolean setBufferSize(uint16_t size) {
			if (size == 0) {
				// Cannot set it back to 0
				return false;
			}
			if (this->bufferSize == 0) {
				this->buffer = (uint8_t*)malloc(size);
			}
			else {
				uint8_t* newBuffer = (uint8_t*)realloc(this->buffer, size);
				if (newBuffer != NULL) {
					this->buffer = newBuffer;
				}
				else {
					return false;
				}
			}
			this->bufferSize = size;
			return (this->buffer != NULL);
		}
		uint16_t getBufferSize() {
			return this->bufferSize;
		}

		boolean connect() {
			if (!connected()) {
				int result = 0;
				if (_client->connected()) {
					result = 1;
				}
				else {
					if (domain != NULL) {
						result = _client->connect(this->domain, this->port);
					}
					else {
						result = _client->connect(this->ip, this->port);
					}
				}

				if (result == 1) {
					nextMsgId = 1;
					// Leave room in the buffer for header and variable length field
					uint16_t length = MQTT_MAX_HEADER_SIZE;
					unsigned int j;

	#if MQTT_VERSION == MQTT_VERSION_3_1
					uint8_t d[9] = { 0x00,0x06,'M','Q','I','s','d','p', MQTT_VERSION };
	#define MQTT_HEADER_VERSION_LENGTH 9
	#elif MQTT_VERSION == MQTT_VERSION_3_1_1
					uint8_t d[7] = { 0x00,0x04,'M','Q','T','T',MQTT_VERSION };
	#define MQTT_HEADER_VERSION_LENGTH 7
	#endif
					for (j = 0; j < MQTT_HEADER_VERSION_LENGTH; j++) {
						this->buffer[length++] = d[j];
					}

					uint8_t v;
					if (_mqttwillTopic) {
						v = 0x04 | (_mqttwillQos << 3) | (_mqttwillRetain << 5);
					}
					else {
						v = 0x00;
					}
					if (_mqttcleanSession) {
						v = v | 0x02;
					}

					if (_mqttUsername != NULL) {
						v = v | 0x80;

						if (_mqttPassword != NULL) {
							v = v | (0x80 >> 1);
						}
					}
					this->buffer[length++] = v;

					this->buffer[length++] = ((this->keepAlive) >> 8);
					this->buffer[length++] = ((this->keepAlive) & 0xFF);

					CHECK_STRING_LENGTH(length, _mqttClientId)
						length = writeString(_mqttClientId, this->buffer, length);
					if (_mqttwillTopic) {
						CHECK_STRING_LENGTH(length, _mqttwillTopic)
							length = writeString(_mqttwillTopic, this->buffer, length);
						CHECK_STRING_LENGTH(length, _mqttwillMessage)
							length = writeString(_mqttwillMessage, this->buffer, length);
					}

					if (_mqttUsername != NULL) {
						CHECK_STRING_LENGTH(length, _mqttUsername)
							length = writeString(_mqttUsername, this->buffer, length);
						if (_mqttPassword != NULL) {
							CHECK_STRING_LENGTH(length, _mqttPassword)
								length = writeString(_mqttPassword, this->buffer, length);
						}
					}

					write(MQTTCONNECT, this->buffer, length - MQTT_MAX_HEADER_SIZE);

					lastInActivity = lastOutActivity = millis();

					while (!_client->available()) {
						unsigned long t = millis();
						if (t - lastInActivity >= ((int32_t)this->socketTimeout * 1000UL)) {
							_state = MQTT_CONNECTION_TIMEOUT;
							_client->stop();
							return false;
						}
					}
					uint8_t llen;
					uint32_t len = readPacket(&llen);

					if (len == 4) {
						if (buffer[3] == 0) {
							lastInActivity = millis();
							pingOutstanding = false;
							_state = MQTT_CONNECTED;
							return true;
						}
						else {
							_state = buffer[3];
						}
					}
					_client->stop();
				}
				else {
					_state = MQTT_CONNECT_FAILED;
				}
				return false;
			}
			return true;
		}
		void disconnect() {
			this->buffer[0] = MQTTDISCONNECT;
			this->buffer[1] = 0;
			_client->write(this->buffer, 2);
			_state = MQTT_DISCONNECTED;
			_client->flush();
			_client->stop();
			lastInActivity = lastOutActivity = millis();
		}
		boolean publish(const char* topic, const char* payload) {
			return publish(topic, (const uint8_t*)payload, payload ? strnlen(payload, this->bufferSize) : 0, false);
		}

		boolean publish(const char* topic, const char* payload, boolean retained) {
			return publish(topic, (const uint8_t*)payload, payload ? strnlen(payload, this->bufferSize) : 0, retained);
		}

		boolean publish(const char* topic, const uint8_t* payload, unsigned int plength) {
			return publish(topic, payload, plength, false);
		}

		boolean publish(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained) {
			if (connected()) {
				if (this->bufferSize < MQTT_MAX_HEADER_SIZE + 2 + strnlen(topic, this->bufferSize) + plength) {
					// Too long
					return false;
				}
				// Leave room in the buffer for header and variable length field
				uint16_t length = MQTT_MAX_HEADER_SIZE;
				length = writeString(topic, this->buffer, length);

				// Add payload
				uint16_t i;
				for (i = 0; i < plength; i++) {
					this->buffer[length++] = payload[i];
				}

				// Write the header
				uint8_t header = MQTTPUBLISH;
				if (retained) {
					header |= 1;
				}
				return write(header, this->buffer, length - MQTT_MAX_HEADER_SIZE);
			}
			return false;
		}

		boolean publish_P(const char* topic, const char* payload, boolean retained) {
			return publish_P(topic, (const uint8_t*)payload, payload ? strnlen(payload, this->bufferSize) : 0, retained);
		}

		boolean publish_P(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained) {
			uint8_t llen = 0;
			uint8_t digit;
			unsigned int rc = 0;
			uint16_t tlen;
			unsigned int pos = 0;
			unsigned int i;
			uint8_t header;
			unsigned int len;
			int expectedLength;

			if (!connected()) {
				return false;
			}

			tlen = strnlen(topic, this->bufferSize);

			header = MQTTPUBLISH;
			if (retained) {
				header |= 1;
			}
			this->buffer[pos++] = header;
			len = plength + 2 + tlen;
			do {
				digit = len & 127; //digit = len %128
				len >>= 7; //len = len / 128
				if (len > 0) {
					digit |= 0x80;
				}
				this->buffer[pos++] = digit;
				llen++;
			} while (len > 0);

			pos = writeString(topic, this->buffer, pos);

			rc += _client->write(this->buffer, pos);

			for (i = 0; i < plength; i++) {
				rc += _client->write((char)pgm_read_byte_near(payload + i));
			}

			lastOutActivity = millis();

			expectedLength = 1 + llen + 2 + tlen + plength;

			return (rc == expectedLength);
		}

		// Start to publish a message.
		// This API:
		//   beginPublish(...)
		//   one or more calls to write(...)
		//   endPublish()
		// Allows for arbitrarily large payloads to be sent without them having to be copied into
		// a new buffer and held in memory at one time
		// Returns 1 if the message was started successfully, 0 if there was an error
		boolean beginPublish(const char* topic, unsigned int plength, boolean retained) {
			if (connected()) {
				// Send the header and variable length field
				uint16_t length = MQTT_MAX_HEADER_SIZE;
				length = writeString(topic, this->buffer, length);
				uint8_t header = MQTTPUBLISH;
				if (retained) {
					header |= 1;
				}
				size_t hlen = buildHeader(header, this->buffer, plength + length - MQTT_MAX_HEADER_SIZE);
				uint16_t rc = _client->write(this->buffer + (MQTT_MAX_HEADER_SIZE - hlen), length - (MQTT_MAX_HEADER_SIZE - hlen));
				lastOutActivity = millis();
				return (rc == (length - (MQTT_MAX_HEADER_SIZE - hlen)));
			}
			return false;
		}

		// Finish off this publish message (started with beginPublish)
		// Returns 1 if the packet was sent successfully, 0 if there was an error
		int endPublish() {
			return 1;
		}

		// Write a single byte of payload (only to be used with beginPublish/endPublish)
		virtual size_t write(uint8_t data) {
			lastOutActivity = millis();
			return _client->write(data);
		}

		// Write size bytes from buffer into the payload (only to be used with beginPublish/endPublish)
		// Returns the number of bytes written
		virtual size_t write(const uint8_t* buffer, size_t size) {
			lastOutActivity = millis();
			return _client->write(buffer, size);
		}
#if defined(WRAPPER_USE_SSL)
		bool manageConnection() {
			bool result = false;
			if (!this->_client || !this->_client->connected()) {
				Client* currentClient = NULL;
				for (int i = 0; i < this->_networkAdapters.getSize(); i++) {
					BaseNetworkAdapter* adapter = _networkAdapters.get(i);
					if (!adapter->isNetworkFailed()) {
						if (adapter->connect()) {
							_currentNetworkAdapter = adapter;
							currentClient = adapter->getClient();
						}
						break;
					}
				}
				if (currentClient == NULL) {
					return false;
				}
				if (this->_proxyClient == NULL) {
					this->_proxyClient = currentClient;
					this->_sslClient.setClient(this->_proxyClient);
					if (_logger != NULL) {
						_logger->print("Switching network to ");
						_logger->println(_currentNetworkAdapter->getTitle());
					}
					if (!this->synchClock()) {
						return false;
					}
				}
				else if (currentClient != this->_proxyClient) {
					if (_logger != NULL)
						_logger->println("Disconnect MQTT");
					disconnect();
					this->_proxyClient = currentClient;
					this->_sslClient.setClient(this->_proxyClient);
					if (_logger != NULL) {
						_logger->print("Switching network to ");
						_logger->println(_currentNetworkAdapter->getTitle());
					}
				}
			}
			if (this->_mainNetworkAdapter != NULL) {
				if (this->_mainNetworkAdapter != this->_currentNetworkAdapter) {
					if (millis() - _lastReconnectMainNetwork > _reconnectMainNetworkDelay) {
						if (!_isReconnectMainNetwork) {
							if (_logger != NULL) {
								_logger->print("Trying to connect main network to ");
								_logger->println(_mainNetworkAdapter->getTitle());
							}
							_mainNetworkAdapter->reset();
							_isReconnectMainNetwork = true;
						}
						else {
							if (!_mainNetworkAdapter->isNetworkFailed()) {
								if (_mainNetworkAdapter->connect()) {
									for (int i = 0; i < this->_networkAdapters.getSize(); i++) {
										BaseNetworkAdapter* adapter = _networkAdapters.get(i);
										adapter->reset();
									}
									disconnect();
									_currentNetworkAdapter = _mainNetworkAdapter;
									this->_proxyClient = _mainNetworkAdapter->getClient();
									this->_sslClient.setClient(this->_proxyClient);
								}
							}
							else {
								_lastReconnectMainNetwork = millis();
								_isReconnectMainNetwork = false;
							}
						}
					}
				}
				else {
					_lastConnectedMainNetwork = millis();
					_lastReconnectMainNetwork = millis();
				}
			}
			return connectMqtt();
		}
#else
		bool manageConnection() {
			bool result = false;
			if (!this->_client || !this->_client->connected()) {
				Client* currentClient = NULL;
				for (int i = 0; i < this->_networkAdapters.getSize(); i++) {
					BaseNetworkAdapter* adapter = _networkAdapters.get(i);
					if (!adapter->isNetworkFailed()) {
						if (adapter->connect()) {
							_currentNetworkAdapter = adapter;
							currentClient = adapter->getClient();
						}
						break;
					}
				}
				if (currentClient == NULL) {
					return false;
				}
				if (this->_client == NULL) {
					this->_client = currentClient;
					if (_logger != NULL) {
						_logger->print("Switching network to ");
						_logger->println(_currentNetworkAdapter->getTitle());
					}
				}
				else if (currentClient != this->_client) {
					if (_logger != NULL)
						_logger->println("Disconnect MQTT");
					disconnect();
					this->_client = currentClient;
					if (_logger != NULL) {
						_logger->print("Switching network to ");
						_logger->println(_currentNetworkAdapter->getTitle());
					}
				}
			}
			if (this->_mainNetworkAdapter != NULL) {
				if (this->_mainNetworkAdapter != this->_currentNetworkAdapter) {
					if (millis() - _lastReconnectMainNetwork > _reconnectMainNetworkDelay) {
						if (!_isReconnectMainNetwork) {
							if (_logger != NULL) {
								_logger->print("Trying to connect main network to ");
								_logger->println(_mainNetworkAdapter->getTitle());
							}
							_mainNetworkAdapter->reset();
							_isReconnectMainNetwork = true;
						}
						else {
							if (!_mainNetworkAdapter->isNetworkFailed()) {
								if (_mainNetworkAdapter->connect()) {
									for (int i = 0; i < this->_networkAdapters.getSize(); i++) {
										BaseNetworkAdapter* adapter = _networkAdapters.get(i);
										adapter->reset();
									}
									disconnect();
									_currentNetworkAdapter = _mainNetworkAdapter;
									this->_client = _mainNetworkAdapter->getClient();
								}
							}
							else {
								_lastReconnectMainNetwork = millis();
								_isReconnectMainNetwork = false;
							}
						}
					}
				}
				else {
					_lastConnectedMainNetwork = millis();
					_lastReconnectMainNetwork = millis();
				}
			}
			return connectMqtt();
		}
#endif

		boolean loop() {
			if (manageConnection()) {
				int handlerCount = this->_publishHandlers.getSize();
				for (int idx = 0; idx < handlerCount; idx++)
				{
					now = millis();
					PublishHandler* handler = _publishHandlers.get(idx);
					if (handler->canHandle(now)) {
						const char* message = handler->handleFunction();
						if (message != nullptr) {
							this->publish(handler->getTopic(), message);
						}
					}
				}
				now = millis();
				if ((now - lastInActivity > this->keepAlive * 1000UL) || (now - lastOutActivity > this->keepAlive * 1000UL)) {
					if (pingOutstanding) {
						this->_state = MQTT_CONNECTION_TIMEOUT;
						_client->stop();
						return false;
					}
					else {
						this->buffer[0] = MQTTPINGREQ;
						this->buffer[1] = 0;
						_client->write(this->buffer, 2);
						lastOutActivity = now;
						lastInActivity = now;
						pingOutstanding = true;
					}
				}

				if (_client->available()) {
					uint8_t llen;
					uint16_t len = readPacket(&llen);
					uint16_t msgId = 0;
					uint8_t* payload;
					if (len > 0) {
						lastInActivity = now;
						uint8_t type = this->buffer[0] & 0xF0;
						if (type == MQTTPUBLISH) {
							if (this->hasSubscription) {
								uint16_t tl = (this->buffer[llen + 1] << 8) + this->buffer[llen + 2]; /* topic length in bytes */
								memmove(this->buffer + llen + 2, this->buffer + llen + 3, tl); /* move topic inside buffer 1 byte to front */
								this->buffer[llen + 2 + tl] = 0; /* end the topic as a 'C' string with \x00 */
								char* topic = (char*)this->buffer + llen + 2;
								// msgId only present for QOS>0
								if ((this->buffer[0] & 0x06) == MQTTQOS1) {
									msgId = (this->buffer[llen + 3 + tl] << 8) + this->buffer[llen + 3 + tl + 1];
									payload = this->buffer + llen + 3 + tl + 2;
									this->subscriptionCallback(topic, payload, len - llen - 3 - tl - 2);

									this->buffer[0] = MQTTPUBACK;
									this->buffer[1] = 2;
									this->buffer[2] = (msgId >> 8);
									this->buffer[3] = (msgId & 0xFF);
									_client->write(this->buffer, 4);
									lastOutActivity = now;

								}
								else {
									payload = this->buffer + llen + 3 + tl;
									this->subscriptionCallback(topic, payload, len - llen - 3 - tl);
								}
							}
						}
						else if (type == MQTTPINGREQ) {
							this->buffer[0] = MQTTPINGRESP;
							this->buffer[1] = 0;
							_client->write(this->buffer, 2);
						}
						else if (type == MQTTPINGRESP) {
							pingOutstanding = false;
						}
					}
					else if (!connected()) {
						// readPacket has closed the connection
						return false;
					}
				}
				return true;
			}
			return false;
		}
		boolean connected() {
			boolean rc;
			if (_client == NULL) {
				rc = false;
			}
			else {
				rc = (int)_client->connected();
				if (!rc) {
					if (this->_state == MQTT_CONNECTED) {
						this->_state = MQTT_CONNECTION_LOST;
						_client->flush();
						_client->stop();
					}
				}
				else {
					return this->_state == MQTT_CONNECTED;
				}
			}
			return rc;
		}

		int state() {
			return this->_state;
		}

		bool connectMqtt() {
			bool result = false;
			if (!connected()) {
				if (millis() - _lastReconnect < 1000)
					return result;
				_connectionAttempt++;
				_lastReconnect = millis();
				if (_connectionAttempt > _maxReconnect) {
					if (enableLogger)
						_logger->println("Restart ESP...");
					ESP.restart();
				}
				if (!this->synchClock()) {
					return false;
				}
				if (enableLogger)
					_logger->print("Attempting MQTT connection: ");
				if (connect()) {
					if (enableLogger)
						_logger->println("Connected");
					int handlerCount = this->_subscribehandlers.getSize();
					for (int idx = 0; idx < handlerCount; idx++)
					{
						SubscribeHandler* handler = _subscribehandlers.get(idx);
						this->subscribe(handler->getTopicFilter());
					}
					_connectionAttempt = 0;
					hasSubscription = handlerCount > 0;
					result = true;
				}
				else {
					if (this->enableLogger) {
						_logger->print("Failed, Reason Code=");
						_logger->println(String(state()).c_str());
#if defined(WRAPPER_USE_SSL)
						char buff[80];
						if (!_sslClient.getLastSSLError(buff, sizeof(buff)) && _logger){
							_logger->print("SSL Error: ");
							_logger->println(buff);
						}
#endif
					}
				}
			}
			else
				result = true;
			return result;
		}

		void init() {
			for (int i = 0; i < _networkAdapters.getSize(); i++) {
				BaseNetworkAdapter * adapter = _networkAdapters.get(i);
				adapter->init();
			}
#if defined(WRAPPER_USE_SSL)
			initSSL();
#endif
			if (this->_mqttClientId == NULL || (this->_mqttClientId && !this->_mqttClientId[0])) {
	#if defined(ESP8266)
				String clientId = "ESP8266-";
	#elif defined(ESP32)
				String clientId = "ESP32-";
	#else
				String clientId = "Arduino-";
	#endif
	#if defined(WRAPPER_USE_WIFI) && (defined(ESP8266) || defined(ESP32))
				String macAddress = WiFi.macAddress();
				macAddress.remove(14, 1);
				macAddress.remove(11, 1);
				macAddress.remove(8, 1);
				macAddress.remove(5, 1);
				macAddress.remove(2, 1);
				macAddress.toUpperCase();
				clientId += macAddress;
	#else
				clientId += getRandomClientId(8);
	#endif

				int str_len = clientId.length() + 1;
				char* clientIdChars = (char*)malloc(str_len);
				clientId.toCharArray(clientIdChars, str_len);
				_mqttClientId = clientIdChars;
				if (enableLogger) {
					_logger->print("MQTT Client ID : ");
					_logger->println(_mqttClientId);
				}
			}
		}

	};
}
#endif