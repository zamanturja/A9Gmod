#include "A9Gmod.h"

#ifndef GF
#define GF(x) (reinterpret_cast<const __FlashStringHelper *>(PSTR(x)))
#endif

/* ------------------------------------------------------------------
 *                   A9G IMPLEMENTATION
 * ------------------------------------------------------------------ */

A9G::A9G(bool debugMode)
  : _modemStream(nullptr),
    _debugMode(debugMode),
    _defaultWaitMS(60000),
    _hasSMS(false),
    _smsIndex(0),
    _onEventCallback(nullptr) {}

/**
 * @brief Initialize the A9G module by sending "AT" and waiting for "OK".
 */
bool A9G::init(Stream *serial) {
  _modemStream = serial;
  // Send basic "AT" check
  _modemStream->println("AT");

  // Wait a couple of seconds for the "OK" response
  if (_waitForOkResponse(2000)) {
    if (_debugMode) {
      Serial.println("[A9G] A9G module responded OK!");
    }
    return true;
  }
  if (_debugMode) {
    Serial.println("[A9G] A9G module not responding!");
  }
  return false;
}

/**
 * @brief Just an example test function that prints out data infinitely.
 */
void A9G::testDebugCommand(char *data) {
  if (!_modemStream) return;
  _modemStream->println(data);
  while (true) {
    while (_modemStream->available()) {
      Serial.write(_modemStream->read());
    }
  }
}

/**
 * @brief Continuously parse available data from the modem
 *        and trigger event callbacks if relevant data is found.
 */
void A9G::pollModem() {
  if (!_modemStream) return;
  _internalModemParser();
}

/**
 * @brief AT+EGMR=2,7 to read IMEI
 */
void A9G::readIMEI() {
  if (!_modemStream) return;
  _modemStream->println("AT+EGMR=2,7");
  _waitForOkResponse(1000);
}

/**
 * @brief AT+CSQ to read signal quality
 */
void A9G::readSignalQuality() {
  if (!_modemStream) return;
  _modemStream->println("AT+CSQ");
  _waitForOkResponse(1000);
}

/**
 * @brief AT+CCID to read SIM CCID
 */
void A9G::readCCID() {
  if (!_modemStream) return;
  _modemStream->println("AT+CCID");
  _waitForOkResponse(1000);
}

/**
 * @brief Wait for the "READY" message (blocking).
 */
bool A9G::waitForModemReady() {
  if (!_modemStream) return false;

  char buffer[100];
  int idx = 0;
  memset(buffer, 0, sizeof(buffer));

  _modemStream->println("AT");
  while (true) {
    if (_modemStream->available()) {
      char c = _modemStream->read();
      buffer[idx++] = c;
      if (idx >= (int)sizeof(buffer) - 1) {
        idx = 0;
      }
      buffer[idx] = '\0';

      if (strstr(buffer, "READY") != nullptr) {
        return true;
      }
      if (strstr(buffer, "NO SIM CARD") != nullptr) {
        if (_debugMode) {
          Serial.println("No SIM card detected!");
        }
      }
    }
  }
  return false;
}

/* ----------------------------------------------------
 *         GPRS & APN 
 * ---------------------------------------------------- */
bool A9G::isGPRSAttached() {
  if (!_modemStream) return false;
  _modemStream->println("AT+CGATT?");
  return _waitForOkResponse(2000);
}

bool A9G::attachGPRS(const char* apn, const char* user, const char* pwd) {
  if (!_modemStream) return false;
  _modemStream->println("AT+CGATT=1");
  _modemStream->print("+CGDCONT=1,\"IP\",\"");
  _modemStream->print(apn);
  _modemStream->println("\"");
  if (!user) user = "";
  if (!pwd)  pwd  = "";
  _modemStream->print("+CSTT=\"");
  _modemStream->print(apn);
  _modemStream->print("\",\"");
  _modemStream->print(user);
  _modemStream->print("\",\"");
  _modemStream->print(pwd);
  _modemStream->println("\"");
  _modemStream->println("+CGACT=1,1");
  _modemStream->println("+CIPMUX=1");
  return _waitForOkResponse(2000);
}

bool A9G::detachGPRS() {
  if (!_modemStream) return false;
  _modemStream->println("AT+CGATT=0");
  return _waitForOkResponse(2000);
}

bool A9G::setAPN(const char *pdpType, const char *apn) {
  if (!_modemStream) return false;
  _modemStream->print("AT+CGDCONT=1,\"");
  _modemStream->print(pdpType);
  _modemStream->print("\",\"");
  _modemStream->print(apn);
  _modemStream->println("\"");
  return _waitForOkResponse(2000);
}

bool A9G::activatePDP() {
  if (!_modemStream) return false;
  _modemStream->println("AT+CGACT=1,1");
  return _waitForOkResponse(2000);
}

bool A9G::deactivatePDP() {
  // Placeholder if desired:
  // _modemStream->println("AT+CGACT=0,1");
  // return _waitForOkResponse(2000);
  return false;
}

/* ----------------------------------------------------
 *         GPS 
 * ---------------------------------------------------- */
bool A9G::enableGPS() {
  if (!_modemStream) return false;
  _modemStream->println("AT+GPS=1");
  return _waitForOkResponse(2000);
}

bool A9G::disableGPS() {
  if (!_modemStream) return false;
  _modemStream->println("AT+GPS=0");
  return _waitForOkResponse(2000);
}

bool A9G::enableAGPS() {
  if (!_modemStream) return false;
  _modemStream->println("AT+AGPS=1");
  return _waitForOkResponse(2000);
}

/**
 * @brief Sends AT+GPSRD=1, then collects whatever GPS NMEA data arrives 
 *        for ~1 second. Returns that raw data to the caller.
 */
String A9G::getGPS() {
  if (!_modemStream) return "";

  // Start GPS data output:
  _modemStream->println("AT+GPSRD=1");
  _waitForOkResponse(500);

  // Read for ~1 second
  unsigned long start = millis();
  String result;
  while (millis() - start < 1000) {
    while (_modemStream->available()) {
      result += (char)_modemStream->read();
    }
  }
  return result;
}

/* ----------------------------------------------------
 *         MQTT 
 * ---------------------------------------------------- */
bool A9G::connectBroker(const char *broker, int port,
                        const char *user, const char *pass,
                        const char *clientID, uint8_t keepAlive,
                        uint16_t cleanSession) {
  if (!_modemStream) return false;
  _modemStream->print("AT+MQTTCONN=\"");
  _modemStream->print(broker);
  _modemStream->print("\",");
  _modemStream->print(port);
  _modemStream->print(",\"");
  _modemStream->print(clientID);
  _modemStream->print("\",");
  _modemStream->print(keepAlive);
  _modemStream->print(",");
  _modemStream->print(cleanSession);
  _modemStream->print(",\"");
  _modemStream->print(user);
  _modemStream->print("\",\"");
  _modemStream->print(pass);
  _modemStream->println("\"");
  return _waitForOkResponse(2000);
}

bool A9G::connectBroker(const char *broker, int port,
                        const char *clientID,
                        uint8_t keepAlive, uint16_t cleanSession) {
  if (!_modemStream) return false;
  _modemStream->print("AT+MQTTCONN=\"");
  _modemStream->print(broker);
  _modemStream->print("\",");
  _modemStream->print(port);
  _modemStream->print(",\"");
  _modemStream->print(clientID);
  _modemStream->print("\",");
  _modemStream->print(keepAlive);
  _modemStream->print(",");
  _modemStream->println(cleanSession);
  return _waitForOkResponse(2000);
}

bool A9G::connectBroker(const char *broker, int port) {
  if (!_modemStream) return false;
  char tempID[10] = { 0 };
  sprintf(tempID, "%ld", random(10000, 99999));
  _modemStream->print("AT+MQTTCONN=\"");
  _modemStream->print(broker);
  _modemStream->print("\",");
  _modemStream->print(port);
  _modemStream->print(",\"");
  _modemStream->print(tempID);
  _modemStream->print("\",120,0");
  _modemStream->println();
  return _waitForOkResponse(2000);
}

bool A9G::disconnectBroker() {
  if (!_modemStream) return false;
  _modemStream->println("AT+MQTTDISCONN");
  return _waitForOkResponse(2000);
}

bool A9G::subscribeTopic(const char *topic, uint8_t qos, unsigned long timeout) {
  if (!_modemStream) return false;
  _modemStream->print("AT+MQTTSUB=\"");
  _modemStream->print(topic);
  _modemStream->print("\",");
  _modemStream->print(qos);
  _modemStream->print(",");
  _modemStream->println(timeout);
  return _waitForOkResponse(2000);
}

bool A9G::subscribeTopic(const char *topic) {
  if (!_modemStream) return false;
  _modemStream->print("AT+MQTTSUB=\"");
  _modemStream->print(topic);
  _modemStream->print("\",1,0");
  _modemStream->println();
  return _waitForOkResponse(2000);
}

bool A9G::unsubscribeTopic(const char *topic) {
  if (!_modemStream) return false;
  _modemStream->print("AT+MQTTUNSUB=\"");
  _modemStream->print(topic);
  _modemStream->println("\"");
  return _waitForOkResponse(2000);
}

bool A9G::publishTopic(const char *topic, const char *msg) {
  if (!_modemStream) return false;
  _modemStream->print("AT+MQTTPUB=\"");
  _modemStream->print(topic);
  _modemStream->print("\",\"");
  _modemStream->print(msg);
  _modemStream->println("\",2,0,0");
  return _waitForOkResponse(2000);
}

/* ----------------------------------------------------
 *         SMS 
 * ---------------------------------------------------- */
bool A9G::activateTextMode() {
  if (!_modemStream) return false;
  _modemStream->println("AT+CNMI=0,1,0,0,0");
  return _waitForOkResponse(2000);
}

bool A9G::setSMSFormatReading(bool mode) {
  if (!_modemStream) return false;
  _modemStream->print("AT+CMGF=");
  _modemStream->println(mode ? 1 : 0);
  return _waitForOkResponse(2000);
}

bool A9G::setMessageStorage() {
  if (!_modemStream) return false;
  _modemStream->println("AT+CPMS=\"ME\",\"ME\",\"ME\"");
  return _waitForOkResponse(2000);
}

void A9G::checkMessageStorage() {
  if (!_modemStream) return;
  _modemStream->println("AT+CPBS?");
}

void A9G::readSMS(uint8_t index) {
  if (!_modemStream) return;
  _modemStream->print("AT+CMGR=");
  _modemStream->println(index);
}

void A9G::deleteSMS(uint8_t index, A9G_MessageType type) {
  if (!_modemStream) return;
  _modemStream->print("AT+CMGD=");
  _modemStream->print(index);
  _modemStream->print(",");
  _modemStream->println(type);
}

bool A9G::sendSMS(const char *number, const char *message) {
  if (!_modemStream) return false;
  _modemStream->println("AT+CMGF=1");
  delay(100);
  _modemStream->print("AT+CMGS=\"");
  _modemStream->print(number);
  _modemStream->print("\"\r\n");
  delay(100);
  _modemStream->print(message);
  _modemStream->write(0x1A);  // Ctrl+Z
  delay(100);
  _modemStream->println("AT");
  return _waitForOkResponse(2000);
}

void A9G::sendSMSNonBlocking(const char *number, const char *message) {
  if (!_modemStream) return;
  _modemStream->print("AT+CMGS=\"");
  _modemStream->print(number);
  _modemStream->print("\"\r\n");
  delay(100);
  _modemStream->print(message);
  _modemStream->write((char)26);  // Ctrl+Z
  delay(100);
  _modemStream->println("AT");
}

/* ----------------------------------------------------
 *         ERROR PRINTS 
 * ---------------------------------------------------- */
void A9G::printCMEError(int err) {
  // Just an example printing of known codes:
  Serial.print("CME Error code: ");
  Serial.println(err);
}

void A9G::printCMSError(int err) {
  // Just an example printing of known codes:
  Serial.print("CMS Error code: ");
  Serial.println(err);
}

/**
 * @brief Register user callback for dispatching events
 */
void A9G::setEventCallback(void (*cb)(A9G_Event *)) {
  _onEventCallback = cb;
}

/* ------------------------------------------------------------------
 *   INTERNAL PARSING HELPERS
 * ------------------------------------------------------------------ */

/**
 * @brief Waits up to 'timeout' ms for the substring "OK" from the modem.
 * @return true if found "OK", false otherwise
 */
bool A9G::_waitForOkResponse(int timeout) {
  if (!_modemStream) return false;

  char response[150];
  memset(response, 0, sizeof(response));
  long start_time = millis();
  int idx = 0;

  // Create a minimal event structure in case we parse some inbound lines
  A9G_Event *evt = (A9G_Event *)malloc(sizeof(A9G_Event));
  if (!evt) return false;

  // Zero out the event
  memset(evt, 0, sizeof(A9G_Event));

  while ((millis() - start_time) < (unsigned long)timeout) {
    while (_modemStream->available()) {
      char c = _modemStream->read();
      if (idx < (int)sizeof(response) - 1) {
        response[idx++] = c;
        response[idx] = '\0';
      }

      // If we see "OK" anywhere in the buffer, success
      if (strstr(response, "OK")) {
        free(evt);
        return true;
      }
    }
  }
  free(evt);
  return false;
}

/**
 * @brief Main internal parser that checks for +TERM lines
 */
void A9G::_internalModemParser() {
  A9G_Event *evt = (A9G_Event *)malloc(sizeof(A9G_Event));
  if (!evt) return;
  memset(evt, 0, sizeof(A9G_Event));

  static char term[100];
  static char termData[128];
  static bool termFound = false;
  static bool termEnded = false;
  static bool readingTermData = false;
  static int termLen = 0;
  static int termDataLen = 0;

  while (_modemStream->available()) {
    char c = _modemStream->read();
    // Detect start of +TERM
    if (c == '+' && !termFound) {
      termFound = true;
      termEnded = false;
      readingTermData = false;
      termLen = 0;
      termDataLen = 0;
      continue;
    }
    // If found +TERM and see '=' or ':', then the "term" ends
    if ((c == '=' || c == ':') && termFound && !termEnded) {
      term[termLen] = '\0';
      termEnded = true;
      // Identify it
      int id = _identifyTermString(term);
      evt->id = (A9G_EventID)id;
      continue;
    }
    // Building the +TERM
    if (termFound && !termEnded) {
      term[termLen++] = c;
      if (termLen >= 99) {
        termFound = false;
      }
    } else if (termEnded && !readingTermData) {
      // Now we read the data part
      readingTermData = true;
    } else if (termEnded && readingTermData) {
      if (c == '\r') {
        // We have the full termData
        _handlePotentialEvent(evt, termData, termDataLen);
        // Dispatch if needed
        _dispatchEvent(evt);

        // Reset flags
        termFound = false;
        termEnded = false;
        readingTermData = false;
        termLen = 0;
        termDataLen = 0;
        memset(term, 0, sizeof(term));
        memset(termData, 0, sizeof(termData));
        break;
      } else {
        if (termDataLen < 127) {
          termData[termDataLen++] = c;
          termData[termDataLen] = '\0';
        }
      }
    }
  }
  free(evt);
}

/**
 * @brief Identify the term string to match an event ID
 */
uint8_t A9G::_identifyTermString(const char *termStr) {
  const char *availableTerms[] = {
    "CREG", "CTZV", "CIEV", "CPMS", "CMT", "CMTI", "CMGL", "CMGR", "GPSRD", "CGATT", "AGPS",
    "GPNT", "MQTTPUBLISH", "CMGS", "CME ERROR", "CMS ERROR", "CSQ", "EGMR", "CCID"
  };
  // Map them in the same order as A9G_EventID
  for (int i = 0; i < 19; i++) {
    if (!strcmp(termStr, availableTerms[i])) {
      return i;  // match index
    }
  }
  return EV_NONE;
}

/**
 * @brief Given the event +TERM and the data portion, fill in the A9G_Event structure
 */
 void A9G::_handlePotentialEvent(A9G_Event *evt, const char *data, int len) {
  if (evt->id == EV_MQTTPUBLISH) {
    
    // We use pointers into the data buffer (which may not be null-terminated, so we work only within len)
    const char *p = data;
    
    // Skip any leading comma
    if (len > 0 && *p == ',') {
      p++;
    }
    
    // Calculate offset from the beginning of data to maintain length safety
    int offset = p - data;
    int remaining = len - offset;
    
    // Find the first comma (which ends token1: topic)
    const char *firstComma = NULL;
    for (int i = 0; i < remaining; i++) {
      if (p[i] == ',') {
        firstComma = p + i;
        break;
      }
    }
    if (firstComma) {
      int topicLen = firstComma - p;
      if (topicLen > (int)sizeof(evt->topic) - 1)
        topicLen = sizeof(evt->topic) - 1;
      strncpy(evt->topic, p, topicLen);
      evt->topic[topicLen] = '\0';
      
      // Now, move past the first comma
      p = firstComma + 1;
      offset = p - data;
      remaining = len - offset;
      
      // Find the next comma (which ends token2, the numeric field)
      const char *secondComma = NULL;
      for (int i = 0; i < remaining; i++) {
        if (p[i] == ',') {
          secondComma = p + i;
          break;
        }
      }
      if (secondComma) {
        // Skip token2 by moving past this comma:
        p = secondComma + 1;
        offset = p - data;
        remaining = len - offset;
        
        // Look for the next comma; if found, payload starts after it.
        const char *thirdComma = NULL;
        for (int i = 0; i < remaining; i++) {
          if (p[i] == ',') {
            thirdComma = p + i;
            break;
          }
        }
        if (thirdComma) {
          p = thirdComma + 1;
          // Now, remaining length for payload:
          int payloadLen = len - (p - data);
          if (payloadLen > (int)sizeof(evt->message) - 1)
            payloadLen = sizeof(evt->message) - 1;
          strncpy(evt->message, p, payloadLen);
          evt->message[payloadLen] = '\0';
        } else {
          // If no third comma is found, assume the rest is payload.
          int payloadLen = remaining;
          if (payloadLen > (int)sizeof(evt->message) - 1)
            payloadLen = sizeof(evt->message) - 1;
          strncpy(evt->message, p, payloadLen);
          evt->message[payloadLen] = '\0';
        }
      }
    }
  } else if (evt->id == EV_CME) {
    // parse numeric code
    evt->error = atoi(data);
  } else if (evt->id == EV_CMTI) {
    // new SMS index
    // ...
  } else if (evt->id == EV_CMGS || evt->id == EV_CMGL) {
    // handle SMS
    // ...
  } else if (evt->id == EV_CSQ) {
    // parse signal quality
    char tmp[10] = { 0 };
    for (int i = 0; i < len; i++) {
      if (data[i] == ',') break;
      tmp[i] = data[i];
    }
    evt->param1 = atoi(tmp);
  }
  // etc. for other event types
}


/**
 * @brief After filling the A9G_Event, call the user callback if set.
 */
void A9G::_dispatchEvent(A9G_Event *evt) {
  if (_onEventCallback) {
    _onEventCallback(evt);
  }
}


/* ------------------------------------------------------------------
 *                   A9Gmod IMPLEMENTATION
 * ------------------------------------------------------------------ */

A9Gmod *A9Gmod::_activeInstance = nullptr;

A9Gmod::A9Gmod(A9G &a9gRef)
  : _a9g(&a9gRef),
    _mqttConnected(false),
    _mqttBroker(""),
    _mqttPort(1883),
    _mqttUserCallback(nullptr) {
  _activeInstance = this;
  // Tie into the A9G's event system
  _a9g->setEventCallback(_onModemEvent);
}

void A9Gmod::setMQTTServer(const char *host, uint16_t port) {
  _mqttBroker = host;
  _mqttPort = port;
}

void A9Gmod::onMQTTMessage(A9G_MQTTCallback callback) {
  _mqttUserCallback = callback;
}

bool A9Gmod::connectMQTT(const char *clientID) {
  bool ok = _a9g->connectBroker(_mqttBroker.c_str(), _mqttPort, clientID, 60, 1);
  _mqttConnected = ok;
  return ok;
}

bool A9Gmod::connectMQTT(const char *clientID,
                         const char *user,
                         const char *pass,
                         uint8_t keepAlive,
                         uint16_t cleanSession) {
  bool ok = _a9g->connectBroker(_mqttBroker.c_str(), _mqttPort, user, pass,
                                clientID, keepAlive, cleanSession);
  _mqttConnected = ok;
  return ok;
}

bool A9Gmod::isMQTTConnected() {
  return _mqttConnected;
}

void A9Gmod::processMQTT() {
  // Pump the A9G parser
  _a9g->pollModem();
}

bool A9Gmod::publishMQTT(const char *topic, const char *payload) {
  if (!_mqttConnected) return false;
  return _a9g->publishTopic(topic, payload);
}

bool A9Gmod::subscribeMQTT(const char *topic) {
  if (!_mqttConnected) return false;
  return _a9g->subscribeTopic(topic);
}

bool A9Gmod::subscribeMQTT(const char *topic, uint8_t qos, unsigned long timeout) {
  if (!_mqttConnected) return false;
  return _a9g->subscribeTopic(topic, qos, timeout);
}

bool A9Gmod::unsubscribeMQTT(const char *topic) {
  if (!_mqttConnected) return false;
  return _a9g->unsubscribeTopic(topic);
}

bool A9Gmod::disconnectMQTT() {
  if (!_mqttConnected) return false;
  bool ret = _a9g->disconnectBroker();
  if (ret) {
    _mqttConnected = false;
  }
  return ret;
}

int A9Gmod::connectionState() {
  // Return 1 if connected, 0 otherwise
  return _mqttConnected ? 1 : 0;
}

/**
 * @brief Static callback that A9G calls whenever there's a new event. 
 */
void A9Gmod::_onModemEvent(A9G_Event *evt) {
  if (_activeInstance) {
    _activeInstance->_handleModemEvent(evt);
  }
}

/**
 * @brief Actual instance handler for events
 */
void A9Gmod::_handleModemEvent(A9G_Event *evt) {
  // If it's an MQTT publish event, pass it to the user callback
  if (evt->id == EV_MQTTPUBLISH) {
    if (_mqttUserCallback) {
      _mqttUserCallback(evt->topic, evt->message);
    }
  }
  // You could handle other events here (lost connection, etc.)
}
