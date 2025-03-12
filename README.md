# A9Gmod Library

A lightweight Arduino library for controlling the AiThinker A9G module, which provides GPRS, GPS, and SMS functionality. It offers two core classes:

1. **A9G**  
   Handles low-level AT commands (e.g., sending SMS, GPRS/APN setup, enabling GPS, simple MQTT commands).

2. **A9Gmod**  
   A higher-level interface that simplifies MQTT usage, similar to other Arduino MQTT clients.

---

## Features

- **Initialization & Basic Checks**
  - Ensure module responsiveness (send `AT` command).
  - Check device IMEI, signal quality, and SIM CCID.
  - Wait for the module to report a `READY` status.

- **GPRS & APN Configuration**
  - Attach/detach GPRS connectivity.
  - Set APN parameters.
  - Activate/deactivate PDP context for data usage.

- **GPS**
  - Enable/disable onboard GPS and AGPS.
  - Fetch raw GPS NMEA sentences for parsing.

- **SMS**
  - Send, read, and delete SMS in text mode.
  - Configure SMS storage.

- **MQTT**
  - Connect to MQTT brokers with optional username/password credentials.
  - Publish and subscribe to topics with customizable QoS settings.
  - Register callbacks to receive incoming MQTT messages.

---

## Installation

1. **Place Library**  
   Copy the library files into a folder named `A9Gmod` under your Arduino `libraries` directory.

2. **Include in Sketch**  
   Add an include statement for the library in your Arduino code.

3. **Compile & Upload**  
   Verify that your board and port settings are correct, then compile and upload your sketch.

---

## Basic Usage Flow

1. **Create an `A9G` instance** for sending AT commands to the A9G module.  
2. **Initialize the module** by pointing to a serial interface.  
3. **Set APN & GPRS** if network connectivity is required.  
4. **Use `A9Gmod`** if you want a high-level approach for MQTT.  
5. **Poll the modem** in your main loop to handle events and incoming data.

---

## Classes Overview

### A9G
Manages direct AT command communication, exposing methods to:
- Configure GPRS and APN.
- Enable/disable GPS and read GPS data.
- Send and receive SMS.
- Connect to an MQTT broker at a lower level.

### A9Gmod
Provides a simplified MQTT client interface:
- Define the broker address and port.
- Connect using client ID, username, and password if needed.
- Publish, subscribe, unsubscribe, and disconnect easily.
- Register a callback to handle incoming messages.

---

## Examples

- **SMS Example**  
  Demonstrates how to send and read text messages.
- **GPS Example**  
  Shows how to enable GPS and retrieve NMEA sentences.
- **MQTT Example**  
  Illustrates connecting to a public broker and handling publish/subscribe operations.

---

## Contributing

Contributions are welcome! Feel free to open issues for bug reports or feature requests, and submit pull requests with improvements or examples.

---

## License

Check the License file for in-depth idea of usage.

---

**Author**:  
@MD. Touhiduzzaman Turja (Zaman Turja)  


For questions, suggestions, or any help, please reach out or create an issue in the repository.
