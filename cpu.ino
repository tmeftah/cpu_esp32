#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"

// Initialize Bluetooth Serial for communication
BluetoothSerial SerialBT;

// Define display dimensions
#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 32   // OLED display height, in pixels

const int LED = 13; // Pin for the onboard LED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variable to track toggle state for visual indicator
static bool toggleState = false;

void setup() {
  // Set the LED pin as an output
  pinMode(LED, OUTPUT);

  // Start serial communication for debugging purposes
  Serial.begin(115200);

  // Initialize the OLED display with I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;); // Enter infinite loop if display initialization fails
  }

  // Set display orientation
  display.setRotation(0);

  // Clear the display buffer
  display.clearDisplay();
  
  // Set text size and color for display
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Initialize Bluetooth Serial with name "ESP32_BT2"
  SerialBT.begin("ESP32_BT2");
  Serial.println("The device started, now you can pair it with Bluetooth!");
}

void loop() {
  handleBluetoothData(); // Handle incoming Bluetooth data
  handleSerialData();    // Handle incoming Serial data
}

void handleBluetoothData() {
  // Check if data is available on Bluetooth
  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n'); // Read data until newline
    displayData(data);                            // Display the received data
    Serial.println(data);                         // Print data to Serial monitor
  }
}

void handleSerialData() {
  // Check if data is available on Serial
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Read data until newline
    displayData(data);                          // Display the received data
  }
}

void displayData(String data) {
  digitalWrite(LED, HIGH); // Turn on LED to indicate data processing

  // Parse the data using " - " as the separator
  int firstSeparator = data.indexOf(" - ");
  int secondSeparator = data.indexOf(" - ", firstSeparator + 1);
  int thirdSeparator = data.indexOf(" - ", secondSeparator + 1);

  // Extract and convert CPU usage
  String cpuUsageStr = data.substring(0, firstSeparator);
  int cpuUsage = cpuUsageStr.toInt();

  // Extract IP address
  String ipAddress = data.substring(firstSeparator + 3, secondSeparator);

  // Extract network usage
  String networkUsage = data.substring(secondSeparator + 3, thirdSeparator);

  // Extract and convert battery level
  String batteryLevelStr = data.substring(thirdSeparator + 3);
  int batteryLevel = batteryLevelStr.toInt();

  // Clear the display buffer
  display.clearDisplay();

  // Display CPU usage and draw a visual bar
  display.setCursor(0, 0);
  display.print("CPU: ");
  display.print(cpuUsage);
  display.print("% ");
  
  int textWidth = 60; // Space allocated for CPU usage text
  int barStartX = textWidth;
  int barWidth = map(cpuUsage, 0, 100, 0, SCREEN_WIDTH - textWidth - 22);
  display.fillRect(barStartX, 0, barWidth, 8, SSD1306_WHITE);

  // Draw battery symbol with visual indication of battery level
  int batteryBarWidth = map(batteryLevel, 0, 100, 0, 10);
  display.fillRect(SCREEN_WIDTH - 19 + 1, 2, 2, 4, SSD1306_WHITE);
  display.drawRect(SCREEN_WIDTH - 16, 0, 14, 8, SSD1306_WHITE);
  display.fillRect(SCREEN_WIDTH - batteryBarWidth - 4, 2, batteryBarWidth, 4, SSD1306_WHITE);

  // Display IP address
  display.setCursor(0, 12);
  display.print("IP: ");
  display.println(ipAddress);

  // Display network usage
  display.setCursor(0, 24);
  display.print("Net: ");
  display.println(networkUsage);

  // Toggle a visual indicator between a dot and a space
  display.setCursor(SCREEN_WIDTH - 7, 24);
  if (toggleState) {
    display.print("."); // Print a dot
  } else {
    display.print(" "); // Print a space
  }
  toggleState = !toggleState; // Toggle state for next cycle

  display.display(); // Send display buffer to the screen
  digitalWrite(LED, LOW); // Turn off LED after processing
}