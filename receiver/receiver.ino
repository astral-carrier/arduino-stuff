/*
  Peripheral Explorer

  This example scans for Bluetooth® Low Energy peripherals until one with a particular name ("LED")
  is found. Then connects, and discovers + prints all the peripheral's attributes.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use it with another board that is compatible with this library and the
  Peripherals -> LED example.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <Arduino_LED_Matrix.h>

const uint32_t empty[] = {
  0,
  0,
  0
};

const uint32_t start[] = {
  0xffffff00,
  0,
  0
};
const uint32_t mid[] = {
  0x000000ff,
  0xffff0000,
  0
};
const uint32_t ready[] = {
  0,
  0x0000ffff,
  0xff000000
};
const uint32_t final[] = {
  0,
  0,
  0x00ffffff
};
const uint32_t full[] = {
  0xffffffff,
  0xffffffff,
  0xffffffff
};

bool second_setup_run = false;
ArduinoLEDMatrix matrix;

byte MIN_SPEED = 0;
byte MAX_SPEED = 255;
int16_t MIN_SETTING = 1;
int16_t MAX_SETTING = 8;

byte in1 = 11;
byte in2 = 12;
byte ena1 = 13;

byte grnLed = 2;
byte redLed = 3;

byte LASER = 10;

int16_t setting = 1;
bool laser_on = 0;

byte EXPECTED_CHARACTERISTIC_COUNT = 8;

uint16_t JOYSTICK_CENTER = 500;
uint16_t JOYSTICK_DEADZONE = 10;

unsigned long last_controller_heartbeat = 0;
unsigned long CONTROLLER_HEARTBEAT_TIMEOUT = 1000;

BLEService indication_service("fff1");
BLEUnsignedShortCharacteristic power("fff2", BLERead | BLENotify);
BLEBoolCharacteristic receiver_heartbeat("fff3", BLENotify);

void advertise() {
  indication_service.addCharacteristic(power);
  indication_service.addCharacteristic(receiver_heartbeat);

  BLE.addService(indication_service);

  // Build scan response data packet
  BLEAdvertisingData scanData;
  // Set parameters for scan response packet
  scanData.setLocalName("Rover Receiver Arduino");
  // Copy set parameters in the actual scan response packet
  BLE.setScanResponseData(scanData);

  /*
  // Build advertising data packet
  BLEAdvertisingData advData;
  // Set parameters for advertising packet
  advData.setManufacturerData(0x004C, manufactData, sizeof(manufactData));
  advData.setAdvertisedService(myService);
  advData.setAdvertisedServiceData(0xfff0, serviceData, sizeof(serviceData));
  // Copy set parameters in the actual advertising packet
  BLE.setAdvertisingData(advData);
  */

  Serial.print("Address is ");
  Serial.println(BLE.address());

  BLE.advertise();
  
  Serial.println("advertising ...");
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena1, OUTPUT);

  pinMode(redLed, OUTPUT);
  pinMode(grnLed, OUTPUT);

  pinMode(LASER, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1) {
      // Serial.println("uwu");
    };
  }

  // advertise();

  matrix.begin();
}

void second_setup() {
  matrix.loadFrame(start);

  // Serial.println("Scan started");
  change_setting(MIN_SETTING);
}

BLEService controller_service = BLEService();

void loop() {
  if (!second_setup_run) {
    second_setup();

    second_setup_run = true;
  }

  if (!controller_service) {
    attempt_connect();
  } else {
    handle_signals();
  }

  // BLE.poll();
}

void handle_signals() {
  if (controller_service.characteristicCount() != EXPECTED_CHARACTERISTIC_COUNT) {
    Serial.print("Polling expected ");
    Serial.print(EXPECTED_CHARACTERISTIC_COUNT);
    Serial.print(" characteristics but got ");
    Serial.println(controller_service.characteristicCount());

    controller_service = BLEService();

    return;
  }

  BLECharacteristic a_pressed = controller_service.characteristic(0);
  BLECharacteristic b_pressed = controller_service.characteristic(1);
  BLECharacteristic c_pressed = controller_service.characteristic(2);
  BLECharacteristic d_pressed = controller_service.characteristic(3);
  BLECharacteristic e_pressed = controller_service.characteristic(4);
  BLECharacteristic joystick_x = controller_service.characteristic(6);
  BLECharacteristic controller_heartbeat = controller_service.characteristic(7);

  if (controller_heartbeat.valueUpdated()) {
    last_controller_heartbeat = millis();
  } else if (millis() - last_controller_heartbeat > CONTROLLER_HEARTBEAT_TIMEOUT) {
    Serial.print("Heartbeat timed out after ");
    Serial.print(CONTROLLER_HEARTBEAT_TIMEOUT);
    Serial.println(" ms");

    controller_service = BLEService();

    return;
  }

  handle_a(a_pressed);
  handle_c(c_pressed);
  handle_d(d_pressed);
  handle_b(b_pressed);
  handle_e(e_pressed);
  handle_joystick(joystick_x);
}

bool read_bool_char(BLECharacteristic characteristic) {
  int32_t read_raw;

  characteristic.readValue(read_raw);

  bool read_value = read_raw & 1;

  digitalWrite(redLed, read_value ? LOW : HIGH);

  return read_value;
}

uint16_t read_short_char(BLECharacteristic characteristic) {
  int32_t read_raw;

  characteristic.readValue(read_raw);

  return read_raw;
}

void change_setting(int16_t attempted_value) {
  setting = constrain(attempted_value, MIN_SETTING, MAX_SETTING);
  byte abs_setting = abs(setting);
  
  // green led on = forward
  // digitalWrite(grnLed, setting > 0 ? HIGH : LOW);
  
  // set brightness of green led to absolute value of power
  analogWrite(grnLed, max((abs_setting << 5) - 1, 0));

  Serial.print("Set setting to ");
  Serial.println(setting);
}

void handle_a(BLECharacteristic pressed) {
  if (pressed.valueUpdated() && !read_bool_char(pressed)) {
    change_setting(setting - 1);
  }
}

void handle_c(BLECharacteristic pressed) {
  if (pressed.valueUpdated() && !read_bool_char(pressed)) {
    change_setting(setting + 1);
  }
}

void handle_d(BLECharacteristic pressed) {
  if (pressed.valueUpdated() && !read_bool_char(pressed)) {
    change_setting(MIN_SETTING);
  }
}

void handle_b(BLECharacteristic pressed) {
  if (pressed.valueUpdated() && !read_bool_char(pressed)) {
    change_setting(MAX_SETTING);
  }
}

void handle_e(BLECharacteristic pressed) {
  if (pressed.valueUpdated() && !read_bool_char(pressed)) {
    laser_on = !laser_on;

    digitalWrite(LASER, laser_on ? HIGH : LOW);
  }
}

void handle_joystick(BLECharacteristic x_char) {
  if (x_char.valueUpdated()) {
    uint16_t x = read_short_char(x_char);

    if (x < JOYSTICK_CENTER - JOYSTICK_DEADZONE) {
      set_motor(setting, false, true);
    } else if (x > JOYSTICK_CENTER + JOYSTICK_DEADZONE) {
      set_motor(setting, true, false);
    } else {
      set_motor(setting, false, false);
    }
  }
}

void set_motor(int16_t set_to, bool backward, bool forward) {
  Serial.print("Seting motor to ");
  Serial.print(set_to);
  Serial.print(" ");
  Serial.println(forward || backward ? (forward ? "forward" : "back") : "stop");

  digitalWrite(in1, forward);
  digitalWrite(in2, backward);

  analogWrite(ena1, (set_to << 5) - 1);
}

void configure_scan() {
  Serial.println("Bluetooth® Low Energy Central - Peripheral Explorer");

  // start scanning for peripherals
  BLE.scanForAddress("f4:12:fa:63:78:65");
}

void attempt_connect() {
  matrix.loadFrame(mid);

  configure_scan();

  Serial.println("scanning");

  BLEDevice controller = BLE.available();

  if (controller) {
    BLE.stopScan();
    matrix.loadFrame(ready);

    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(controller.address());
    Serial.print(" '");
    Serial.print(controller.localName());
    Serial.print("' ");
    Serial.print(controller.advertisedServiceUuid());
    Serial.println();

    matrix.loadFrame(final);

    explorerPeripheral(controller);

    attempt_subscribe(controller);
  }
}

void attempt_subscribe(BLEDevice controller) {
  BLEService service = controller.service("fff0");

  if (!service) {
    Serial.println("failed to find controller service");

    return;
  }

  if (service.characteristicCount() != EXPECTED_CHARACTERISTIC_COUNT) {
    Serial.print("Connection expected ");
    Serial.print(EXPECTED_CHARACTERISTIC_COUNT);
    Serial.print(" characteristics but got ");
    Serial.println(service.characteristicCount());

    return;
  }

  for (int index = 0; index < service.characteristicCount(); index++) {
    BLECharacteristic characteristic = service.characteristic(index);

    if (!characteristic.subscribe()) {
      Serial.print("failed to subscribe to characteristic ");
      Serial.println(index);

      return;
    }
  }

  controller_service = service;
  last_controller_heartbeat = millis();

  Serial.println("Subscribed to all features, controller heartbeat initialized");
  matrix.loadFrame(full);
}

void explorerPeripheral(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // read and print device name of peripheral
  Serial.println();
  Serial.print("Device name: ");
  Serial.println(peripheral.deviceName());
  Serial.print("Appearance: 0x");
  Serial.println(peripheral.appearance(), HEX);
  Serial.println();

  /*
  // loop the services of the peripheral and explore each
  for (int i = 0; i < peripheral.serviceCount(); i++) {
    BLEService service = peripheral.service(i);

    exploreService(service);
  }

  Serial.println();
  */

  /*
  // we are done exploring, disconnect
  Serial.println("Disconnecting ...");
  peripheral.disconnect();
  Serial.println("Disconnected");
  */
}

/*
void exploreService(BLEService service) {
  // print the UUID of the service
  Serial.print("Service ");
  Serial.println(service.uuid());

  // loop the characteristics of the service and explore each
  for (int i = 0; i < service.characteristicCount(); i++) {
    BLECharacteristic characteristic = service.characteristic(i);

    exploreCharacteristic(characteristic);
  }
}


void exploreCharacteristic(BLECharacteristic characteristic) {
  // print the UUID and properties of the characteristic
  Serial.print("\tCharacteristic ");
  Serial.print(characteristic.uuid());
  Serial.print(", properties 0x");
  Serial.print(characteristic.properties(), HEX);

  // check if the characteristic is readable
  if (characteristic.canRead()) {
    // read the characteristic value
    characteristic.read();

    if (characteristic.valueLength() > 0) {
      // print out the value of the characteristic
      Serial.print(", value 0x");
      printData(characteristic.value(), characteristic.valueLength());
    }
  }
  Serial.println();

  // loop the descriptors of the characteristic and explore each
  for (int i = 0; i < characteristic.descriptorCount(); i++) {
    BLEDescriptor descriptor = characteristic.descriptor(i);

    exploreDescriptor(descriptor);
  }
}

void exploreDescriptor(BLEDescriptor descriptor) {
  // print the UUID of the descriptor
  Serial.print("\t\tDescriptor ");
  Serial.print(descriptor.uuid());

  // read the descriptor value
  descriptor.read();

  // print out the value of the descriptor
  Serial.print(", value 0x");
  printData(descriptor.value(), descriptor.valueLength());

  Serial.println();
}

void printData(const unsigned char data[], int length) {
  for (int i = 0; i < length; i++) {
    unsigned char b = data[i];

    if (b < 16) {
      Serial.print("0");
    }

    Serial.print(b, HEX);
  }
}
*/
