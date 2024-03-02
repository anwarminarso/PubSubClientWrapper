
#ifndef CustomNetworkAdapter_H
#define CustomNetworkAdapter_H
#include "CommonBase.h"
#include "PubSubHandler.h"

namespace a2n::iot {

#ifdef _GLIBCXX_STD_FUNCTION_H
	typedef std::function<bool()> ArBoolHandler;
	typedef std::function<time_t()> ArTimeHandler;
#else
	typedef nonstd::function<bool()> ArBoolHandler;
	typedef nonstd::function<time_t()> ArTimeHandler;
#endif

	class CustomNetworkAdapter : public BaseNetworkAdapter {
	private:
		const char* _title;
		ArBoolHandler _onInit = NULL;
		ArBoolHandler _onConnect = NULL;
		ArBoolHandler _isConnected = NULL;
		ArBoolHandler _NTPSynch = NULL;
		ArTimeHandler _timeHandler = NULL;
	public:
		CustomNetworkAdapter(Client &client, const char* title) {
			this->_client = &client;
			this->_title = title;
		}
		const char* getTitle() override {
			return this->_title;
		}
		void reset() override {
			_connectionAttempt = 0;
		}
		void setTimeHandler(ArTimeHandler handler) {
			_timeHandler = handler;
		}
		time_t getEpochTime() override {
			time_t now = time(NULL);
			if (_timeHandler != NULL) {
				now = _timeHandler();
			}
			return now;
		}
		void onInit(ArBoolHandler handler) {
			_onInit = handler;
		}
		void onConnect(ArBoolHandler handler) {
			_onConnect = handler;
		}
		void onIsConnected(ArBoolHandler handler) {
			_isConnected = handler;
		}
		void init() override {
			if (_onInit == NULL || !_onInit()) {
				if (!_logger) {
					_logger->print("ERROR initializing network adapter: ");
					_logger->println(_title);
				}
			}
		}
		bool connect() override {
			if (_onConnect == NULL || !_onConnect())
				return false;
			return true;
		}

		bool isConnected() override {
			if (_isConnected == NULL || !_isConnected())
				return false;
			return true;
		}
	};
}
#endif