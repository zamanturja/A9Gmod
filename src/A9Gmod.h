#ifndef A9GMOD_H
#define A9GMOD_H

#include <Arduino.h>
#include <Stream.h>

/*!
 * @file A9Gmod.h
 *
 * @brief This header file unifies:
 *        1) The A9G class that handles all AT commands (SMS, GPRS, MQTT, GPS, etc.).
 *        2) The A9Gmod class that acts like a high-level MQTT client (similar to PubSubClient).
 * @author
 * Written by @MD. Touhiduzzaman Turja [AKA - Zaman Turja]
 * V0.1.25.3.3
 */

/* ------------------------------------------------------------------
 *                      A9G EVENT STRUCTS & ENUMS
 * ------------------------------------------------------------------ */

/**
 * @brief Event ID used to determine what kind of data or notification has arrived
 */
typedef enum A9G_EventID {
  EV_CREG = 0,
  EV_CTZV,
  EV_CIEV,
  EV_CPMS,
  EV_CMT,
  EV_CMTI,
  EV_CMGL,
  EV_NEW_SMS_RECEIVED,
  EV_GPSRD,
  EV_CGATT,
  EV_AGPS,
  EV_GPNT,
  EV_MQTTPUBLISH,
  EV_CMGS,
  EV_CME,
  EV_CMS,
  EV_CSQ,
  EV_IMEI,
  EV_CCID,
  EV_MAX,
  EV_NONE
} A9G_EventID;

/**
 * @brief Common CME (modem) error codes
 */
typedef enum A9G_CME_Error {
  PHONE_FAILURE,
  NO_CONNECT_PHONE,
  PHONE_ADAPTER_LINK_RESERVED,
  OPERATION_NOT_ALLOWED,
  OPERATION_NOT_SUPPORTED,
  PHSIM_PIN_REQUIRED,
  PHFSIM_PIN_REQUIRED,
  PHFSIM_PUK_REQUIRED,
  SIM_NOT_INSERTED = 10,
  SIM_PIN_REQUIRED,
  SIM_PUK_REQUIRED,
  SIM_FAILURE,
  SIM_BUSY,
  SIM_WRONG,
  INCORRECT_PASSWORD,
  SIM_PIN2_REQUIRED,
  SIM_PUK2_REQUIRED,
  MEMORY_FULL = 20,
  INVALID_INDEX,
  NOT_FOUND,
  MEMORY_FAILURE,
  TEXT_LONG,
  INVALID_CHAR_INTEXT,
  DAIL_STR_LONG,
  INVALID_CHAR_INDIAL,
  NO_NET_SERVICE = 30,
  NETWORK_TIMOUT,
  NOT_ALLOW_EMERGENCY,
  NET_PER_PIN_REQUIRED = 40,
  NET_PER_PUK_REQUIRED,
  NET_SUB_PER_PIN_REQ,
  NET_SUB_PER_PUK_REQ,
  SERVICE_PROV_PER_PIN_REQ,
  SERVICE_PROV_PER_PUK_REQ,
  CORPORATE_PER_PIN_REQ,
  CORPORATE_PER_PUK_REQ,
  PHSIM_PBK_REQUIRED,
  EXE_NOT_SURPORT,
  EXE_FAIL,
  NO_MEMORY,
  OPTION_NOT_SURPORT,
  PARAM_INVALID,
  EXT_REG_NOT_EXIT,
  EXT_SMS_NOT_EXIT,
  EXT_PBK_NOT_EXIT,
  EXT_FFS_NOT_EXIT,
  INVALID_COMMAND_LINE,
  GPRS_ILLEGAL_MS_3 = 103,
  GPRS_ILLEGAL_MS_6 = 106,
  GPRS_SVR_NOT_ALLOWED,
  GPRS_PLMN_NOT_ALLOWED = 111,
  GPRS_LOCATION_AREA_NOT_ALLOWED,
  GPRS_ROAMING_NOT_ALLOWED,
  GPRS_OPTION_NOT_SUPPORTED = 132,
  GPRS_OPTION_NOT_SUBSCRIBED,
  GPRS_OPTION_TEMP_ORDER_OUT,
  GPRS_PDP_AUTHENTICATION_FAILURE = 149,
  GPRS_INVALID_MOBILE_CLASS = 150,
  GPRS_UNSPECIFIED_GPRS_ERROR = 148,
  SIM_VERIFY_FAIL = 264,
  SIM_UNBLOCK_FAIL,
  SIM_CONDITION_NO_FULLFILLED,
  SIM_UNBLOCK_FAIL_NO_LEFT,
  SIM_VERIFY_FAIL_NO_LEFT,
  SIM_INVALID_PARAMETER,
  SIM_UNKNOW_COMMAND,
  SIM_WRONG_CLASS,
  SIM_TECHNICAL_PROBLEM,
  SIM_CHV_NEED_UNBLOCK,
  SIM_NOEF_SELECTED,
  SIM_FILE_UNMATCH_COMMAND,
  SIM_CONTRADICTION_CHV,
  SIM_CONTRADICTION_INVALIDATION,
  SIM_MAXVALUE_REACHED,
  SIM_PATTERN_NOT_FOUND,
  SIM_FILEID_NOT_FOUND,
  SIM_STK_BUSY,
  SIM_UNKNOW,
  SIM_PROFILE_ERROR,
  CME_ERROR_MAX
} A9G_CME_Error;

/**
 * @brief Common CMS (SMS) error codes
 */
typedef enum A9G_CMS_Error {
  UNASSIGNED_NUM = 1,
  OPER_DETERM_BARR = 8,
  CALL_BARRED = 10,
  SM_TRANS_REJE = 21,
  DEST_OOS = 27,
  UNINDENT_SUB,
  FACILIT_REJE,
  UNKONWN_SUB,
  NW_OOO = 38,
  TMEP_FAIL = 41,
  CONGESTION,
  RES_UNAVAILABLE = 47,
  REQ_FAC_NOT_SUB = 50,
  RFQ_FAC_NOT_IMP = 69,
  INVALID_SM_TRV = 81,
  INVALID_MSG = 95,
  INVALID_MAND_INFO,
  MSG_TYPE_ERROR,
  MSG_NOT_COMP,
  INFO_ELEMENT_ERROR,
  PROT_ERROR = 111,
  IW_UNSPEC = 127,
  TEL_IW_NOT_SUPP,
  SMS_TYPE0_NOT_SUPP,
  CANNOT_REP_SMS,
  UNSPEC_TP_ERROR = 143,
  DCS_NOT_SUPP,
  MSG_CLASS_NOT_SUPP,
  UNSPEC_TD_ERROR = 159,
  CMD_CANNOT_ACT,
  CMD_UNSUPP,
  UNSPEC_TC_ERROR = 175,
  TPDU_NOT_SUPP,
  SC_BUSY = 192,
  NO_SC_SUB,
  SC_SYS_FAIL,
  INVALID_SME_ADDR,
  DEST_SME_BARR,
  SM_RD_SM,
  TP_VPF_NOT_SUPP,
  TP_VP_NOT_SUPP,
  D0_SIM_SMS_STO_FULL = 208,
  NO_SMS_STO_IN_SIM,
  ERR_IN_MS,
  MEM_CAP_EXCCEEDED,
  SIM_APP_TK_BUSY,
  SIM_DATA_DL_ERROR,
  UNSPEC_ERRO_CAUSE = 255,
  ME_FAIL = 300,
  SMS_SERVIEC_RESERVED,
  OPER_NOT_ALLOWED,
  OPER_NOT_SUPP,
  INVALID_PDU_PARAM,
  INVALID_TXT_PARAM,
  SIM_NOT_INSERT = 310,
  CMS_SIM_PIN_REQUIRED,
  PH_SIM_PIN_REQUIRED,
  SIM_FAIL,
  CMS_SIM_BUSY,
  CMS_SIM_WRONG,
  CMS_SIM_PUK_REQUIRED,
  CMS_SIM_PIN2_REQUIRED,
  CMS_SIM_PUK2_REQUIRED,
  MEM_FAIL = 320,
  INVALID_MEM_INDEX,
  MEM_FULL,
  SCA_ADDR_UNKNOWN = 330,
  NO_NW_SERVICE,
  NW_TIMEOUT,
  NO_CNMA_ACK_EXPECTED = 340,
  UNKNOWN_ERROR = 500,
  USER_ABORT = 512,
  UNABLE_TO_STORE,
  INVALID_STATUS,
  INVALID_ADDR_CHAR,
  INVALID_LEN,
  INVALID_PDU_CHAR,
  INVALID_PARA,
  INVALID_LEN_OR_CHAR,
  INVALID_TXT_CHAR,
  TIMER_EXPIRED = 521,
  CMS_ERROR_NONE,
  CMS_ERROR_MAX
} A9G_CMS_Error;

/**
 * @brief Message storage type used for deleting messages
 */
typedef enum A9G_MessageType {
  READ_MESSAGE = 1,
  UNREAD_MESSAGE = 2,
  ALL_MESSAGE = 4
} A9G_MessageType;

/**
 * @brief Core event structure with data from the modem
 */
typedef struct A9G_Event {
  A9G_EventID id;      ///< The event ID/type
  int error;           ///< Error code if any
  char message[100];   ///< General message buffer
  char topic[50];      ///< MQTT topic buffer
  char number[16];     ///< Phone number (for SMS)
  char date_time[25];  ///< Date/time info (for SMS)
  int param1;          ///< For numeric parameters
  char param2[30];     ///< For additional textual data
  char param3[50];     ///< Extension if needed
} A9G_Event;


/* ------------------------------------------------------------------
 *                   A9G CLASS (AT COMMAND HANDLER)
 * ------------------------------------------------------------------ */

/**
 * @class A9G
 * @brief Manages the low-level AT commands for the AiThinker A9G module:
 *        - Initialization & readiness checks
 *        - SMS reading/deletion
 *        - GPRS attach, APN setup, etc.
 *        - MQTT connect/publish/subscribe
 *        - GPS enabling and data retrieval
 */
class A9G {
public:
  /**
     * @brief Constructor
     * @param debugMode If set true, prints debug logs to Serial
     */
  A9G(bool debugMode = false);

  /**
     * @brief Initializes the A9G module, sends an "AT" test command,
     *        and returns true if successful.
     * @param serial Pointer to the serial (HardwareSerial / SoftwareSerial / etc.)
     * @return true if modem responded correctly, false otherwise
     */
  bool init(Stream *serial);

  /**
     * @brief Process any available data from the A9G module and dispatch events.
     *        Call frequently inside your main loop().
     */
  void pollModem();

  /**
      * @brief Allows external access to the modem stream i.e- [available(), read(), print(), println()]
      */
  Stream* getModemStream() { return _modemStream; }

  /**
     * @brief Simple method for sending a custom AT command for debugging
     * @param data The AT command string to send
     *
     * This runs in a blocking mode that prints data forever (for test only).
     */
  void testDebugCommand(char *data);

  /* ----------------------------------------------------
     *         BASIC DEVICE & NETWORK INFORMATION
     * ---------------------------------------------------- */
  /**
     * @brief Reads the IMEI of the device (will trigger an event with param2 set).
     */
  void readIMEI();

  /**
     * @brief Reads signal quality (CSQ). Will trigger an event with param1 set.
     */
  void readSignalQuality();

  /**
     * @brief Reads the CCID (SIM chip ID). Will trigger an event with param2 set.
     */
  void readCCID();

  /**
     * @brief Waits for device "READY" message (blocking).
     * @return true if modem eventually reports "READY", false if timed out
     */
  bool waitForModemReady();


  /* ----------------------------------------------------
     *         GPRS & APN HANDLING
     * ---------------------------------------------------- */
  bool isGPRSAttached();
  bool attachGPRS(const char* apn, const char* user = nullptr, const char* pwd = nullptr);
  bool detachGPRS();
  bool setAPN(const char *pdpType, const char *apn);
  bool activatePDP();
  bool deactivatePDP();


  /* ----------------------------------------------------
     *         GPS HANDLING
     * ---------------------------------------------------- */
  /**
     * @brief Enables GPS hardware (AT+GPS=1).
     * @return true on success
     */
  bool enableGPS();

  /**
     * @brief Disables GPS hardware (AT+GPS=0).
     * @return true on success
     */
  bool disableGPS();

  /**
     * @brief Enables AGPS (AT+AGPS=1).
     * @return true on success
     */
  bool enableAGPS();

  /**
     * @brief Retrieves raw NMEA GPS data as a String by sending AT+GPSRD=1.
     *        This method just returns the lines captured in a short window.
     *        You can parse them further as needed.
     * @return String containing NMEA sentences
     */
  String getGPS();


  /* ----------------------------------------------------
     *         MQTT AT COMMANDS
     * ---------------------------------------------------- */
  bool connectBroker(const char *broker, int port,
                     const char *user, const char *pass,
                     const char *clientID, uint8_t keepAlive,
                     uint16_t cleanSession);

  bool connectBroker(const char *broker, int port,
                     const char *clientID,
                     uint8_t keepAlive, uint16_t cleanSession);

  bool connectBroker(const char *broker, int port);
  bool disconnectBroker();
  bool subscribeTopic(const char *topic, uint8_t qos, unsigned long timeout);
  bool subscribeTopic(const char *topic);
  bool unsubscribeTopic(const char *topic);
  bool publishTopic(const char *topic, const char *msg);


  /* ----------------------------------------------------
     *         SMS HANDLING
     * ---------------------------------------------------- */
  bool activateTextMode();
  bool setSMSFormatReading(bool mode);
  bool setMessageStorage();
  void checkMessageStorage();
  void readSMS(uint8_t index);
  void deleteSMS(uint8_t index, A9G_MessageType type);

  /**
     * @brief Send an SMS in a blocking manner
     * @param number Phone number to send to
     * @param message The message body
     * @return true if the command was accepted (not a guarantee of final success)
     */
  bool sendSMS(const char *number, const char *message);

  /**
     * @brief Send an SMS in a non-blocking manner (for advanced usage).
     * @param number Phone number
     * @param message The message body
     */
  void sendSMSNonBlocking(const char *number, const char *message);

  /**
     * @brief Optional helper to delete all messages
     */
  bool deleteAllSMS() {
    // Not implemented in original
    return false;
  }

private:
  /* --------------------------------------
     *    INTERNAL FIELDS
     * -------------------------------------- */
  Stream *_modemStream;          ///< The serial interface to A9G
  bool _debugMode;               ///< Debug logs on/off
  unsigned long _defaultWaitMS;  ///< Default wait-time for responses
  bool _hasSMS;                  ///< Simple state flag for SMS
  int _smsIndex;                 ///< Tracks SMS index for reading

  /**
     * @brief Function pointer for external event callback
     */
  typedef void (*A9G_EventCallback)(A9G_Event *evt);
  A9G_EventCallback _onEventCallback;

  /* --------------------------------------
     *    INTERNAL PARSING & HELPERS
     * -------------------------------------- */
  bool _waitForOkResponse(int timeout);
  void _handlePotentialEvent(A9G_Event *evt, const char *data, int len);
  uint8_t _identifyTermString(const char *termStr);
  void _processEventsIfAny(A9G_Event *evt);
  void _dispatchEvent(A9G_Event *evt);

  /**
     * @brief Called by pollModem() to keep reading from the serial port
     *        and detect +TERM or +something lines.
     */
  void _internalModemParser();

  /* --------------------------------------
     *    ERROR PRINT METHODS
     * -------------------------------------- */
public:
  void printCMEError(int err);
  void printCMSError(int err);

  /**
     * @brief Allow external registration of an event callback
     * @param cb The function pointer that receives A9G_Event pointers
     */
  void setEventCallback(void (*cb)(A9G_Event *));
};


/* ------------------------------------------------------------------
 *            A9Gmod CLASS (HIGH-LEVEL MQTT INTERFACE)
 * ------------------------------------------------------------------ */

/**
 * @brief Callback for incoming MQTT messages
 */
typedef void (*A9G_MQTTCallback)(const char *topic, const char *payload);

/**
 * @class A9Gmod
 * @brief A high-level MQTT client wrapper that uses A9G to send AT commands.
 *
 * Similar to the original PubSubClient style: setServer(), connect(), publish(), subscribe(), etc.
 */
class A9Gmod {
public:
  /**
     * @brief Construct a new A9Gmod object
     * @param a9gRef Reference to an already-initialized A9G object
     */
  A9Gmod(A9G &a9gRef);

  /**
     * @brief Provide the MQTT broker info (host and port).
     */
  void setMQTTServer(const char *host, uint16_t port);

  /**
     * @brief Provide a callback for incoming MQTT publish messages.
     */
  void onMQTTMessage(A9G_MQTTCallback callback);

  /**
     * @brief Connect with just a client ID. KeepAlive=60, CleanSession=1 by default.
     */
  bool connectMQTT(const char *clientID);

  /**
     * @brief Connect with user/pass credentials.
     */
  bool connectMQTT(const char *clientID,
                   const char *user,
                   const char *pass,
                   uint8_t keepAlive = 60,
                   uint16_t cleanSession = 1);

  /**
     * @brief Is the current MQTT connection active?
     */
  bool isMQTTConnected();

  /**
     * @brief Must be called regularly to process inbound events
     *        (MQTT PUBLISH data, etc.).
     */
  void processMQTT();

  /**
     * @brief Publish a message to the given topic.
     */
  bool publishMQTT(const char *topic, const char *payload);

  /**
     * @brief Subscribe to a topic.
     */
  bool subscribeMQTT(const char *topic);
  bool subscribeMQTT(const char *topic, uint8_t qos, unsigned long timeout);

  /**
     * @brief Unsubscribe from a topic.
     */
  bool unsubscribeMQTT(const char *topic);

  /**
     * @brief Disconnect from the MQTT broker.
     */
  bool disconnectMQTT();

  /**
     * @brief Return numeric state: 1 if connected, 0 otherwise.
     */
  int connectionState();

private:
  // The underlying A9G module reference
  A9G *_a9g;
  bool _mqttConnected;
  String _mqttBroker;
  uint16_t _mqttPort;
  A9G_MQTTCallback _mqttUserCallback;

  /**
     * @brief Static pointer for hooking into A9G's event system
     */
  static A9Gmod *_activeInstance;

  /**
     * @brief A9G's static callback calls this method
     */
  static void _onModemEvent(A9G_Event *evt);

  /**
     * @brief Internal event handler for MQTT-related events
     */
  void _handleModemEvent(A9G_Event *evt);
};

#endif  // A9GMOD_H
