#pragma once
#ifndef PubSubClientWrapper_H
#define PubSubClientWrapper_H

#include "Arduino.h"
#include "IPAddress.h"
#include "Client.h"
#include "Stream.h"
#include "List.hpp"


#ifdef WRAPPER_USE_WIFI
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
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

#if defined(WRAPPER_USE_SSL)
#if defined(WRAPPER_USE_WIFI) && defined(ESP32)
#include <WiFiClientSecure.h>
#endif
#endif


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


#if defined(WRAPPER_USE_WIFI) && defined(WRAPPER_USE_MODEM)
#define TOTAL_NETWORK_ADAPTER 2
#elif defined(WRAPPER_USE_WIFI) || defined(WRAPPER_USE_MODEM)
#define TOTAL_NETWORK_ADAPTER 1
#else
#define TOTAL_NETWORK_ADAPTER 0
#endif // 


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


#ifndef _GLIBCXX_STD_FUNCTION_H
//void* operator new(size_t size, void* ptr) {
//	return ptr;
//}
#define _CUSTOM_USE_NOEXCEPT noexcept
inline void* operator new(size_t size, void* ptr) _CUSTOM_USE_NOEXCEPT
{
	return ptr;
}
namespace nonstd
{
	template<class T>struct tag { using type = T; };
	template<class Tag>using type_t = typename Tag::type;

	using size_t = decltype(sizeof(int));

	//move

	template<class T>
	T&& move(T& t) { return static_cast<T&&>(t); }

	//forward

	template<class T>
	struct remove_reference :tag<T> {};
	template<class T>
	struct remove_reference<T&> :tag<T> {};
	template<class T>using remove_reference_t = type_t<remove_reference<T>>;

	template<class T>
	T&& forward(remove_reference_t<T>& t) {
		return static_cast<T&&>(t);
	}
	template<class T>
	T&& forward(remove_reference_t<T>&& t) {
		return static_cast<T&&>(t);
	}

	//decay

	template<class T>
	struct remove_const :tag<T> {};
	template<class T>
	struct remove_const<T const> :tag<T> {};

	template<class T>
	struct remove_volatile :tag<T> {};
	template<class T>
	struct remove_volatile<T volatile> :tag<T> {};

	template<class T>
	struct remove_cv :remove_const<type_t<remove_volatile<T>>> {};


	template<class T>
	struct decay3 :remove_cv<T> {};
	template<class R, class...Args>
	struct decay3<R(Args...)> :tag<R(*)(Args...)> {};
	template<class T>
	struct decay2 :decay3<T> {};
	template<class T, size_t  N>
	struct decay2<T[N]> :tag<T*> {};

	template<class T>
	struct decay :decay2<remove_reference_t<T>> {};

	template<class T>
	using decay_t = type_t<decay<T>>;

	//is_convertible

	template<class T>
	T declval(); // no implementation

	template<class T, T t>
	struct integral_constant {
		static constexpr T value = t;
		constexpr integral_constant() {};
		constexpr operator T()const { return value; }
		constexpr T operator()()const { return value; }
	};
	template<bool b>
	using bool_t = integral_constant<bool, b>;
	using true_type = bool_t<true>;
	using false_type = bool_t<false>;

	template<class...>struct voider :tag<void> {};
	template<class...Ts>using void_t = type_t<voider<Ts...>>;

	namespace details {
		template<template<class...>class Z, class, class...Ts>
		struct can_apply :false_type {};
		template<template<class...>class Z, class...Ts>
		struct can_apply<Z, void_t<Z<Ts...>>, Ts...> :true_type {};
	}
	template<template<class...>class Z, class...Ts>
	using can_apply = details::can_apply<Z, void, Ts...>;

	namespace details {
		template<class From, class To>
		using try_convert = decltype(To{ declval<From>() });
	}
	template<class From, class To>
	struct is_convertible : can_apply< details::try_convert, From, To > {};
	template<>
	struct is_convertible<void, void> :true_type {};

	//enable_if

	template<bool, class = void>
	struct enable_if {};
	template<class T>
	struct enable_if<true, T> :tag<T> {};
	template<bool b, class T = void>
	using enable_if_t = type_t<enable_if<b, T>>;

	//res_of

	namespace details {
		template<class G, class...Args>
		using invoke_t = decltype(declval<G>()(declval<Args>()...));

		template<class Sig, class = void>
		struct res_of {};
		template<class G, class...Args>
		struct res_of<G(Args...), void_t<invoke_t<G, Args...>>> :
			tag<invoke_t<G, Args...>>
		{};
	}
	template<class Sig>
	using res_of = details::res_of<Sig>;
	template<class Sig>
	using res_of_t = type_t<res_of<Sig>>;

	//aligned_storage

	template<size_t size, size_t align>
	struct alignas(align) aligned_storage_t {
		char buff[size];
	};

	//is_same

	template<class A, class B>
	struct is_same :false_type {};
	template<class A>
	struct is_same<A, A> :true_type {};

	template<class Sig, size_t sz, size_t algn>
	struct small_task;

	template<class R, class...Args, size_t sz, size_t algn>
	struct small_task<R(Args...), sz, algn> {
		struct vtable_t {
			void(*mover)(void* src, void* dest);
			void(*destroyer)(void*);
			R(*invoke)(void const* t, Args&&...args);
			template<class T>
			static vtable_t const* get() {
				static const vtable_t table = {
				  [](void* src, void* dest) {
					new(dest) T(move(*static_cast<T*>(src)));
				  },
				  [](void* t) { static_cast<T*>(t)->~T(); },
				  [](void const* t, Args&&...args)->R {
					return (*static_cast<T const*>(t))(forward<Args>(args)...);
				  }
				};
				return &table;
			}
		};
		vtable_t const* table = nullptr;
		aligned_storage_t<sz, algn> data;
		template<class F,
			class dF = decay_t<F>,
			enable_if_t < !is_same<dF, small_task>{} > * = nullptr,
			enable_if_t < is_convertible< res_of_t<dF& (Args...)>, R >{} > * = nullptr
		>
		small_task(F && f) :
			table(vtable_t::template get<dF>())
		{
			static_assert(sizeof(dF) <= sz, "object too large");
			static_assert(alignof(dF) <= algn, "object too aligned");
			new(&data) dF(forward<F>(f));
		}
		~small_task() {
			if (table)
				table->destroyer(&data);
		}
		small_task(const small_task& o) :
			table(o.table)
		{
			data = o.data;
		}
		small_task(small_task&& o) :
			table(o.table)
		{
			if (table)
				table->mover(&o.data, &data);
		}
		small_task() {}
		small_task& operator=(const small_task& o) {
			this->~small_task();
			new(this) small_task(move(o));
			return *this;
		}
		small_task& operator=(small_task&& o) {
			this->~small_task();
			new(this) small_task(move(o));
			return *this;
		}
		explicit operator bool()const { return table; }
		R operator()(Args...args)const {
			return table->invoke(&data, forward<Args>(args)...);
		}
	};

	template<class R, class...Args, size_t sz, size_t algn>
	inline bool operator==(const small_task<R(Args...), sz, algn>& __f, nullptr_t)
	{
		return !static_cast<bool>(__f);
	}

	/// @overload
	template<class R, class...Args, size_t sz, size_t algn>
	inline bool  operator==(nullptr_t, const small_task<R(Args...), sz, algn>& __f)
	{
		return !static_cast<bool>(__f);
	}

	template<class R, class...Args, size_t sz, size_t algn>
	inline bool operator!=(const small_task<R(Args...), sz, algn>& __f, nullptr_t)
	{
		return static_cast<bool>(__f);
	}

	/// @overload
	template<class R, class...Args, size_t sz, size_t algn>
	inline bool operator!=(nullptr_t, const small_task<R(Args...), sz, algn>& __f)
	{
		return static_cast<bool>(__f);
	}

	template<class Sig>
	using function = small_task<Sig, sizeof(void*) * 4, alignof(void*) >;
}

#endif


namespace a2n::iot {
#ifdef _GLIBCXX_STD_FUNCTION_H
	typedef std::function<void(char*, uint8_t*, unsigned int)> ArSubscribeHandlerFunction;
	typedef std::function<void(const char*)> ArSubscribeMessageHandlerFunction;
	typedef std::function<bool(const char*, const char*)> ArSubscribeTopicFilterHandlerFunction;
	typedef std::function<String()> ArPublishHandlerFunction;
#else
	typedef nonstd::function<void(char*, uint8_t*, unsigned int)> ArSubscribeHandlerFunction;
	typedef nonstd::function<void(const char*)> ArSubscribeMessageHandlerFunction;
	typedef nonstd::function<bool(const char*, const char*)> ArSubscribeTopicFilterHandlerFunction;
	typedef nonstd::function<String()> ArPublishHandlerFunction;
#endif
	class SubscribeHandler {
	protected:
		const char* _topicFilter;
		ArSubscribeMessageHandlerFunction _func1;
		ArSubscribeHandlerFunction _func2;
		ArSubscribeTopicFilterHandlerFunction _funcFilter;
	public:
		const char* getTopicFilter() {
			return _topicFilter;
		}
		void setTopicFilter(const char* topicFilter) { _topicFilter = topicFilter; }
		void setFunction(ArSubscribeMessageHandlerFunction func) { _func1 = func; }
		void setFunction(ArSubscribeHandlerFunction func) { _func2 = func; }
		void setTopicFilterFunction(ArSubscribeTopicFilterHandlerFunction func) { _funcFilter = func; }
		bool canHandle(const char* topic) {
			if (_funcFilter)
				return _funcFilter(topic, _topicFilter);
			else
				return strcmp(topic, _topicFilter) == 0;
		}
		void handleFunction(char* topic, uint8_t* payload, unsigned int length) {
			if (_func1) {
				String message;
				for (int i = 0; i < length; i++)
					message += (char)payload[i];
				_func1(message.c_str());
			}
			else if (_func2)
				_func2(topic, payload, length);
		}
	};

	class PublishHandler {
	protected:
		const char* _topic;
		long _startDelay = 0;
		long _interval = 1000;
		long _delta = 0;
		uint32_t _lastMillis = 0;
		ArPublishHandlerFunction _func;
	public:
		void setTopic(const char* topic) { _topic = topic; }
		const char* getTopic() {
			return _topic;
		}
		bool isTopicEqual(const char* topic) {
			return strcmp(topic, _topic) == 0;
		}
		void setFunction(ArPublishHandlerFunction func) { _func = func; }
		void setInterval(long interval) { _interval = interval; }
		void setStartDelay(long startDelay) { _startDelay = startDelay; }
		bool canHandle(uint32_t now) {
			if (_startDelay > now)
				return false;
			if (_lastMillis > now)
				_lastMillis = 0;
			_delta = now - _lastMillis;
			return _interval <= _delta;
		}
		const char* handleFunction(void) {
			char* result = nullptr;
			if (_func) {
				String val = _func();
				int str_len = val.length() + 1;
				result = (char*)malloc(str_len);
				val.toCharArray(result, str_len);
			}
			_lastMillis = millis();
			return result;
		}
	};

	class BaseWrapper : public Print {
	protected:
#if defined(WRAPPER_USE_SSL)
#if defined(ESP8266)
		/*const uint8_t* _rootCA = NULL;
		const uint8_t* _clientCA = NULL;
		const uint8_t* _clientKey = NULL;*/
		 X509List* _rootCA = NULL;
		 X509List* _clientCA = NULL;
		 PrivateKey* _clientKey = NULL;
#else
		const char* _rootCA;
		const char* _clientCA;
		const char* _clientKey;
#endif
#endif
#if defined(WRAPPER_USE_WIFI)
		const char* _wifiHostName;
		const char* _wifiSSID;
		const char* _wifiPass;

		int _wifiMaxReconnect = 30;
		int _wifiReconnectDelay = 1000;
		int _wifiConnectionAttempt = 0;
		unsigned long _wifiLastReconnect = 0;
#endif
	public:
#ifdef WRAPPER_USE_SSL

#if defined(ESP8266)
		void setCACert(X509List &certificate)
		{
			this->_rootCA = &certificate;
		}
		void setCertificate(X509List &certificate)
		{
			this->_clientCA = &certificate;
		}
		void setPrivateKey(PrivateKey &private_key)
		{
			this->_clientKey = &private_key;
		}
		X509List* getCACert()
		{
			return this->_rootCA;
		}
		X509List* getCertificate()
		{
			return this->_clientCA;
		}
		PrivateKey* getPrivateKey()
		{
			return this->_clientKey;
		}
#else
		void setCACert(const char* rootCA)
		{
			this->_rootCA = rootCA;
		}
		void setCertificate(const char* client_ca)
		{
			this->_clientCA = client_ca;
		}
		void setPrivateKey(const char* private_key)
		{
			this->_clientKey = private_key;
		}
		const char* getCACert()
		{
			return this->_rootCA;
		}
		const char* getCertificate()
		{
			return this->_clientCA;
		}
		const char* getPrivateKey()
		{
			return this->_clientKey;
		}
#endif
#endif

#if defined(WRAPPER_USE_WIFI)
		void setWiFi(const char* hostName, const char* SSID, const char* wifiPassword) {
			this->_wifiHostName = hostName;
			this->_wifiSSID = SSID;
			this->_wifiPass = wifiPassword;
		}
		const char* getWiFiHostName() {
			return this->_wifiHostName;
		}
		const char* getWiFiSSID() {
			return this->_wifiSSID;
		}
		const char* getWiFiPassword() {
			return this->_wifiPass;
		}
#endif
	};

	class BaseNetworkAdapter {
	protected:
		int _connectionAttempt = 0;
		int _maxReconnectAttempt = 60;
		int _reconnectDelay = 1000;
		unsigned long lastTimeReconnect = 0;
		Client* _client = NULL;
		Print* _logger = NULL;
		bool _lastConnected = false;
		BaseWrapper* _wrapper = NULL;
	public:
		virtual const char* getTitle() {
			return "";
		}
		void setWrapper(BaseWrapper * wrapper) {
			_wrapper = wrapper;
		}
		BaseWrapper* getWrapper() {
			return _wrapper;
		}
		virtual void reset() {
			_connectionAttempt = 0;
		}
		virtual void init() {
		}
		bool isNetworkFailed() {
			return _connectionAttempt > _maxReconnectAttempt;
		}
		void setLogger(Print* logger) {
			this->_logger = logger;
		}
		Client* getClient() {
			return this->_client;
		}
		virtual bool connect() {
			if (_logger != NULL) {
				_logger->println("BaseNetworkAdapter::connect() is not implemented.");
			}
			return false;
		}
		virtual void printError() {
		}
	};
#if defined(WRAPPER_USE_WIFI)

#if defined(ESP32) || defined(ESP8266)
	class ESPWiFiNetworkAdapter : public BaseNetworkAdapter {
	private:
#if defined(ESP32)
		WiFiClass wifi = WiFi;
#else
		ESP8266WiFiClass wifi = WiFi;
#endif
#if !defined(WRAPPER_USE_SSL)
		WiFiClient _wifiClient;
#else
		WiFiClientSecure _wifiClient;
	#if defined(ESP8266)
		bool _isClockSync = false;
		void setClock() {
			configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
			if (this->_logger != NULL)
				this->_logger->print("Waiting for NTP time sync: ");
			time_t now = time(nullptr);
			while (now < 8 * 3600 * 2) {
				delay(500);
				if (this->_logger != NULL)
					this->_logger->print(".");
				now = time(nullptr);
			}
			struct tm timeinfo;
			gmtime_r(&now, &timeinfo);
			if (this->_logger != NULL) {
				this->_logger->println();
				this->_logger->print("Current time (UTC): ");
				this->_logger->println(asctime(&timeinfo));
			}
		}
	#endif
#endif
	public:
		ESPWiFiNetworkAdapter() {
			this->_client = &_wifiClient;
		}
		const char* getTitle() override {
#if defined(ESP32)
			return "ESP32 WiFi";
#else
			return "ESP8266 WiFi";
#endif
		}
		void reset() override {
			_connectionAttempt = 0;
		}
		void init() override {
		#if defined(WRAPPER_USE_SSL)
			#if defined(ESP32)
			_wifiClient.setCACert(_wrapper->getCACert());
			_wifiClient.setCertificate(_wrapper->getCertificate());
			_wifiClient.setPrivateKey(_wrapper->getPrivateKey());
			#else
			if (_wrapper->getCACert() != NULL)
			{
				/*const uint8_t* buffer = _wrapper->getCACert();
				_caCert.append(buffer, sizeof(buffer));*/
				//_caCert = new X509List(buffer, sizeof(buffer));
				_wifiClient.setTrustAnchors(_wrapper->getCACert());
			}
			if (_wrapper->getCertificate()  != NULL && _wrapper->getPrivateKey() != NULL) {
				/*const uint8_t* bufferCert = _wrapper->getCertificate();
				const uint8_t* bufferKey = _wrapper->getPrivateKey();
				_certificate.append(bufferCert, sizeof(bufferCert));
				_privateKey.parse(bufferKey, sizeof(bufferKey));*/
				_wifiClient.setClientRSACert(_wrapper->getCertificate(), _wrapper->getPrivateKey());
			}
			#endif
		#endif
			_connectionAttempt = 0;
#if defined(ESP32)
			wifi.persistent(false);
			wifi.disconnect(true, true);
			delay(100);
			wifi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
			delay(500);
			if (!wifi.setHostname(_wrapper->getWiFiHostName()))
			{
				if (this->_logger != NULL) {
					this->_logger->print("Failure to set hostname. Current Hostname : ");
					this->_logger->println(WiFi.getHostname());
				}
			}
#elif defined(ESP8266)
			wifi.disconnect(true);
			delay(1000);
#endif
			wifi.mode(WIFI_STA);
			wifi.begin(this->_wrapper->getWiFiSSID(), this->_wrapper->getWiFiPassword());
			if (this->_logger != NULL) {
				this->_logger->print("WiFi Initialized. SSID: ");
				this->_logger->print(this->_wrapper->getWiFiSSID());
				this->_logger->print(", MAC Address: ");
				this->_logger->println(WiFi.macAddress());
			}
			
		}
		bool connect() override {
			if (wifi.status() != WL_CONNECTED) {
				if (millis() - lastTimeReconnect >= _reconnectDelay) {
					_connectionAttempt++;
					lastTimeReconnect = millis();
					if (_connectionAttempt <= _maxReconnectAttempt) {
						if (_connectionAttempt == 1) {
							if (_logger) {
								_logger->print("Attempting WiFi connection.");
							}
							wifi.reconnect();
						}
						else {
							_lastConnected = false;
							if (_logger)
								_logger->print(".");
						}
					}
				}
				return false;
			}

			if (!_lastConnected && _logger) {
				_logger->println();
				_logger->print("Connected. IP Address: ");
				_logger->println(WiFi.localIP().toString());
#if defined(ESP8266) && defined(WRAPPER_USE_SSL)
				setClock();
#endif
			}
			_lastConnected = true;
			_connectionAttempt = 0;
			return true;
		}
#if defined(WRAPPER_USE_SSL)
		void printError() override {
			if (this->_logger == NULL)
				return;
			char buf[80];
	#if defined(ESP32)
			int error = _wifiClient.lastError(buf, sizeof(buf));
			if (error) {
				this->_logger->println("Network Error: " + String(error) + ", " + buf);
			}
	#else
			int sslError = _wifiClient.getLastSSLError(buf, sizeof(buf));
			if (sslError) {
				this->_logger->println("SSL Error: " + String(sslError) + ", " + buf);
			}
	#endif
		}
#endif
	};

#endif

#endif

	class PubSubClientWrapper : public BaseWrapper {
	private:
#pragma region Fields
		List<SubscribeHandler*> _subscribehandlers;
		List<PublishHandler*> _publishHandlers;
		List<BaseNetworkAdapter*> _networkAdapters;
		//BaseNetworkAdapter _networkAdapters[TOTAL_NETWORK_ADAPTER];
		Client* _client = NULL;
		BaseNetworkAdapter*  _currentNetworkAdapter = NULL;


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
			this->_client = NULL;
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

		void addNetworkAdapter(BaseNetworkAdapter * adapter) {
			_networkAdapters.add(adapter);
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

		void setClientId(const char* clientId) {
			this->_mqttClientId = clientId;
		}
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
						_logger->print("Switch network to ");
						_logger->println(_currentNetworkAdapter->getTitle());
					}
				}
				if (currentClient != this->_client) {
					if (_logger != NULL)
						_logger->println("Disconnect MQTT");
					disconnect();
					this->_client = currentClient;
					if (_logger != NULL) {
						_logger->print("Switch network to ");
						_logger->println(_currentNetworkAdapter->getTitle());
					}
				}
			}
			return connectMqtt();
		}

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
						if (_currentNetworkAdapter)
							_currentNetworkAdapter->printError();
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