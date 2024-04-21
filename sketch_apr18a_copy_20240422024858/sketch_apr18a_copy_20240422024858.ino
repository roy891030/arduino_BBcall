#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // 设置I2C地址和尺寸
File myFile;

const int joyPinX = A0;  // Joystick X轴
const int buttonAPin = 2; // A按钮
const int buttonBPin = 3; // B按钮
const int buttonCPin = 4; // C按钮
const int buttonDPin = 5; // D按钮
const int csSD = 10;      // SD卡CS引脚

String inputString = "";  // 存储输入的字符串
char currentChar = 'A';   // 当前选择的字符
int lastJoyX = 0;         // 上次操纵杆X轴读数
enum State { MENU, CREATE, READ, EDIT };  // 状态枚举
State currentState = MENU;  // 初始状态
int menuIndex = 0;          // 菜单索引
String menuItems[2] = {"Create New File", "Read File"};  // 菜单项

void setup() {
  lcd.init();                      // 初始化LCD
  lcd.backlight();                 // 打开背光
  pinMode(buttonAPin, INPUT_PULLUP);
  pinMode(buttonBPin, INPUT_PULLUP);
  pinMode(buttonCPin, INPUT_PULLUP);
  pinMode(buttonDPin, INPUT_PULLUP);
  Serial.begin(9600);              // 串行通信初始化
  if (!SD.begin(csSD)) {
    lcd.print("SD Fail");
    return;
  }
  lcd.print("SD OK");
  delay(2000);
  lcd.clear();
  updateMenu();  // 显示菜单
}

void loop() {
  int joyX = analogRead(joyPinX);  // 读取操纵杆X轴位置

  if (currentState == MENU) {
    handleMenu(joyX);
  } else if (currentState == EDIT) {
    handleEdit(joyX);
  }
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(inputString);
  lcd.setCursor(0, 1);
  lcd.print(currentChar);
}

void updateMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">");
  lcd.print(menuItems[menuIndex]);
  lcd.setCursor(0, 1);
  lcd.print(menuIndex == 0 ? "  Read File" : "  Create New File");
}

void handleMenu(int joyX) {
  if (joyX < 400 && lastJoyX >= 400) {
    menuIndex = (menuIndex + 1) % 2;
    updateMenu();
  } else if (joyX > 600 && lastJoyX <= 600) {
    menuIndex = (menuIndex - 1 + 2) % 2;
    updateMenu();
  }
  lastJoyX = joyX;

  if (digitalRead(buttonBPin) == LOW) {
    if (menuIndex == 0) {
      currentState = EDIT;
      inputString = "";
      currentChar = 'A';
      lcd.clear();
      updateLCD();
    } else {
      currentState = READ;
      // Implement file reading logic
    }
    delay(200);
  }
}

void handleEdit(int joyX) {
  if (joyX < 400 && lastJoyX >= 400) {
    currentChar = currentChar <= 'A' ? 'Z' : currentChar - 1;
    updateLCD();
  } else if (joyX > 600 && lastJoyX <= 600) {
    currentChar = currentChar >= 'Z' ? 'A' : currentChar + 1;
    updateLCD();
  }
  lastJoyX = joyX;

  if (digitalRead(buttonBPin) == LOW) {
    inputString += ' ';
    updateLCD();
    delay(200);
  }
  if (digitalRead(buttonAPin) == LOW) {
    inputString += currentChar;
    updateLCD();
    delay(200);
  }
  if (digitalRead(buttonDPin) == LOW) {
    if (inputString.length() > 0) {
      inputString.remove(inputString.length() - 1);
      updateLCD();
    }
    delay(200);
  }
  if (digitalRead(buttonCPin) == LOW) {
    saveToFile();
    delay(200);
  }
}

void saveToFile() {
  myFile = SD.open("input.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(inputString);
    myFile.close();
    lcd.clear();
    lcd.print("Saved!");
  } else {
    lcd.print("Error Save");
  }
  delay(2000);
  lcd.clear();
  updateLCD();
}
