#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // 設置I2C地址和尺寸
File myFile;
File root;

const int joyPinX = A0;  // Joystick X軸
const int joyPinY = A1;  // Joystick Y軸, 確保這行已經添加
const int buttonAPin = 2; // A按鈕
const int buttonBPin = 3; // B按鈕
const int buttonCPin = 4; // C按鈕
const int buttonDPin = 5; // D按鈕
const int csSD = 10;      // SD卡CS引腳

String inputString = "";  // 存儲輸入的字符串
char currentChar = 'A';   // 當前選擇的字符
int lastJoyX = 0;         // 上次操縱桿X軸讀數
enum State { MENU, CREATE_NAME, EDIT, READ, READ_SELECT };
int lastJoyY = 0;
int fileIndex = 0;
String fileName = "";
State currentState = MENU;
int menuIndex = 0;
String menuItems[2] = {"Create New File", "Read File"};
int selectedIndex = 0;  // 添加這行來聲明selectedIndex
String fileList[10];    // 假設最多存儲10個文件名，確保這行已經添加
void setup() {
  lcd.init();                      // 初始化LCD
  lcd.backlight();                 // 打開背光
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
  updateMenu();  // 顯示菜單
}

void loop() {
  int joyX = analogRead(joyPinX);  // 讀取操縱桿X軸位置
  int joyY = analogRead(joyPinY);

  switch (currentState) {
    case MENU:
      handleMenu(joyX);
      break;
    case CREATE_NAME:
      handleCreateName(joyX);
      break;
    case EDIT:
      handleEdit(joyX);
      break;
    case READ:
      handleRead();
      break;
    case READ_SELECT:
    //  handleReadSelect(joyY);
      break;
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
      currentState = CREATE_NAME;
      inputString = "";
      currentChar = 'A';
      lcd.clear();
      lcd.print("Enter filename:");
    } else {
      currentState = READ;
      listFiles();
    }
    delay(200);
  }
}


void handleCreateName(int joyX) {
    if (joyX < 400 && lastJoyX >= 400) {
        currentChar = currentChar <= 'A' ? 'Z' : currentChar - 1;
        updateLCD(); // 更新LCD顯示當前字符
    } else if (joyX > 600 && lastJoyX <= 600) {
        currentChar = currentChar >= 'Z' ? 'A' : currentChar + 1;
        updateLCD(); // 更新LCD顯示當前字符
    }
    lastJoyX = joyX;

    if (digitalRead(buttonAPin) == LOW) {
        inputString += currentChar; // 添加字符到文件名
        updateLCD(); // 刷新顯示
        delay(200); // 防止按鍵彈跳
    }

    if (digitalRead(buttonBPin) == LOW) {
        if (inputString.length() > 0) { // 確保文件名不為空
            fileName = inputString + ".txt"; // 組成完整文件名
            inputString = ""; // 清空輸入字符串以用於文件內容
            currentState = EDIT; // 切換到編輯狀態
            lcd.clear();
            lcd.print("Editing: " + fileName);
            delay(200); // 添加延遲以穩定狀態轉換
        } else {
            lcd.setCursor(0, 1); // 如果文件名為空，提示用戶
            lcd.print("Name Empty!");
            delay(2000); // 顯示錯誤信息一段時間
            lcd.clear();
            lcd.print("Enter filename:");
        }
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


void handleRead() {
  lcd.clear();
  if (fileIndex == 0) {
    lcd.print("No file");
    delay(2000);
    currentState = MENU;
    updateMenu();
  } else {
    currentState = READ_SELECT;
    selectedIndex = 0;  // 
    listFiles();  // 假設這個函式正確填充了 fileList 和更新了 fileIndex
    displayFileList();  // 顯示文件列表
  }
}
void displayFileList() {
    lcd.clear();  // 清空LCD顯示
    int displayCount = min(fileIndex, 2);  // 假設LCD只能顯示兩行

    for (int i = 0; i < displayCount; i++) {
        lcd.setCursor(0, i);  // 設置光標位置
        lcd.print(fileList[i]);  // 顯示文件名
    }
}

void handleReadSelect(int joyY) {
    static int selectedIndex = 0;  // 選擇的文件索引，靜態變量保持其值
    int joyVal = analogRead(joyPinY);

    // 根據操縱桿的Y軸讀數來更新選擇
    if (joyVal < 400 && lastJoyY >= 400) {
        selectedIndex = selectedIndex > 0 ? selectedIndex - 1 : 0;
    } else if (joyVal > 600 && lastJoyY <= 600) {
        selectedIndex = selectedIndex < fileIndex - 1 ? selectedIndex + 1 : fileIndex - 1;
    }
    lastJoyY = joyVal;

    // 顯示當前選擇的文件名
    lcd.clear();
    lcd.print(">");
    lcd.print(fileList[selectedIndex]);  // 確保使用正確的變量名

    // 確認選擇
    if (digitalRead(buttonBPin) == LOW) {
        // 打開選擇的文件進行讀取或編輯
        fileName = fileList[selectedIndex];  // 使用選中的文件名
        inputString = "";  // 清空編輯緩衝區
        currentState = EDIT;  // 切換到編輯狀態
        lcd.clear();
        lcd.print("Editing: ");
        lcd.print(fileName);
        delay(500);  // 防止按鈕彈跳引起的多次觸發
    }
}

void saveToFile() {
  if (fileName == "") {
    fileName = "default.txt";  // 如果沒有指定文件名，使用默認名稱
  }
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) {
    myFile.println(inputString);
    myFile.close();
    lcd.clear();
    lcd.print("Saved: ");
    lcd.print(fileName);
  } else {
    lcd.clear();
    lcd.print("Error Saving");
  }
  delay(2000);  // 顯示保存狀態一段時間
  lcd.clear();
  updateLCD();  // 回到編輯視圖或其他適當視圖
}


void listFiles() {
  root = SD.open("/");
  if (!root) {
    lcd.clear();
    lcd.print("No Files!");
    return;
  }
  fileIndex = 0;  // 重設文件索引
  lcd.clear();
  File entry = root.openNextFile();
  while (entry) {
    if (!entry.isDirectory()) {
      fileList[fileIndex] = String(entry.name());  // 儲存文件名到數組
      lcd.setCursor(0, fileIndex);
      lcd.print(entry.name());
      fileIndex++;
      if (fileIndex >= 10) break;  // 假設最多顯示10個文件
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();  // 關閉根目錄以便下次重新讀取
}
