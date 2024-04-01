#include <ArduinoBLE.h>
#include <Arduino_LED_Matrix.h>

BLEService controller_service("fff0");
BLEBoolCharacteristic a_pressed("fff1", BLERead | BLENotify);
BLEBoolCharacteristic b_pressed("fff2", BLERead | BLENotify);
BLEBoolCharacteristic c_pressed("fff3", BLERead | BLENotify);
BLEBoolCharacteristic d_pressed("fff4", BLERead | BLENotify);

// Advertising parameters should have a global scope. Do NOT define them in 'setup' or in 'loop'
const uint8_t manufactData[4] = {0x01, 0x02, 0x03, 0x04};
const uint8_t serviceData[3] = {0x00, 0x01, 0x02};

const uint32_t full[] = {
  0xffffffff,
  0xffffffff,
  0xffffffff
};
ArduinoLEDMatrix matrix;
bool second_setup_run = false;

void setup() {
  matrix.begin();
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("failed to initialize BLE!");
    while (1);
  }

  controller_service.addCharacteristic(a_pressed);
  controller_service.addCharacteristic(b_pressed);
  controller_service.addCharacteristic(c_pressed);
  controller_service.addCharacteristic(d_pressed);

  BLE.addService(controller_service);

  // Build scan response data packet
  BLEAdvertisingData scanData;
  // Set parameters for scan response packet
  scanData.setLocalName("Arduino R4 Wifi Peripheral");
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

  BLE.advertise();
  Serial.println("advertising ...");
}

void second_setup() {
  matrix.loadFrame(full);
}

void loop() {
  if (!second_setup_run) {
    second_setup();

    second_setup_run = true;
  }

  BLE.poll();
}
