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
int16_t MIN_SETTING = -8;
int16_t MAX_SETTING = 8;

byte in1 = 11;
byte in2 = 12;
byte ena1 = 13;

int16_t setting = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena1, OUTPUT);

  matrix.begin();
}

void second_setup() {
  matrix.loadFrame(start);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1) {
      // Serial.println("uwu");
    };
  }

  // Serial.println("Scan started");
}

BLEService controller_service;

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
}

void handle_signals() {
  BLECharacteristic a_pressed = controller_service.characteristic(0);
  BLECharacteristic b_pressed = controller_service.characteristic(1);
  BLECharacteristic c_pressed = controller_service.characteristic(2);
  BLECharacteristic d_pressed = controller_service.characteristic(3);

  handle_a(a_pressed);
  handle_c(c_pressed);
  handle_d(d_pressed);
  handle_b(b_pressed);
}

bool read_bool_char(BLECharacteristic characteristic) {
  int32_t read_raw;

  characteristic.readValue(read_raw);

  bool read_value = read_raw & 1;

  matrix.loadFrame(read_value ? empty : full);

  return read_value;
}

void change_setting(int16_t attempted_value) {
  setting = constrain(attempted_value, MIN_SETTING, MAX_SETTING);

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
    change_setting(0);
  }
}

void handle_b(BLECharacteristic pressed) {
  if (pressed.valueUpdated()) {
    set_motor(setting * !read_bool_char(pressed));
  }
}

void set_motor(int16_t set_to) {
  Serial.print("Seting motor to ");
  Serial.println(set_to);

  byte abs_setting = abs(set_to);

  digitalWrite(in1, set_to > 0);
  digitalWrite(in2, set_to < 0);

  if (set_to == 0) {
    return;
  }

  analogWrite(ena1, (abs_setting << 5) - 1);
}

void configure_scan() {
  Serial.println("Bluetooth® Low Energy Central - Peripheral Explorer");

  // start scanning for peripherals
  BLE.scanForAddress("f4:12:fa:63:78:65");
}

void attempt_connect() {
  matrix.loadFrame(mid);

  configure_scan();

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
  controller_service = controller.service("fff0");

  if (!controller_service) {
    Serial.println("failed to find controller service");

    return;
  }

  for (int index = 0; index < controller_service.characteristicCount(); index++) {
    BLECharacteristic characteristic = controller_service.characteristic(index);

    if (!characteristic.subscribe()) {
      Serial.print("failed to subscribe to characteristic ");
      Serial.println(index);

      return;
    }
  }

  Serial.println("Subscribed to all features");

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

  // loop the services of the peripheral and explore each
  for (int i = 0; i < peripheral.serviceCount(); i++) {
    BLEService service = peripheral.service(i);

    exploreService(service);
  }

  Serial.println();

  /*
  // we are done exploring, disconnect
  Serial.println("Disconnecting ...");
  peripheral.disconnect();
  Serial.println("Disconnected");
  */
}

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
