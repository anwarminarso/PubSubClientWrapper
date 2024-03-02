
#ifndef ESPWiFiNetworkAdapter_H
#define ESPWiFiNetworkAdapter_H
#include "CommonBase.h"
#include "PubSubHandler.h"

namespace a2n::iot {
	class ESPWiFiNetworkAdapter : public BaseNetworkAdapter {
	private:
#if defined(ESP32)
		bool _isClockSync = false;
		WiFiClass wifi = WiFi;
		WiFiClient _wifiClient;
#else
		ESP8266WiFiClass wifi = WiFi;
		WiFiClient _wifiClient;
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
					else {
						if (_logger)
							_logger->println("Failed");
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

		bool isConnected() override {
			return wifi.status() == WL_CONNECTED;
		}

		time_t getEpochTime() override {
			if (!_isClockSync) {
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
			return time(nullptr);
		}

	};
}
#endif