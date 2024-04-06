#include <ArduinoBLE.h>
#include <Arduino_LED_Matrix.h>

int const UP_BTN = 2;
int const DOWN_BTN = 4;
int const LEFT_BTN = 5;
int const RIGHT_BTN = 3;
int const E_BTN = 6;
int const F_BTN = 7;
int const JOYSTICK_BTN = 8;
int const JOYSTICK_AXIS_X = A0;
int const JOYSTICK_AXIS_Y = A1;
int buttons[] = {UP_BTN, DOWN_BTN, LEFT_BTN, RIGHT_BTN, E_BTN, F_BTN, JOYSTICK_BTN};

BLEService controller_service("fff0");
BLEBoolCharacteristic a_pressed("fff1", BLERead | BLENotify);
BLEBoolCharacteristic b_pressed("fff2", BLERead | BLENotify);
BLEBoolCharacteristic c_pressed("fff3", BLERead | BLENotify);
BLEBoolCharacteristic d_pressed("fff4", BLERead | BLENotify);
BLEBoolCharacteristic e_pressed("fff5", BLERead | BLENotify);
BLEBoolCharacteristic f_pressed("fff6", BLERead | BLENotify);
BLEUnsignedShortCharacteristic joystick_x("fff7", BLERead | BLENotify);

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

void advertise() {
  controller_service.addCharacteristic(a_pressed);
  controller_service.addCharacteristic(b_pressed);
  controller_service.addCharacteristic(c_pressed);
  controller_service.addCharacteristic(d_pressed);
  controller_service.addCharacteristic(e_pressed);
  controller_service.addCharacteristic(f_pressed);
  controller_service.addCharacteristic(joystick_x);

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

void setup() {
  //Set all button pins as inputs with internal pullup resistors enabled.
  for (int i; i < 7; i++)  pinMode(buttons[i], INPUT_PULLUP);

  matrix.begin();
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("failed to initialize BLE!");
    while (1);
  }

  advertise();
}

void second_setup() {
  matrix.loadFrame(full);
}

void loop() {
  if (!second_setup_run) {
    second_setup();

    second_setup_run = true;
  }

  handle_controller_vals();

  BLE.poll();
}

void handle_controller_vals() {
  /*
  // Check each button input and print the status to the Serial Monitor Window
  Serial.print("UP="),Serial.print(digitalRead(UP_BTN));
  Serial.print("\tDOWN="),Serial.print(digitalRead(DOWN_BTN));
  Serial.print("\tLEFT="),Serial.print(digitalRead(LEFT_BTN));
  Serial.print("\tRIGHT="),Serial.print(digitalRead(RIGHT_BTN));
  Serial.print("\tE="),Serial.print(digitalRead(E_BTN));
  Serial.print("\tF="),Serial.print(digitalRead(F_BTN));
  Serial.print("\tJOYSTICK BTN="),Serial.print(digitalRead(JOYSTICK_BTN));
      
  // Print the full X/Y joystick values (0-1023)
  Serial.print("\tX="),Serial.print(analogRead(JOYSTICK_AXIS_X));
  Serial.print("\tY="),Serial.println(analogRead(JOYSTICK_AXIS_Y)); 
  */

  write_and_notify_bool(a_pressed, digitalRead(UP_BTN));
  write_and_notify_bool(b_pressed, digitalRead(RIGHT_BTN));
  write_and_notify_bool(c_pressed, digitalRead(DOWN_BTN));
  write_and_notify_bool(d_pressed, digitalRead(LEFT_BTN));
  write_and_notify_bool(e_pressed, digitalRead(E_BTN));
  write_and_notify_bool(f_pressed, digitalRead(F_BTN));
  write_and_notify_short(joystick_x, analogRead(JOYSTICK_AXIS_X), 5);

  // delay(250);
}

void write_and_notify_bool(BLEBoolCharacteristic characteristic, bool value) {
  int32_t read_raw;

  characteristic.readValue(read_raw);

  bool read_value = read_raw & 1;

  if (value != read_value) {
    characteristic.writeValue(value);
    
    Serial.print("Wrote and notified characteristic ");
    Serial.print(characteristic.uuid());
    Serial.print(". New value: ");
    Serial.println(value);
  }
}

void write_and_notify_short(BLEUnsignedShortCharacteristic characteristic, uint16_t value, uint16_t threshold) {
  int32_t read_raw;

  characteristic.readValue(read_raw);

  uint16_t read_value = read_raw;

  if (abs(value - read_value) >= threshold) {
    characteristic.writeValue(value);
    
    Serial.print("Wrote and notified characteristic ");
    Serial.print(characteristic.uuid());
    Serial.print(". New value: ");
    Serial.println(value);
  }
}
