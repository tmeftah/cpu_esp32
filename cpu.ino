#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"

// Initialize Bluetooth Serial
BluetoothSerial SerialBT;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

const int LED = 13; // Initialize LED pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static bool toggleState = false; // Variable to store the toggle state

void setup() {
  pinMode(LED, OUTPUT); // Set LED pin as output

  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  // Rotate display 90 degrees
  display.setRotation(0);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Initialize Bluetooth Serial
  SerialBT.begin("ESP32_BT2"); // Name of your Bluetooth device
  Serial.println("The device started, now you can pair it with Bluetooth!");
}

void loop() {
  handleBluetoothData();
  handleSerialData();
}

void handleBluetoothData() {
  // Process data received via Bluetooth
  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');
    displayData(data);
    Serial.println(data);
  }
}

void handleSerialData() {
  // Process data received via Serial
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    displayData(data);
  }
}

void displayData(String data) {
  digitalWrite(LED, HIGH); // Turn on LED

  int firstSeparator = data.indexOf(" - ");
  int secondSeparator = data.indexOf(" - ", firstSeparator + 1);
  int thirdSeparator = data.indexOf(" - ", secondSeparator + 1);

  String cpuUsageStr = data.substring(0, firstSeparator);
  int cpuUsage = cpuUsageStr.toInt();

  String ipAddress = data.substring(firstSeparator + 3, secondSeparator);
  String networkUsage = data.substring(secondSeparator + 3, thirdSeparator);
  String batteryLevelStr = data.substring(thirdSeparator + 3);
  int batteryLevel = batteryLevelStr.toInt();

  display.clearDisplay();

  // Display CPU usage and visual bar
  display.setCursor(0, 0);
  display.print("CPU: ");
  display.print(cpuUsage);
  display.print("% ");

  int textWidth = 60; // Width for CPU usage text
  int barStartX = textWidth;
  int barWidth = map(cpuUsage, 0, 100, 0, SCREEN_WIDTH - textWidth - 22);

  display.fillRect(barStartX, 0, barWidth, 8, SSD1306_WHITE);

  // Battery symbol
  int batteryBarWidth = map(batteryLevel, 0, 100, 0, 10);
  display.fillRect(SCREEN_WIDTH - 19 + 1, 2, 2, 4, SSD1306_WHITE);
  display.drawRect(SCREEN_WIDTH - 16, 0, 14, 8, SSD1306_WHITE);
  display.fillRect(SCREEN_WIDTH - batteryBarWidth - 4, 2, batteryBarWidth, 4, SSD1306_WHITE);

  // Display IP Address
  display.setCursor(0, 12);
  display.print("IP: ");
  display.println(ipAddress);

  // Display Network Usage
  display.setCursor(0, 24);
  display.print("Net: ");
  display.println(networkUsage);

  // Toggle between states
  display.setCursor(SCREEN_WIDTH - 7, 24);
  if (toggleState) {
    display.print("."); // Print dot
  } else {
    display.print(" "); // Print space
  }
  toggleState = !toggleState;

  display.display(); // Update display
  digitalWrite(LED, LOW); // Turn off LED
}