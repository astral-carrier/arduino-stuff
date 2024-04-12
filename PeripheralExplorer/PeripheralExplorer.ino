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

void setup() {
  Serial.begin(9600);
  while (!Serial);

  matrix.begin();
}

void second_setup() {
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1) {
      // Serial.println("uwu");
    };
  }

  matrix.loadFrame(start);

  Serial.println("Bluetooth® Low Energy Central - Peripheral Explorer");

  // start scanning for peripherals
  // BLE.scanForAddress("28:ea:0b:e6:62:da");
  BLE.scan();

  matrix.loadFrame(mid);

  // Serial.println("Scan started");
}

void loop() {
  if (!second_setup_run) {
    second_setup();

    second_setup_run = true;
  }

  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // see if peripheral is a LED
    if (true) {
      // stop scanning
      BLE.stopScan();

      peripheral.connect();

      matrix.loadFrame(ready);
      Serial.println("Connected");

      test(peripheral);

      // we are done exploring, disconnect
      Serial.println("Disconnecting ...");
      peripheral.disconnect();
      Serial.println("Disconnected");

      // peripheral disconnected, we are done
      while (1) {
      }
    }
  }
}

void test(BLEDevice device) {
  // discover peripheral attributes
  Serial.println("Discovering attributes ...");

  BLEService hid_char = device.service("1812");

  matrix.loadFrame(final);

  if (hid_char) {
    Serial.println("hid characteristic FOUND");
  } else {
    Serial.println("hid characteristic NOT found");
  }

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
