
#ifndef CommonBase_H
#define CommonBase_H


#ifdef WRAPPER_USE_SSL
#include <ESP_SSLClient.h>
#endif

namespace a2n::iot {
	class BaseWrapper : public Print {
	protected:
		bool _clockSynched = false;
		const char* _NTPServer;
		int _NTPTimeZone;
#if defined(WRAPPER_USE_SSL)

		X509List* _rootCA = NULL;
		X509List* _clientCA = NULL;
		PrivateKey* _clientKey = NULL;
 
//#if defined(ESP8266)
//		/*const uint8_t* _rootCA = NULL;
//		const uint8_t* _clientCA = NULL;
//		const uint8_t* _clientKey = NULL;*/
//		X509List* _rootCA = NULL;
//		X509List* _clientCA = NULL;
//		PrivateKey* _clientKey = NULL;
//#else
//		const char* _rootCA;
//		const char* _clientCA;
//		const char* _clientKey;
//#endif
		virtual void initSSL() {
		}
#endif
#if defined(WRAPPER_USE_WIFI)
		const char* _wifiHostName;
		const char* _wifiSSID;
		const char* _wifiPass;
#endif
#if defined(WRAPPER_USE_MODEM)
		const char* _modemPIN = NULL;

		const char* _modemAPN;
		const char* _modemUser;
		const char* _modemPass;
#endif
	public:
#ifdef WRAPPER_USE_SSL

//#if defined(ESP8266)
//		void setCACert(X509List& certificate)
//		{
//			this->_rootCA = &certificate;
//		}
//		void setCertificate(X509List& certificate)
//		{
//			this->_clientCA = &certificate;
//		}
//		void setPrivateKey(PrivateKey& private_key)
//		{
//			this->_clientKey = &private_key;
//		}
//		X509List* getCACert()
//		{
//			return this->_rootCA;
//		}
//		X509List* getCertificate()
//		{
//			return this->_clientCA;
//		}
//		PrivateKey* getPrivateKey()
//		{
//			return this->_clientKey;
//		}
//#else
//		void setCACert(const char* rootCA)
//		{
//			this->_rootCA = rootCA;
//		}
//		void setCertificate(const char* client_ca)
//		{
//			this->_clientCA = client_ca;
//		}
//		void setPrivateKey(const char* private_key)
//		{
//			this->_clientKey = private_key;
//		}
//		const char* getCACert()
//		{
//			return this->_rootCA;
//		}
//		const char* getCertificate()
//		{
//			return this->_clientCA;
//		}
//		const char* getPrivateKey()
//		{
//			return this->_clientKey;
//		}
//#endif

		void setCACert(X509List& certificate)
		{
			this->_rootCA = &certificate;
		}
		void setCertificate(X509List& certificate)
		{
			this->_clientCA = &certificate;
		}
		void setPrivateKey(PrivateKey& private_key)
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
#if defined(WRAPPER_USE_MODEM)
		void setModem(const char* modemAPN, const char* modemUser, const char* modemPass) {
			this->_modemAPN = modemAPN;
			this->_modemUser = modemUser;
			this->_modemPass = modemPass;
		}
		void setModemPIN(const char* modemPIN) {
			this->_modemPIN = modemPIN;
		}
		const char* getModemAPN() {
			return this->_modemAPN;
		}
		const char* getModemUser() {
			return this->_modemUser;
		}
		const char* getModemPass() {
			return this->_modemPass;
		}
		const char* getModemPIN() {
			return this->_modemPIN;
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
		void setWrapper(BaseWrapper* wrapper) {
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
		virtual bool isNetworkFailed() {
			return _connectionAttempt > _maxReconnectAttempt;
		}
		virtual time_t getEpochTime() {
			return time(nullptr);
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
		virtual bool isConnected() {
			return false;
		}
	};
}
#endif