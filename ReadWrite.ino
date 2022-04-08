#include <EEPROM.h>

/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Arduino_JSON.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

//#ifndef STASSID
//#define STASSID "8249FB"
//#define STAPSK  "52T76B2B00184"
//#endif

#define SET false
#define REC true
#define API_KEY "a7fUD3411gE"
#define SSID_ADDRESS      0x100
#define PASSWORD_ADDRESS  0x200

//const char *ssid = STASSID;
//const char *password = STAPSK;

ESP8266WebServer server(80);
File myFile;


String enclosureSet1 = "ENCLSET1.TXT";
String enclosureSet2 = "ENCLSET2.TXT";
String enclosureSet3 = "ENCLSET3.TXT";

String enclosureRec1 = "ENCLREC1.TXT";
String enclosureRec2 = "ENCLREC2.TXT";
String enclosureRec3 = "ENCLREC3.TXT";

String wifiCredentialsFile = "CREDENTIALS.TXT";

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.nist.gov", 0, 60000);
unsigned long unix_epoch = 0;

typedef struct {
  uint32_t timestmp;
  uint8_t temperature; 
  uint8_t humidity; 
  uint16_t luminosityLow;
  uint16_t luminosityHigh; 
} recordedData;


typedef struct {
  uint8_t temperature;
  uint8_t humidity;
  uint16_t luminosity;
} desiredData;


String defaultDesiredConditions = "80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,1000+80,30,0+80,30,0+80,30,0+80,30,0+80,30,0+80,30,0";


String html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Indoor Farming Enclosure</title>
    <style>
        html, body { font-family: Arial, Helvetica, sans-serif; }
        body { padding: 12px; }
        button { float: right; font-weight: bold; background-color: #008CBA; padding: 16px 32px; text-align: center; margin: 8px 0px; text-decoration: none; font-size: 16px; color: white;}
        div div input { width: 80%; }
        .api-key { padding-bottom: 12px; }
        .enclosure { padding-bottom: 12px; }
        .grid-item { padding: 8px; }
        .grid-row-wrapper { display: contents; } 
        .grid-row-wrapper:nth-child(odd) > .grid-item { background-color: #dddddd; }
        .grid-wrapper { display: grid; grid-template-columns: repeat(4, 1fr); text-align: center; }
        .grid-wrapper div { border: 1px solid black; }
        .header { font-weight: bold; text-align: left; padding: 8px; }
        .hour { font-weight: bold; text-align: left; padding: 8px; }
        .label { font-weight: bold; }
    </style>
</head>
<body>
    <h1>Indoor Farming Enclosure: Set Desired Conditions</h1>

    <form action="http://www.mywebsitename.com" method="GET">
        <div class="api-key">
            <label class="label" for="api">API Key</label>
            <input type="text" name="api-key">
        </div>
        <div class="enclosure">
            <label class="label">Enclosure</label>
            <select name="enclosureList" id="enclosures">
                <option value="1">1</option>
                <option value="2">2</option>
                <option value="3">3</option>
            </select>
        </div>
        <div class="grid-wrapper">
            <div class="header">Hour</div>
            <div class="header">Temperature</div>
            <div class="header">Humidity</div>
            <div class="header">Luminosity</div>

            <div class="grid-row-wrapper">
                <div class="grid-item hour">0</div>
                <div class="grid-item"><input type="number" name="0-temperature"></div>
                <div class="grid-item"><input type="number" name="0-humidity"></div>
                <div class="grid-item"><input type="number" name="0-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">1</div>
                <div class="grid-item"><input type="number" name="1-temperature"></div>
                <div class="grid-item"><input type="number" name="1-humidity"></div>
                <div class="grid-item"><input type="number" name="1-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">2</div>
                <div class="grid-item"><input type="number" name="2-temperature"></div>
                <div class="grid-item"><input type="number" name="2-humidity"></div>
                <div class="grid-item"><input type="number" name="2-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">3</div>
                <div class="grid-item"><input type="number" name="3-temperature"></div>
                <div class="grid-item"><input type="number" name="3-humidity"></div>
                <div class="grid-item"><input type="number" name="3-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">4</div>
                <div class="grid-item"><input type="number" name="4-temperature"></div>
                <div class="grid-item"><input type="number" name="4-humidity"></div>
                <div class="grid-item"><input type="number" name="4-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">5</div>
                <div class="grid-item"><input type="number" name="5-temperature"></div>
                <div class="grid-item"><input type="number" name="5-humidity"></div>
                <div class="grid-item"><input type="number" name="5-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">6</div>
                <div class="grid-item"><input type="number" name="6-temperature"></div>
                <div class="grid-item"><input type="number" name="6-humidity"></div>
                <div class="grid-item"><input type="number" name="6-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">7</div>
                <div class="grid-item"><input type="number" name="7-temperature"></div>
                <div class="grid-item"><input type="number" name="7-humidity"></div>
                <div class="grid-item"><input type="number" name="7-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">8</div>
                <div class="grid-item"><input type="number" name="8-temperature"></div>
                <div class="grid-item"><input type="number" name="8-humidity"></div>
                <div class="grid-item"><input type="number" name="8-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">9</div>
                <div class="grid-item"><input type="number" name="9-temperature"></div>
                <div class="grid-item"><input type="number" name="9-humidity"></div>
                <div class="grid-item"><input type="number" name="9-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">10</div>
                <div class="grid-item"><input type="number" name="10-temperature"></div>
                <div class="grid-item"><input type="number" name="10-humidity"></div>
                <div class="grid-item"><input type="number" name="10-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">11</div>
                <div class="grid-item"><input type="number" name="11-temperature"></div>
                <div class="grid-item"><input type="number" name="11-humidity"></div>
                <div class="grid-item"><input type="number" name="11-luminosity"></div>
            </div>
                        <div class="grid-row-wrapper">
                <div class="grid-item hour">12</div>
                <div class="grid-item"><input type="number" name="12-temperature"></div>
                <div class="grid-item"><input type="number" name="12-humidity"></div>
                <div class="grid-item"><input type="number" name="12-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">13</div>
                <div class="grid-item"><input type="number" name="13-temperature"></div>
                <div class="grid-item"><input type="number" name="13-humidity"></div>
                <div class="grid-item"><input type="number" name="13-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">14</div>
                <div class="grid-item"><input type="number" name="14-temperature"></div>
                <div class="grid-item"><input type="number" name="14-humidity"></div>
                <div class="grid-item"><input type="number" name="14-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">15</div>
                <div class="grid-item"><input type="number" name="15-temperature"></div>
                <div class="grid-item"><input type="number" name="15-humidity"></div>
                <div class="grid-item"><input type="number" name="15-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">16</div>
                <div class="grid-item"><input type="number" name="16-temperature"></div>
                <div class="grid-item"><input type="number" name="16-humidity"></div>
                <div class="grid-item"><input type="number" name="16-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">17</div>
                <div class="grid-item"><input type="number" name="17-temperature"></div>
                <div class="grid-item"><input type="number" name="17-humidity"></div>
                <div class="grid-item"><input type="number" name="17-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">18</div>
                <div class="grid-item"><input type="number" name="18-temperature"></div>
                <div class="grid-item"><input type="number" name="18-humidity"></div>
                <div class="grid-item"><input type="number" name="18-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">19</div>
                <div class="grid-item"><input type="number" name="19-temperature"></div>
                <div class="grid-item"><input type="number" name="19-humidity"></div>
                <div class="grid-item"><input type="number" name="19-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">20</div>
                <div class="grid-item"><input type="number" name="20-temperature"></div>
                <div class="grid-item"><input type="number" name="20-humidity"></div>
                <div class="grid-item"><input type="number" name="20-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">21</div>
                <div class="grid-item"><input type="number" name="21-temperature"></div>
                <div class="grid-item"><input type="number" name="21-humidity"></div>
                <div class="grid-item"><input type="number" name="21-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">22</div>
                <div class="grid-item"><input type="number" name="22-temperature"></div>
                <div class="grid-item"><input type="number" name="22-humidity"></div>
                <div class="grid-item"><input type="number" name="22-luminosity"></div>
            </div>
            <div class="grid-row-wrapper">
                <div class="grid-item hour">23</div>
                <div class="grid-item"><input type="number" name="23-temperature"></div>
                <div class="grid-item"><input type="number" name="23-humidity"></div>
                <div class="grid-item"><input type="number" name="23-luminosity"></div>
            </div>
        </div>
    </form>
<button onclick="submit()">SUBMIT</button>
    <script>
        function submit() {
            var apiKey = document.getElementsByName('api-key')[0].value;
            var enclosure = document.getElementsByName('enclosureList')[0].value;

            var temps = [];
            var humidity = [];
            var lum = [];

            for(let i = 0; i < 24; i++) {
              temps[i] = document.getElementsByName(i.toString() + '-temperature')[0].value;
              humidity[i] = document.getElementsByName(i.toString() + '-humidity')[0].value;
              lum[i] = document.getElementsByName(i.toString() + '-luminosity')[0].value;
            }
            
            var finalUrl =
            '/APIKEY/' + apiKey + 
            '/ENCLOSURE/' + enclosure + 
            '/SETDATA/';

            for(let j = 0; j < 23; j++) {
              finalUrl += temps[j] + ',' + humidity[j] + ',' + lum[j] + '+';
            }
            finalUrl += temps[23] + ',' + humidity[23] + ',' + lum[23];
            
            console.log(finalUrl);
            openInNewTab(finalUrl);
        }

        function openInNewTab(url) {
            window.open(url, '_blank').focus;
        }
    </script>
</body>
</html>
)=====";




bool verifyDesiredDataLine(String dataLine);

// -------------------------------------------------------------
// -------------------------------------------------------------

// true: rec
// false: set
String getFile(int enclosureNumber, bool type) {
  String enclosureFile;
  
  switch(enclosureNumber) {
    case 1:
      if(type) {
        return enclosureRec1;
      }
      else {
        return enclosureSet1;
      }
    case 2:
      if(type) {
        return enclosureRec2;
      }
      else {
        return enclosureSet2;
      }
    case 3:
      if(type) {
        return enclosureRec3;
      }
      else {
        return enclosureSet3;
      }
    default:
      return "";
  }
}

//String getWebPage(void) {
//  myFile = SD.open("INDEX.HTM");
//  String str;
//  while(myFile.available()) {
//    str += myFile.read();
//  }
//
//  myFile.close();
//  return str;
//}
// -------------------------------------------------------------

bool verifyRecordedData(String data) {
  String temp;
  int vals[5];
  int valCount = 0;
  
  for(int i = 0; i < data.length(); i++) {
    if(data[i] != ',') {
      temp += data[i];
    }
    else {
      vals[valCount++] = temp.toInt();
      temp = "";
    }
  }

  if(
    (vals[0] == 0 && vals[1] == 0 && vals[2] == 0 && vals[3] == 0 && vals[4] == 0) || 
    valCount != 5
  ) {
    return false;
  }
  
  return true;
}

// -------------------------------------------------------------

bool recordCurrentEnclosureData(int enclosureNumber, String data) { 
  if(!verifyRecordedData(data)) {
    return false; 
  }
  
  myFile = SD.open(getFile(enclosureNumber, REC), FILE_WRITE);
  myFile.println(data);
  myFile.close();
  
  return true;
}

// -------------------------------------------------------------

String getRecordedEnclosureData(int enclosureNumber, int line) {
  JSONVar obj;
  char c = ' ';
  String data;
  String temp;
  int tempNum[5];
  int count = 0;
  
  myFile = SD.open(getFile(enclosureNumber, REC));

  // Read a line of the file at a time until desired line is reached
  while(line > 0) {
    // Read a line of the file
    while(myFile.available() && myFile.read() != '\n');
    line--;
  }

  while(myFile.available() && c != '\n') {
    c = myFile.read();
    data += c;
  }

  myFile.close();

  if(data != "") {
    for(int i = 0; i < data.length(); i++) {
      if(data[i] != ',') {
        temp += data[i];
      }
      else {
        tempNum[count++] = temp.toInt();
        temp = "";
      }
    }
    obj["timestamp"] = tempNum[0];
    obj["temperature"] = tempNum[1];
    obj["humidity"] = tempNum[2];
    obj["luminosityLow"] = tempNum[3];
    obj["luminosityHigh"] = tempNum[4];
    
    return JSON.stringify(obj);
  }
  else {
    return "error";
  }
}

// -------------------------------------------------------------

String getCurrentEnclosureData(int enclosureNumber) {
  return "80)";
}

// -------------------------------------------------------------

bool verifyDesiredData(String data) {
  String tempData;
  int lineCount = 0;
  
  for(int i = 0; i < data.length(); i++) {
    if(data[i] != '+') {
      tempData += data[i];
    }
    else {
      if(!verifyDesiredDataLine(tempData)) {
        return false;
      }
      lineCount++;
      tempData = "";
    }
  }
  if(!verifyDesiredDataLine(tempData) || lineCount != 23) {
      return false;
  }
  return true;
}

// -------------------------------------------------------------

bool verifyDesiredDataLine(String dataLine) {
  String tempData;
  int elementCount = 0;
  
  for(int i = 0; i < dataLine.length(); i++) {
    if(dataLine[i] == ',' || dataLine[i] == '\n') {
      if((elementCount == 0 || elementCount == 1) && tempData.toInt() < 0 && tempData.toInt() > 99) {
        return false;
      }
      else if(elementCount == 2 && tempData.toInt() < 0 && tempData.toInt() > 50000) {
        return false;
      }
      elementCount++;
      tempData = "";
    }
    else {
      tempData += dataLine[i];
    }
  }
  return true;
}

// -------------------------------------------------------------

bool setEnclosureDesiredData(int enclosureNumber, String data) {
  if(enclosureNumber > 3) {
    return false;
  }
  
  String enclosureFile = getFile(enclosureNumber, SET);
  String temp = "0,";
  uint8_t hourCount = 1;

  if(!verifyDesiredData(data)) {
    return false;
  }
  
  // Delete the file to clear old data
  SD.remove(enclosureFile);

  // Recreate file
  myFile = SD.open(enclosureFile, FILE_WRITE);

  // Ensure file was recreated
  if(!SD.exists(enclosureFile)) {
    return false;
  }
  
  // Load file with desired data
  for(int i = 0; i < data.length(); i++) {
    if(data[i] != '+') {
      temp += data[i];
    }
    else {
      myFile.println(temp);
      temp = (String)hourCount++ + ",";
    }
  }

  myFile.println(temp);

  // Close the file
  myFile.close();

  return true;
}

// -------------------------------------------------------------

JSONVar getEnclosureDesiredData(int enclosureNumber, int currentDayHours) {
  JSONVar obj;
  char c = ' ';
  String data;
  String temp;
  int tempNum[7];
  int count = 0;
  
  String enclosureFile = getFile(enclosureNumber, SET);

  // Open the file
  myFile = SD.open(enclosureFile);

  // Read a line of the file at a time until desired line is reached
  while(currentDayHours > 0) {
    // Read a line of the file
    while(myFile.available() && myFile.read() != '\n');
    currentDayHours--;
  }

  while(myFile.available() && c != '\n') {
    c = myFile.read();
    data += c;
  }
  
  myFile.close();

  for(int i = 0; i < data.length(); i++) {
    if(data[i] != ',' && data[i] != '\n') {
      temp += data[i];
    }
    else {
      tempNum[count++] = temp.toInt();
      temp = "";
    }
  }
  obj["hour"] = tempNum[0];
  obj["temperature"] = tempNum[1];
  obj["humidity"] = tempNum[2];
  obj["luminosity"] = tempNum[3];
  
  return obj;//JSON.stringify(obj);
}

// -------------------------------------------------------------

String getFullDayEnclosureDesiredData(int enclosureNumber) {
  String data = "{\"0\":";
  for(int i = 0; i < 24; i++) {
    data += JSON.stringify(getEnclosureDesiredData(enclosureNumber, i));
    if(i != 23) {
      data += ",\"" + (String)(i+1) + "\":";
    }
  }
  data += "}";
  return data;
}

// ---------------------------------------------------------------------

void initAPI(void) {
  if(WiFi.status() == WL_CONNECTED) {
    server.on(F("/"), []() {
      server.send(200, "text/plain", "University of Nebraska - Lincoln\nElectrical & Computer Engineering\n\nCapstone Team 108\n2021 - 2022\n\n\nIndoor Farming Enclosure");
    });

    server.on("/set", []() {
      server.send(200, "text/html", html);
    });
  
  
    // ---------------------------------------------------------------------
    // GET /apiKey/${}/enclosure/${}/setData/${}
    // setData: t0,h0,l0+t1,h1,l1+...+t23,h23,l23
    // ---------------------------------------------------------------------
    server.on(UriBraces("/APIKEY/{}/ENCLOSURE/{}/SETDATA/{}"), []() {
      String apiKey = server.pathArg(0);
      String enclosure = server.pathArg(1);
      String data = server.pathArg(2);
      if(apiKey == API_KEY) {
        if(setEnclosureDesiredData(enclosure.toInt(), data)) {
          server.send(200, "text/plain", "Enclosure desired data saved: Success.");
        }
        else {
          server.send(400, "text/plain", "Enclosure desired data saved: ERROR.");
        }
      }
      else {
        server.send(401, "text/plain", "Invalid API key.");
      }
    });
  
    // ---------------------------------------------------------------------
    // GET /recordedData/enclosure/${}/line/${}
    // ---------------------------------------------------------------------
    server.on(UriRegex("^/recordedData\\/enclosure\\/([0-9]+)\\/line\\/([0-9]+)$"), []() {
      String enclosure = server.pathArg(0);
      String line = server.pathArg(1);
      String recData = getRecordedEnclosureData(enclosure.toInt(), line.toInt());
      if(recData == "error") {
        server.send(400, "text/plain", "Invalid line number");
      }
      else {
        server.send(200, "application/json", recData);
      }
    });
  
    // ---------------------------------------------------------------------
    // GET /recordedData/enclosure/${}/current
    // ---------------------------------------------------------------------
    server.on(UriRegex("^/recordedData\\/enclosure\\/([0-9]+)\\/current$"), []() {
      String enclosure = server.pathArg(0);
      String recData = getCurrentEnclosureData(enclosure.toInt());
      if(recData == "error") {
        server.send(400, "text/plain", "Invalid line number");
      }
      else {
        server.send(200, "application/json", recData);
      }
    });
  
    // ---------------------------------------------------------------------
    // GET /enclosure/${}/desiredConditions
    // ---------------------------------------------------------------------
    server.on(UriRegex("^\\/enclosure\\/([0-9]+)/desiredConditions$"), []() {
      String enclosure = server.pathArg(0);
      server.send(200, "application/json", getFullDayEnclosureDesiredData(enclosure.toInt()));
    });

    server.begin();
  }
}

// -------------------------------------------------------------

bool wifiConnect(void) { 
  WiFi.mode(WIFI_STA);
  WiFi.begin(readStoredSSID(), readStoredPassword());

  // Wait for connection. Try connecting 10 times in 1s intervals.
  // Timeout after 10s.
  for(int i = 0; i < 10; i++) {
    if(WiFi.status() != WL_CONNECTED) {
      delay(1000);
    }
    else {
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------

bool changeWifiCredentials(String ssid, String password) {
  if(ssid.equals("") || password.equals("")) {
    return false;
  }
  
  // Delete the file
  SD.remove(wifiCredentialsFile);

  myFile = SD.open(wifiCredentialsFile, FILE_WRITE);

  myFile.print(ssid);
  myFile.print(',');
  myFile.print(password);
  myFile.println(',');

  myFile.close();

  // Ensure the correct data is stored in EEPROM
  if(!readStoredSSID().equals(ssid) || !readStoredPassword().equals(password)) {
    return false;
  }

  WiFi.begin(readStoredSSID(), readStoredPassword());

  return true;
}

// -------------------------------------------------------------

String readStoredSSID(void) {
  String _ssid;
  char temp = 0;

  myFile = SD.open(wifiCredentialsFile);

  while(myFile.available()) {
    temp = myFile.read();
    if(temp == ',' || temp == '\n') {
      myFile.close();
      return _ssid;
    }
    else {
      _ssid += temp;
    }
  }

  myFile.close();
  return "error";
}

// -------------------------------------------------------------

String readStoredPassword(void) {
  String _password;
  char temp = 0;

  myFile = SD.open(wifiCredentialsFile);

  while(myFile.available() && myFile.read() != ',');
  while(myFile.available()) {
    temp = myFile.read();
    if(temp == ',' || temp == '\n') {
      myFile.close();
      return _password;
    }
    else {
      _password += temp;
    }
  }

  myFile.close();
  return "error";
}

// -------------------------------------------------------------

void initSD(void) {
  // CRITICAL ERROR
  // Cannot connect to SD card
  // DO NOT CONTINUE UNTIL CONNECTION IS MADE
  // Alert the MCU every 5 seconds until the problem is fixed
  while(!SD.begin(4)) {
    Serial.println("XYZ,SD,");
    delay(5000);
  }

  if(!SD.exists(enclosureSet1)) {
    setEnclosureDesiredData(1, defaultDesiredConditions);
  }
  if(!SD.exists(enclosureSet2)) {
    setEnclosureDesiredData(2, defaultDesiredConditions);
  }
  if(!SD.exists(enclosureSet3)) {
    setEnclosureDesiredData(3, defaultDesiredConditions);
  }
}

// -------------------------------------------------------------

void parseMCUData(String data) {
  String code = data.substring(0,2);

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is requesting desired conditions for an enclosure
  if(code.equals("A0")) {
    int enclosure = data.substring(3,4).toInt();
    int dayHour = data.substring(5).toInt();

    // Send error message to MCU if invalid enclosure number or
    // day hour is sent. 
    if(enclosure < 1 || enclosure > 3 || dayHour < 0 || dayHour > 23) {
      Serial.println("A0,XYZ,");
    }
    // Send desired conditions to the MCU
    else {
      JSONVar obj = getEnclosureDesiredData(enclosure, dayHour);

      Serial.print("A0,");
      Serial.print((int)obj["temperature"]);
      Serial.print(',');
      Serial.print((int)obj["humidity"]);
      Serial.print(',');
      Serial.print((int)obj["luminosity"]);
      Serial.println(',');
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is sending current conditions of an enclosure to record
  else if(code.equals("A1")) {
    // Parse enclosure number
    int enclosure = data.substring(3,4).toInt();

    // Parse enclosure current conditions
    String currentConditionsData = data.substring(5);

    // Verify that the data was stored successfully
    if(recordCurrentEnclosureData(enclosure, currentConditionsData)) {
      Serial.println("A1,ABC,");
    }
    else {
      Serial.println("A1,XYZ,");
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is requesting the current unix timestamp
  else if(code.equals("A2")) {
    Serial.print("A2,");
    Serial.print(unix_epoch);
    Serial.println(',');
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is requesting the IP address of the ESP
  else if(code.equals("A3")) {
    if(WiFi.status() == WL_CONNECTED) {
      Serial.print("A3,");
      Serial.print(WiFi.localIP());
      Serial.println(',');
    }
    else {
      Serial.println("A3,XYZ,");
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is sending new desired conditions for an enclosure
  else if(code.equals("A4")) {
    // Parse enclosure number
    int enclosure = data.substring(3,4).toInt();

    // Parse enclosure new desired conditions
    String newDesiredData = data.substring(5);

    // Verify that the data was stored successfully
    if(setEnclosureDesiredData(enclosure, newDesiredData)) {
      Serial.println("A4,ABC,");
    }
    // Send the error code to the MCU if unsuccessful
    else {
      Serial.println("A4,XYZ,");
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is requesting the state of the ESP WiFi connection
  else if(code.equals("A5")) {
    // ESP is connected to WiFi
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("A5,ABC,");
    }
    // ESP is not connected to WiFi
    else {
      Serial.println("A5,XYZ,");
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is sending new WiFi credentials
  else if(code.equals("A6")) {
    int passwordStart = 0;
    String newSSID, newPassword;

    // Parse new SSID
    for(int i = 3; data[i] != ','; i++) {
      newSSID += data[i];
      passwordStart = i + 2;
    }

    // Parse new password
    for(int j = passwordStart; data[j] != ','; j++) {
      newPassword += data[j];
    }

    // Set new WiFi credentials
    if(changeWifiCredentials(newSSID, newPassword)) {
      Serial.println("A6,ABC,");
    }
    // Send error message to MCU if data not store correctly
    else {
      Serial.println("A6,XYZ,");
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // MCU is requesting the ESP API key
  else if(code.equals("A7")) {
    Serial.print("A7,");
    Serial.print(API_KEY);
    Serial.println(',');
  }


  else if(code.equals("A8")) {
    Serial.print("A8,");
    Serial.print(readStoredSSID());
    Serial.println(",");
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Invalid code
  else {
    Serial.print("XX,XYZ,CODE,");
    Serial.println(code);
  }
}

// -------------------------------------------------------------

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n' || inChar == '\0') {
      stringComplete = true;
    }
  }
}

// -------------------------------------------------------------

void setup() {
  Serial.begin(9600);

  Serial.println('\0');

  Serial.println(html.length());

  // Initialize SD card module
  // This function will not return until a connection is made.
  initSD();
  
  wifiConnect();

  initAPI();
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);  

//  getWebPage();
}

// -------------------------------------------------------------

void loop() {
  String tempData;

  // TODO: check status of SD card module. Send critical error to
  // MCU if connection failure.

  if(WiFi.status() == WL_CONNECTED) {
    server.handleClient();
    timeClient.update();
    unix_epoch = timeClient.getEpochTime();
  }
  else {
    // Clear the timestamp when not connected to WiFi to avoid
    // sending incorrect data to the MCU
    unix_epoch = 0;
  }
  
  // String is complete when a newline arrives:
  if (stringComplete) {
    parseMCUData(inputString);
    
    // clear the string
    inputString = "";
    stringComplete = false;
  }
}
