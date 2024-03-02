
#ifndef ModemNetworkAdapter_H
#define ModemNetworkAdapter_H
#include "CommonBase.h"
#include "PubSubHandler.h"

#include <TinyGSM.h>

namespace a2n::iot {
	class ModemNetworkAdapter : public BaseNetworkAdapter {
	private:
		TinyGsm* _modem;
		TinyGsmClient* _modemClient;
		bool _isNTPSynch = false;
		const char* _ntpServer = NULL;
		// TimeZone Range (-47 to 48)
		// multiply by 4 to get the value
		// for example:
		// UTC+(07:00) => 7*4 = 28
		// UTC+(09:45) => 9*4 + ((45/60) * 4) = 36
		// UTC-(08:15) => -(8*4 + ((15/60) * 4)) = -33
	public:
		ModemNetworkAdapter() {}

		explicit ModemNetworkAdapter(TinyGsm& modem, TinyGsmClient& client) {
			_modem = &modem;
			_modemClient = &client;
			this->_client = _modemClient;
		}
		
		void setNTPServer(const char* server) {
			_ntpServer = server;
		}
		const char* getTitle() override {
			return "Modem";
		}
		void reset() override {
			_connectionAttempt = 0;
		}
		void init() override {
			_connectionAttempt = 0;
			_modem->restart();
			if (_wrapper->getModemPIN() && _modem->getSimStatus() != 3) {
				_modem->simUnlock(_wrapper->getModemPIN());
			}

			if (this->_logger != NULL) {
				String modemInfo = _modem->getModemInfo();
				this->_logger->print("Modem Info: ");
				this->_logger->println(modemInfo);
			}
			connectNetwork();
		}
		bool connectNetwork() {
			if (_logger)
				_logger->print("Waiting for network...");
			uint32_t timeout = _maxReconnectAttempt * _reconnectDelay;
			if (!_modem->waitForNetwork(timeout)) {
				if (_logger)
					_logger->println(" fail");
				_connectionAttempt = _maxReconnectAttempt;
				return false;
			}
			if (_modem->isNetworkConnected()) {
				if (_logger)
					_logger->println("connected to " + _modem->getOperator());
			}
			else {
				if (_logger)
					_logger->println("fail");
				_connectionAttempt = _maxReconnectAttempt;
				return false;
			}
			return true;
		}
		bool connect() override {
			if (!_modem->isNetworkConnected()) {
				if (!!connectNetwork())
					return false;
			}
			if (!_modem->isGprsConnected()) {
				if (millis() - lastTimeReconnect >= _reconnectDelay) {
					_connectionAttempt++;
					lastTimeReconnect = millis();
					if (_connectionAttempt <= _maxReconnectAttempt) {
						if (_connectionAttempt == 1) {
							if (_logger) {
								_logger->print("Attempting GPRS connection.");
							}
							_modem->gprsConnect(_wrapper->getModemAPN(), _wrapper->getModemUser(), _wrapper->getModemPass());
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
				_logger->println(_modem->getLocalIP());
			}
			_lastConnected = true;
			_connectionAttempt = 0;
			return true;
		}
		time_t getEpochTime() override {
			if (!_isNTPSynch) {
				if (!_modem->isNetworkConnected() || !_modem->isGprsConnected())
					return false;
				byte result = 0;
				if (_logger)
					_logger->println("NTP synch!!");
				if (_ntpServer == NULL)
					result = _modem->NTPServerSync("pool.ntp.org", 0);
				else
					result = _modem->NTPServerSync(this->_ntpServer, 0);
				if (result == 1)
					_isNTPSynch = true;
				else {
					if (_logger)
						_logger->println("Failed to sync NTP time");
					return 0;
				}
			}
			int year, month, day, hour, minute, second;
			float tz;
			_modem->getNetworkTime(&year, &month, &day, &hour, &minute, &second, &tz);
			if (_logger) {
				_logger->print("Network Time (UTC): ");
				_logger->print(year);
				_logger->print("-");
				_logger->print(month);
				_logger->print("-");
				_logger->print(day);
				_logger->print(" ");
				_logger->print(hour);
				_logger->print(":");
				_logger->print(minute);
				_logger->print(":");
				_logger->println(second);
			}
			struct tm timeinfo;
			timeinfo.tm_year = year - 1900;
			timeinfo.tm_mon = month - 1; // months sice January
			timeinfo.tm_mday = day; //day of the month
			timeinfo.tm_hour = hour;
			timeinfo.tm_min = minute;
			timeinfo.tm_sec = second;
			time_t now = mktime(&timeinfo);
			return now;
		}
		bool isConnected() override {
			return _modem->isNetworkConnected();
		}
	};
}
#endif