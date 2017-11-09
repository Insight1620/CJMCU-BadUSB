#include <Keyboard.h>
#include <SPI.h>
#include <SD.h>

#define KEY_MENU 0xED
#define KEY_BREAK 0xD0
#define KEY_NUMLOCK 0xDB
#define KEY_PRINTSCREEN 0xCE
#define KEY_SCROLLLOCK 0xCF
#define KEY_SPACE 0xB4
#define KEY_LEFT_CTRL 0x80
#define KEY_LEFT_SHIFT 0x81
#define KEY_LEFT_ALT 0x82
#define KEY_LEFT_GUI 0x83
#define KEY_RIGHT_CTRL 0x84
#define KEY_RIGHT_SHIFT 0x85
#define KEY_RIGHT_ALT 0x86
#define KEY_RIGHT_GUI 0x87
#define KEY_UP_ARROW 0xDA
#define KEY_DOWN_ARROW 0xD9
#define KEY_LEFT_ARROW 0xD8
#define KEY_RIGHT_ARROW 0xD7
#define KEY_BACKSPACE 0xB2
#define KEY_TAB 0xB3
#define KEY_RETURN 0xB0
#define KEY_ESC 0xB1
#define KEY_INSERT 0xD1
#define KEY_DELETE 0xD4
#define KEY_PAGE_UP 0xD3
#define KEY_PAGE_DOWN 0xD6
#define KEY_HOME 0xD2
#define KEY_END 0xD5
#define KEY_CAPS_LOCK 0xC1
#define KEY_F1 0xC2
#define KEY_F2 0xC3
#define KEY_F3 0xC4
#define KEY_F4 0xC5
#define KEY_F5 0xC6
#define KEY_F6 0xC7
#define KEY_F7 0xC8
#define KEY_F8 0xC9
#define KEY_F9 0xCA
#define KEY_F10 0xCB
#define KEY_F11 0xCC
#define KEY_F12 0xCD
#define KEY_F13 0xF0
#define KEY_F14 0xF1
#define KEY_F15 0xF2
#define KEY_F16 0xF3
#define KEY_F17 0xF4
#define KEY_F18 0xF5
#define KEY_F19 0xF6
#define KEY_F20 0xF7
#define KEY_F21 0xF8
#define KEY_F22 0xF9
#define KEY_F23 0xFA
#define KEY_F24 0xFB

const int chipSelect = 4;
String cmd;
String arg;
String mode;
String payload;
String prevCmd;
String prevArg;
char argChar;
char prevArgChar;
char charBuff;
char breakChar;
int defaultDelay = 0;
int led2 = 8;
File root;
File myFile;
bool errLog;

void setup() {
  pinMode(led2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Keyboard.begin();
  Serial.begin(9600);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }

  root = SD.open("/");

  mode = readConfig("mode.cfg");
  payload = readConfig("exec.cfg");

  if (mode == "c") {
    delivery(payload);
  }
  else if (mode == "a") {
    delivery(payload);
    mode = "m";
    writeConfig("mode.cfg", mode);
  }
  //delivery("script2.bds");
  
  management();
  
  Keyboard.end();
}

void management() {
  digitalWrite(led2, HIGH);
  
  while (!Serial) {
    if (errLog) {
      digitalWrite(led2, HIGH);
      delay(200);
      digitalWrite(led2, LOW);
      delay(200);
    }
    ;
  }
 
  Serial.println("Available payloads:");
  printDirectory(root, 0);
  root.close();
  Serial.println();
  Serial.println("Available modes: ");
  Serial.println("m => management mode");
  Serial.println("a => auto-disarm mode");
  Serial.println("c => continuous delivery mode");
  Serial.println();
  Serial.print("Current mode: ");
  Serial.println(mode);
  Serial.print("Current payload: ");
  Serial.println(payload);

  Serial.println();
  Serial.println("Input mode:");
  writeConfig("mode.cfg", inputData());
  Serial.println();
  Serial.println("Input payload:");
  writeConfig("exec.cfg", inputData());
  Serial.println();
  Serial.println("Get ready to have some fun :)");
}

void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

String inputData() {
  String inputStr;
  while (1) {
    if (Serial.available() > 0) {
      inputStr = Serial.readStringUntil('\n');
      break;
    }
  }
  return inputStr;
}

String readConfig(String fileName) {
  String fileContent = "";
  myFile = SD.open(fileName);
  if (myFile) {
    while (myFile.available()) {
      fileContent += char(myFile.read());
    }
    return fileContent;
    myFile.close();
  } else {
    Serial.println("error opening file");
  }
}

void writeConfig(String fileName, String inputData) {
  SD.remove(fileName);
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) {
    myFile.print(inputData);
    myFile.close();
  }
}

void delivery (String fileName) {
  delay(800);
  File dataFile = SD.open(fileName);
  
  if (dataFile) {
    while (dataFile.available()) {
      if (defaultDelay != 0) {
        delay(defaultDelay);
      }
      
      parseCmd(dataFile);
      
      if (cmd == "GUI" || cmd == "WINDOWS") {
        if (breakChar == ' ') {
          argChar = dataFile.read();
          cmdGui(argChar);
          //just to remove trailing \n
          dataFile.read();
        }
        else {
          cmdGui(0x00);
        }

      }
      else if (cmd == "DELAY") {
        parseArg(dataFile);
        cmdDelay(arg);
      }
      else if (cmd == "STRING") {
        cmdString(dataFile);
      }
      else if (cmd == "ENTER") {
        
        cmdPressKey(KEY_RETURN);
      }
      else if (cmd == "REM") {
        if (breakChar == ' ') {
          parseArg(dataFile);
        }
        cmdRem();
      }
      else if (cmd == "DEFAULT_DELAY" || cmd == "DEFAULTDELAY") {
        parseArg(dataFile);
        cmdDefaultDelay(arg);
      }
      else if (cmd == "MENU" || cmd == "APP") {
        cmdPressKey(KEY_MENU);
      }
      else if (cmd == "DOWNARROW" || cmd == "DOWN") {
        cmdPressKey(KEY_DOWN_ARROW);
      }
      else if (cmd == "LEFTARROW" || cmd == "LEFT") {
        cmdPressKey(KEY_LEFT_ARROW);
      }
      else if (cmd == "RIGHTARROW" || cmd == "RIGHT") {
        cmdPressKey(KEY_RIGHT_ARROW);
      }
      else if (cmd == "UPARROW" || cmd == "UP") {
        cmdPressKey(KEY_UP_ARROW);
      }
      else if (cmd == "BREAK" || cmd == "PAUSE") {
        cmdPressKey(KEY_BREAK);
      }
      else if (cmd == "CAPSLOCK") {
        cmdPressKey(KEY_CAPS_LOCK);
      }
      else if (cmd == "DELETE") {
        cmdPressKey(KEY_DELETE);
      }
      else if (cmd == "END") {
        cmdPressKey(KEY_END);
      }
      else if (cmd == "ESC" || cmd == "ESCAPE") {
        cmdPressKey(KEY_ESC);
      }
      else if (cmd == "HOME") {
        cmdPressKey(KEY_HOME);
      }
      else if (cmd == "INSERT") {
        cmdPressKey(KEY_INSERT);
      }
      else if (cmd == "NUMLOCK") {
        cmdPressKey(KEY_NUMLOCK);
      }
      else if (cmd == "PAGEUP") {
        cmdPressKey(KEY_PAGE_UP);
      }
      else if (cmd == "PAGEDOWN") {
        cmdPressKey(KEY_PAGE_DOWN);
      }
      else if (cmd == "PRINTSCREEN") {
        cmdPressKey(KEY_PRINTSCREEN);
      }
      else if (cmd == "SCROLLLOCK") {
        cmdPressKey(KEY_SCROLLLOCK);
      }
      else if (cmd == "SPACE") {
        cmdPressKey(KEY_SPACE);
      }
      else if (cmd == "TAB") {
        cmdPressKey(KEY_TAB);
      }
      else if (cmd == "REPEAT") {
        parseArg(dataFile);
        cmdRepeat(arg);
      }
      else if (cmd == "CTRL" || cmd == "CONTROL" ) {
        parseArg(dataFile);
        arg = arg + '\n';
        cmdKeyCombo(KEY_LEFT_CTRL, arg);
      }
      else if (cmd == "ALT") {
        parseArg(dataFile);
        arg = arg + '\n';
        cmdKeyCombo(KEY_LEFT_ALT, arg);
      }
      else if (cmd == "SHIFT") {
        parseArg(dataFile);
        arg = arg + '\n';
        cmdKeyCombo(KEY_LEFT_SHIFT, arg);
      }
      else if (cmd == "F1") {
        cmdPressKey(KEY_F1);
      }
      else if (cmd == "F2") {
        cmdPressKey(KEY_F2);
      }
      else if (cmd == "F3") {
        cmdPressKey(KEY_F3);
      }
      else if (cmd == "F4") {
        cmdPressKey(KEY_F4);
      }
      else if (cmd == "F5") {
        cmdPressKey(KEY_F5);
      }
      else if (cmd == "F6") {
        cmdPressKey(KEY_F6);
      }
      else if (cmd == "F7") {
        cmdPressKey(KEY_F7);
      }
      else if (cmd == "F8") {
        cmdPressKey(KEY_F8);
      }
      else if (cmd == "F9") {
        cmdPressKey(KEY_F9);
      }
      else if (cmd == "F10") {
        cmdPressKey(KEY_F10);
      }
      else if (cmd == "F11") {
        cmdPressKey(KEY_F11);
      }
      else if (cmd == "F12") {
        cmdPressKey(KEY_F12);
      }
      else {
        errLog = true;
        cmd = "";
        arg = "";
        continue;
      }
    }
    dataFile.close();
  }
  else {
    Serial.println("Error opening script file");
  }
}

void cmdRepeat (String arg_l) {
  int counter = arg_l.toInt();
  argChar = prevArgChar;
  
  while (counter) {
    if (defaultDelay != 0) {
      delay(defaultDelay);
    }
    
    cmd = prevCmd;
    arg = prevArg;
    
    counter -= 1;

    if (prevCmd == "GUI" || prevCmd == "WINDOWS") {
      cmdGui(prevArgChar);
    }
    else if (prevCmd == "DELAY") {
      cmdDelay(prevArg);
    }
    else if (prevCmd == "ENTER") {
      cmdPressKey(KEY_RETURN);
    }
    else if (prevCmd == "MENU" || prevCmd == "APP") {
      cmdPressKey(KEY_MENU);
    }
    else if (prevCmd == "DOWNARROW" || prevCmd == "DOWN") {
      cmdPressKey(KEY_DOWN_ARROW);
    }
    else if (prevCmd == "LEFTARROW" || prevCmd == "LEFT") {
      cmdPressKey(KEY_LEFT_ARROW);
    }
    else if (prevCmd == "RIGHTARROW" || prevCmd == "RIGHT") {
      cmdPressKey(KEY_RIGHT_ARROW);
    }
    else if (prevCmd == "UPARROW" || prevCmd == "UP") {
      cmdPressKey(KEY_UP_ARROW);
    }
    else if (prevCmd == "BREAK" || prevCmd == "PAUSE") {
      cmdPressKey(KEY_BREAK);
    }
    else if (prevCmd == "CAPSLOCK") {
      cmdPressKey(KEY_CAPS_LOCK);
    }
    else if (prevCmd == "DELETE") {
      cmdPressKey(KEY_DELETE);
    }
    else if (prevCmd == "END") {
      cmdPressKey(KEY_END);
    }
    else if (prevCmd == "ESC" || prevCmd == "ESCAPE") {
      cmdPressKey(KEY_ESC);
    }
    else if (prevCmd == "HOME") {
      cmdPressKey(KEY_HOME);
    }
    else if (prevCmd == "INSERT") {
      cmdPressKey(KEY_INSERT);
    }
    else if (prevCmd == "NUMLOCK") {
      cmdPressKey(KEY_NUMLOCK);
    }
    else if (prevCmd == "PAGEUP") {
      cmdPressKey(KEY_PAGE_UP);
    }
    else if (prevCmd == "PAGEDOWN") {
      cmdPressKey(KEY_PAGE_DOWN);
    }
    else if (prevCmd == "PRINTSCREEN") {
      cmdPressKey(KEY_PRINTSCREEN);
    }
    else if (prevCmd == "SCROLLLOCK") {
      cmdPressKey(KEY_SCROLLLOCK);
    }
    else if (prevCmd == "SPACE") {
      cmdPressKey(KEY_SPACE);
    }
    else if (prevCmd == "TAB") {
      cmdPressKey(KEY_TAB);
    }
    else if (cmd == "CTRL" || cmd == "CONTROL" ) {
      cmdKeyCombo(KEY_LEFT_CTRL, prevArg);
    }
    else if (cmd == "ALT") {
      cmdKeyCombo(KEY_LEFT_ALT, prevArg);
    }
    else if (cmd == "SHIFT") {
      cmdKeyCombo(KEY_LEFT_SHIFT, prevArg);
    }
    else if (cmd == "F1") {
      cmdPressKey(KEY_F1);
    }
    else if (cmd == "F2") {
      cmdPressKey(KEY_F2);
    }
    else if (cmd == "F3") {
      cmdPressKey(KEY_F3);
    }
    else if (cmd == "F4") {
      cmdPressKey(KEY_F4);
    }
    else if (cmd == "F5") {
      cmdPressKey(KEY_F5);
    }
    else if (cmd == "F6") {
      cmdPressKey(KEY_F6);
    }
    else if (cmd == "F7") {
      cmdPressKey(KEY_F7);
    }
    else if (cmd == "F8") {
      cmdPressKey(KEY_F8);
    }
    else if (cmd == "F9") {
      cmdPressKey(KEY_F9);
    }
    else if (cmd == "F10") {
      cmdPressKey(KEY_F10);
    }
    else if (cmd == "F11") {
      cmdPressKey(KEY_F11);
    }
    else if (cmd == "F12") {
      cmdPressKey(KEY_F12);
    }
  }
  cmd = "";
  arg = "";
}

void cmdRem (){
  cmd = "";
  arg = "";
}

void cmdDefaultDelay (String arg_l) {
  defaultDelay = arg_l.toInt();
  cmd = "";
  arg = "";
}

void cmdDelay (String arg_l) {
  delay(arg_l.toInt());
  prevCmd = cmd;
  cmd = "";
  prevArg = arg_l;
  arg = "";
}

void cmdGui (char argChar_l) {
  Keyboard.press(KEY_LEFT_GUI);
  delay(100);
  if (argChar != 0x00) {
    Keyboard.press(argChar_l);
    delay(100); 
  }
  Keyboard.releaseAll();
  prevCmd = cmd;
  cmd = "";
  prevArgChar = argChar_l;
  argChar = 0x00;
}

void cmdPressKey(int key) {
  Keyboard.press(key);
  delay(100);
  Keyboard.release(key);
  prevCmd = cmd;
  cmd = "";
  prevArg = arg;
  arg = "";
}

void cmdString (File dataFile) {
  while (true) {
    charBuff = dataFile.read();
    if (charBuff == '\n') {
      //Keyboard.print(charBuff); //adds \n at the end of the line
      break;
    }
    else {
      Keyboard.print(charBuff);
    }
  }
  cmd = "";
  arg = "";
}

void parseCmd(File dataFile) {
  while (true) {
    charBuff = dataFile.read();
    if (charBuff == ' ' || charBuff == '\n' || cmd.length() > 15) {
      breakChar = charBuff;
      break;
    }
    else {
      cmd = cmd + charBuff;
    }
  }
}

void parseArg(File dataFile) {
  while (true) {
    charBuff = dataFile.read();
    if (charBuff == '\n') {
      break;
    }
    else {
      arg = arg + charBuff;
    }
  }
}

void cmdKeyCombo(int key, String arg_l) {
  String argKey;
  int argLength = arg_l.length();
  
  Keyboard.press(key);
  delay(100);
  
  for (int i = 0; i <= argLength; i++) {
    charBuff = arg_l.charAt(i);
    if (charBuff == ' ' || charBuff == '\n') {
      if (argKey.length() == 1) {
        Keyboard.press(argKey.charAt(0));
        delay(100);
      }
      else {
        if (argKey == "ENTER") {
          Keyboard.press(KEY_RETURN);
        }
        else if (argKey == "MENU" || argKey == "APP") {
          Keyboard.press(KEY_MENU);
        }
        else if (argKey == "DOWNARROW" || argKey == "DOWN") {
          Keyboard.press(KEY_DOWN_ARROW);
        }
        else if (argKey == "LEFTARROW" || argKey == "LEFT") {
          Keyboard.press(KEY_LEFT_ARROW);
        }
        else if (argKey == "RIGHTARROW" || argKey == "RIGHT") {
          Keyboard.press(KEY_RIGHT_ARROW);
        }
        else if (argKey == "UPARROW" || argKey == "UP") {
          Keyboard.press(KEY_UP_ARROW);
        }
        else if (argKey == "BREAK" || argKey == "PAUSE") {
          Keyboard.press(KEY_BREAK);
        }
        else if (argKey == "CAPSLOCK") {
          Keyboard.press(KEY_CAPS_LOCK);
        }
        else if (argKey == "DELETE") {
          Keyboard.press(KEY_DELETE);
        }
        else if (argKey == "END") {
          Keyboard.press(KEY_END);
        }
        else if (argKey == "ESC" || argKey == "ESCAPE") {
          Keyboard.press(KEY_ESC);
        }
        else if (argKey == "HOME") {
          Keyboard.press(KEY_HOME);
        }
        else if (argKey == "INSERT") {
          Keyboard.press(KEY_INSERT);
        }
        else if (argKey == "NUMLOCK") {
          Keyboard.press(KEY_NUMLOCK);
        }
        else if (argKey == "PAGEUP") {
          Keyboard.press(KEY_PAGE_UP);
        }
        else if (argKey == "PAGEDOWN") {
          Keyboard.press(KEY_PAGE_DOWN);
        }
        else if (argKey == "PRINTSCREEN") {
          Keyboard.press(KEY_PRINTSCREEN);
        }
        else if (argKey == "SCROLLLOCK") {
          Keyboard.press(KEY_SCROLLLOCK);
        }
        else if (argKey == "SPACE") {
          Keyboard.press(KEY_SPACE);
        }
        else if (argKey == "TAB") {
          Keyboard.press(KEY_TAB);
        }
        else if (argKey == "ALT") {
          Keyboard.press(KEY_LEFT_ALT);
        }
        else if (argKey == "SHIFT") {
          Keyboard.press(KEY_LEFT_SHIFT);
        }
        else if (argKey == "CTRL") {
          Keyboard.press(KEY_LEFT_CTRL);
        }
        else if (argKey == "F1") {
          Keyboard.press(KEY_F1);
        }
        else if (argKey == "F2") {
          Keyboard.press(KEY_F2);
        }
        else if (argKey == "F3") {
          Keyboard.press(KEY_F3);
        }
        else if (argKey == "F4") {
          Keyboard.press(KEY_F4);
        }
        else if (argKey == "F5") {
          Keyboard.press(KEY_F5);
        }
        else if (argKey == "F6") {
          Keyboard.press(KEY_F6);
        }
        else if (argKey == "F7") {
          Keyboard.press(KEY_F7);
        }
        else if (argKey == "F8") {
          Keyboard.press(KEY_F8);
        }
        else if (argKey == "F9") {
          Keyboard.press(KEY_F9);
        }
        else if (argKey == "F10") {
          Keyboard.press(KEY_F10);
        }
        else if (argKey == "F11") {
          Keyboard.press(KEY_F11);
        }
        else if (argKey == "F12") {
          Keyboard.press(KEY_F12);
        }
        else if (argKey == "GUI" || argKey == "WINDOWS") {
          Keyboard.press(KEY_LEFT_GUI);
        }
        delay(100);
      }
      argKey = "";
    }
    else {
      argKey = argKey + charBuff;
    }
  }
  Keyboard.releaseAll();
  prevCmd = cmd;
  cmd = "";
  prevArg = arg;
  arg = "";
}

void loop() {
}

