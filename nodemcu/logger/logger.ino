 #include <Ethernet.h>

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <SD.h>
#include <SDFS.h>
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val){
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val){
  return( (val/16*10) + (val%16) );
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year){
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.write(0x00); //Start
  Wire.endTransmission();
}
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

String getTime(){
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);

  char tbs[32];

  sprintf(tbs, "20%02d-%02d-%02d %02d:%02d:%02d", year, month, dayOfMonth, hour, minute, second);

  return String(tbs);
}

int shift1Hours = 6;
int shift1Minutes = 30;
int shift2Hours = 16;
int shift2Minutes = 30;



void shiftsFromFile() {
  if (SD.begin(D0) && SD.exists("shifts.txt")) {
   File dataFile = SD.open("shifts.txt", FILE_READ);
      
    int index = 0;
    char hrs[5];
    char minutes[5];
    int line = 0;
    
    int next = dataFile.read();
  
    while (next != -1)
    {
      bool isMin = false;
      char nextChar = (char) next;
      if (nextChar == '\n')
      {
        minutes[index] = '\0';
        if (line <2) {
          String hrsStr = String(hrs);
          hrsStr.replace("\r","");
          hrsStr.replace("\n","");

          String minStr = String(minutes);
          minStr.replace("\r","");
          minStr.replace("\n","");

          if (line == 0) {
            shift1Hours = hrsStr.toInt();
            shift1Minutes = minStr.toInt();
          } else if (line == 1) {
            shift2Hours = hrsStr.toInt();
            shift2Minutes = minStr.toInt();
          }
        }
        index = 0;
        line += 1;
      }
      else
      {
        if (nextChar == ':') {
          isMin = true;
          hrs[index] = '\0';
          index = 0;
        } else {
          if (isMin) {
            minutes[index] = nextChar;
          } else {
            hrs[index] = nextChar;
          }
          index += 1;
        }
      }
  
      next = dataFile.read();
    }
  
    dataFile.close();
   } else {
     shift1Hours = 6;
     shift1Minutes = 30;

     shift2Hours = 16;
     shift2Minutes = 30;
   }
}

String getShiftId() {
  shiftsFromFile();

  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);

  int hm1 = shift1Hours * 60 + shift1Minutes;
  int hm2 = shift2Hours * 60 + shift2Minutes;

  int curr = hour * 60 + minute;

  if (curr>=hm1 && curr < hm2) return "AM";

  return "PM";
}

String getDateAndTimeCsv(){
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);

  char tbs[32];

  sprintf(tbs, "%02d/%02d/20%02d,%02d:%02d:%02d",  month, dayOfMonth,year, hour, minute, second);

  return String(tbs);
}

String getDateFileName(){
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);

  char tbs[32];

  sprintf(tbs, "20%02d%02d%02d_%02d%02d%02d", year, month, dayOfMonth, hour, minute, second);

  return String(tbs);
}


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

String idleString =  "";
unsigned long partStarted = millis();
unsigned long partProcessed =  millis();
unsigned long procTime = 0;
unsigned long idleTime = 0;
unsigned long partCount = 0;
unsigned long maxIdle = 0;
unsigned long maxProc = 0;
unsigned long minIdle = 999999;
unsigned long minProc = 999999;

unsigned long prevStateChangeMs = 0;

bool prevState = false;
bool hasSD = false;
bool newFile = true;
String fileName = "20000101_000000_data.csv";
String fileDate = "20000101_000000";
String currShiftId = "";

String files[500]; 
int fileIdx = 0;

IPAddress local_IP(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

String ssid = "";
int repeatDelay = 500;
String delayString = "500";

bool enableSerial = false;


bool readDelay() {
   if (SD.begin(D0) && SD.exists("delay.txt")) {
   File dataFile = SD.open("delay.txt", FILE_READ);
      
    int index = 0;
    char stringArray[20];
    int line = 0;
    
    int next = dataFile.read();
  
    while (next != -1)
    {
     
      char nextChar = (char) next;
      if (nextChar == '\n')
      {
        stringArray[index] = '\0';
        if (line == 0) {
          delayString = String(stringArray);
          delayString.replace("\r","");
          delayString.replace("\n","");
          repeatDelay = delayString.toInt();
        }
        index = 0;
        line += 1;
      }
      else
      {
        stringArray[index] = nextChar;
        index += 1;
      }
  
      next = dataFile.read();
    }
  
    dataFile.close();
   }
}

bool connectWiFi() {
  bool connected = false;
  if (SD.begin(D0) && SD.exists("wifi.txt")) {
      ssid = "";
      String pwd = "";
      File dataFile = SD.open("wifi.txt", FILE_READ);
      
        int index = 0;
        char stringArray[500];
        int line = 0;
        
        int next = dataFile.read();

        while (next != -1)
        {
         
          char nextChar = (char) next;
          if (nextChar == '\n')
          {
            stringArray[index] = '\0';
            if (line == 0) {
              ssid = String(stringArray);
              ssid.replace("\r","");
              ssid.replace("\n","");
            }
            if (line == 1) {
              pwd = String(stringArray);
              pwd.replace("\r","");
              pwd.replace("\n","");
            }
            index = 0;
            line += 1;
          }
          else
          {
            stringArray[index] = nextChar;
            index += 1;
          }
  
          next = dataFile.read();
        }
  
        dataFile.close();
        serialPrintln(ssid);
        serialPrintln(pwd);
        if ( ssid.length() >0 && pwd.length() >0) {
           WiFi.begin(ssid, pwd);
           int attempts = 30; 
           while (WiFi.status() != WL_CONNECTED && attempts >0) {
             delay(500);
             serialPrint(".");
             attempts -=1;
          }
          if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            serialPrint("Connected to network. IP address: ");
            //serialPrintln(WiFi.localIP());
          }
        }
    }
    

  if (!connected) {
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP("LAP0002", "logger123", 1, false, 4);
    serialPrint("Own network. IP address: ");
    //serialPrintln(WiFi.softAPIP());
  }
     
}

void setup() {
  if (enableSerial) {
    Serial.begin(115200);
  }
  // Initialize the output variables as outputs
  pinMode(A0, INPUT);  

  Wire.begin();
  
  connectWiFi();
  readDelay();
  server.begin();
}

void resetStats(String shiftId) {
   currShiftId = shiftId;
   newFile = true;
   previousTime = 0;
   idleString =  "";
   partStarted = millis();
   partProcessed =  millis();
   procTime = 0;
   idleTime = 0;
   partCount = 0;
   maxIdle = 0;
   maxProc = 0;
   minIdle = 999999;
   minProc = 999999;
}

int stateInt = 0;

unsigned long lastStats = 0;
int lastPartCount = 0;

int smoothData[5];

bool smooth(bool val){
  int sum = 0;
  for (int i=1; i<5; i++) {
    smoothData[i-1] = smoothData[i];
    sum += smoothData[i];
    if (val) {
      sum += 1;
      smoothData[4] = 1;
    } else {
      smoothData[4] = 0;
    }
  }
  return sum/5.0 >= 0.5;
}

void loop(){  

   stateInt = analogRead(A0);  //0-1024
   bool state = smooth(stateInt>768);

   //Reset file for a new shift
   String shiftId = getShiftId();

   //reset stats
   if (currShiftId != shiftId) {
     resetStats(shiftId);
   }
  
  //SD Card
  if (SD.begin(D0)) {
    hasSD = true;
    if (newFile) {
      setFileName();
      newFile = false;
    }
  }
  else {
    hasSD = false;
  }

  //Do not allow changes more often than 0.5 sec  
  unsigned long now = millis();
  if ((now-prevStateChangeMs)>repeatDelay) {
    if (prevState!=state) {
      prevStateChangeMs = now;
      if (state) {
        fnPartStarted();
      }
      else {      
        fnPartProcessed();
      }
      prevState = state;
    }
  }

  WiFiClient client = server.available();   // Listen for incoming clients 
  
  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && (currentTime - previousTime <= timeoutTime)) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            String url = header;
            int stIdx = url.indexOf("GET")+4;
            int endIdx = url.indexOf(" HTTP/1.1");
            url = url.substring(stIdx, endIdx);
                       
            if (url.indexOf("/Download")>=0) {
               String fName = url;
               fName.replace("/Download/","");
               downloadFile(client, fName);
            }
            else if (url.indexOf("/Settings")>=0) {
              renderSettings(client);
            }
            else if (url.indexOf("/SetDelay")>=0) {
              setDelay(url);
              renderPage(client);
            }
            else if (url.indexOf("/SetShifts")>=0) {
              setShifts(url);
              renderPage(client);
            }
            else if (url.indexOf("/SetTime")>=0) {
              setTime(url);
              renderPage(client);
            }
            else if (url.indexOf("/Files")>=0) {
              renderFiles(client);
            }
            else if (url.indexOf("/SetWifi")>=0) {
              setWifi(url);
              renderPage(client);
            }
            else {
              renderPage(client);
            }
            
             // Break out of the while loop
             break;
          } else { // if you got a newline, then clear currentLine
             currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }

  //Write statistics every minute (if more parts has processed)
  int currMillis = millis();
  if ((currMillis-lastStats>60000) && (partCount!=lastPartCount)) {
    lastStats = currMillis;
    lastPartCount = partCount;
    writeSummary();
  }
}

void setDelay(String url) {
  if (hasSD) { 
    //if (SD.exists("delay.txt")) {
    //  SD.remove("delay.txt");
    //}
    File dataFile = SD.open("delay.txt", sdfat::O_CREAT | sdfat::O_TRUNC | sdfat::O_WRITE);
    if (dataFile) {
      String val = url;
      val.replace("/SetDelay?delay=","");
      
      dataFile.println(val);
      dataFile.close();
    }
  }
}

void setShifts(String url) {
  if (hasSD) { 
    File dataFile = SD.open("shifts.txt", sdfat::O_CREAT | sdfat::O_TRUNC | sdfat::O_WRITE);
    if (dataFile) {
      String val = url;
      val.replace("/SetDelay?shift1=","");
      val.replace("shift2=","");
      int idx = val.indexOf("&");
      String val1 = val.substring(0, idx-1);
      String val2 = val.substring(idx+1);

      dataFile.println(val1);
      dataFile.println(val2);
      dataFile.close();
    }
  }
}

void setWifi(String url) {
  if (hasSD) { 
    //if (SD.exists("wifi.txt")) {
    //  SD.remove("wifi.txt");
    //}
    File dataFile = SD.open("wifi.txt", sdfat::O_CREAT | sdfat::O_TRUNC |sdfat::O_WRITE);
    if (dataFile) {
      String val = url;
      val.replace("/SetWifi?ssid=","");
      val.replace("&pwd=","\n");
      
      dataFile.println(val);
      dataFile.close();
    }
  }
}

void setTime(String url) {
  String val = url;
  val.replace("/SetTime?date=","");

  byte year = val.substring(2,4).toInt();
  byte month = val.substring(5,7).toInt();
  byte day = val.substring(8,10).toInt();
  byte hour = val.substring(11,13).toInt();
  byte minute = val.substring(16,18).toInt();
  byte second = 0;

  setDS3231time(second,minute,hour,1,day,month,year);
}

void setFileName() {

  fileDate = getDateFileName();
  fileName = fileDate+"_data.csv";

}

void listFiles() {
  if (hasSD) {
    fileIdx = 0;
    File root = SD.open("/");
    while (true) {
  
      File entry =  root.openNextFile();
      if (! entry) {
        // no more files
        break;
      }
      String entryName = entry.name();
      if (entryName.endsWith(".csv")) {
        files[fileIdx] = entryName;
        fileIdx++;
      }
      entry.close();
    }
    root.close();
  }
}


String timeStarted = "";

void fnPartStarted() {
    partStarted = millis();
    unsigned long currRange = partStarted - partProcessed;
    idleTime +=  currRange;
    if (currRange > maxIdle) {
      maxIdle = currRange;
    }
    if (currRange < minIdle) {
      minIdle = currRange;
    }

    idleString = String(currRange/1000.0);
    timeStarted = getDateAndTimeCsv();
}
 

String msToTimeString(unsigned long durationMs) {
  unsigned long allSeconds=durationMs/1000;
  int runHours= allSeconds/3600;
  int secsRemaining=allSeconds%3600;
  int runMinutes=secsRemaining/60;
  int runSeconds=secsRemaining%60;
  int runMills = durationMs - (allSeconds*1000);

  char buf[21];
  sprintf(buf,"%02d:%02d:%02d.%03d",runHours,runMinutes,runSeconds, runMills);
  return String(buf);
}

void fnPartProcessed() {
  partProcessed = millis();
  unsigned long currRange = partProcessed - partStarted;
  String timeProcessed = getDateAndTimeCsv();
  procTime += currRange;
  if (currRange > maxProc) {
    maxProc = currRange;
  }
  if (currRange < minProc) {
    minProc = currRange;
  }   
  partCount +=1;

  if (hasSD) { 
    bool firstLine = !SD.exists(fileName);
    File dataFile = SD.open(fileName, sdfat::O_CREAT | sdfat::O_APPEND | sdfat::O_WRITE);
    if (dataFile) {
      if (firstLine) {
        dataFile.println("Part#,Processing (sec),Start Date,Start Time, End Date, End Time, Idle (sec)");
      }
      
      dataFile.println(String(partCount)+","+String(currRange/1000.0)+ "," + timeStarted + "," + timeProcessed +"," + idleString );
      dataFile.close();
    }
  }  
}


void writeSummary() {
  if (hasSD) { 
    String statsFile =  fileDate+"_summary.csv";
    //if (SD.exists(statsFile)) {
    //  SD.remove(statsFile);
    //}
    
    File dataFile = SD.open(statsFile, sdfat::O_CREAT | sdfat::O_TRUNC | sdfat::O_WRITE);
    if (dataFile) {
      dataFile.println("Parts processed,"+String(partCount));
      if (idleTime+procTime !=0) {
        dataFile.println("Idle time (minutes),"+String(idleTime/(1000.0*60.0))+","+String((float(idleTime)/float(idleTime+procTime))*100.0)+"%");
        dataFile.println("Processing time (minutes),"+String(procTime/(1000.0*60.0))+","+String((procTime/float(idleTime+procTime))*100.0)+"%");
      } 
      else {
        dataFile.println("Idle time (minutes),"+String(idleTime/(1000.0*60.0)));
        dataFile.println("Processing time (minutes),"+String(procTime/(1000.0*60.0)));
      }
      dataFile.println("Total time (minutes),"+String((idleTime+procTime)/(1000.0*60.0)));
      dataFile.println("Longest idle time (sec),"+String(maxIdle/(1000.0)));
      if (minProc!=999999) { 
        dataFile.println("Shortest idle time (sec),"+String(minIdle/(1000.0))); 
      }
      else {
        dataFile.println("Shortest idle time (sec),0"); 
      }
      dataFile.println("Longest processing time (sec),"+String(maxProc/(1000.0))); 
      if (minProc!=999999) {
        dataFile.println("Shortest processing time (sec),"+String(minIdle/(1000.0)));
      }
      else {
        dataFile.println("Shortest processing time (sec),0");
      }
      if (partCount > 0) { 
        dataFile.println("Average processing time (sec),"+String((procTime/(1000.0))/float(partCount)));
      }
      else {
        dataFile.println("Average processing time (sec),0");
      }
      
      dataFile.close();
    }
  }  
}



void downloadFile(WiFiClient client, String fName) {
      // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
      // and a content-type so the client knows what's coming, then a blank line:
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/csv");
      client.println("Connection: close");
      client.println();

      if (hasSD && SD.exists(fName)) {
        File dataFile = SD.open(fName, FILE_READ);
      
        int index = 0;
        char stringArray[500];
  
        int next = dataFile.read();
  
        while (next != -1)
        {
         
          char nextChar = (char) next;
          if (nextChar == '\n')
          {
            stringArray[index] = '\0';
            client.print(stringArray);
            index = 0;
          }
          else
          {
            stringArray[index] = nextChar;
            index += 1;
          }
  
          next = dataFile.read();
        }
  
        dataFile.close();
      }
      // The HTTP response ends with another blank line
      client.println();
}


void addHeader(WiFiClient client) {
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
      // and a content-type so the client knows what's coming, then a blank line:
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      
      // Display the HTML web page
      client.println("<!DOCTYPE html><html>");
      client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      client.println("<link rel='icon' href='data:;base64,iVBORw0KGgo='>");
      // CSS to style the on/off buttons 
      // Feel free to change the background-color and font-size attributes to fit your preferences
      client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
      client.println("form { display: flex;   flex-direction: column;   align-items: flex-end; max-width: 300px; align-content:center; }");
      client.println("ul.nav li{float: right;}");
      client.println("ul.nav{list-style-type: none;margin: 0;padding: 0; overflow: hidden; background-color: #333;}");
      client.println("ul.nav div {float: left; text-align: center; padding: 14px 16px;}");
      client.println("ul.nav li{float: right;} .logo {color: white; text-decoration: none;}");
      client.println("ul.nav div {color: white; vertical-align:middle;}");
      client.println("ul.nav li a {display: block;color: white; text-align: center; padding: 14px 16px; text-decoration: none;}");
      client.println("ul.nav li a:hover { background-color: #111;}");
      
      client.println("body { width: 100vw;  height: 100vh;  margin: 0; } .container { display: grid; align-items: stretch; grid-template-rows: min-content auto; width: 100vw;  height: 100vh;}");
      client.println("div#stats {background-color:lightgray;}  #stats li {text-align: left; }");

      client.println("</style></head>");
}

void renderMenu(WiFiClient client) {
      // Web Page Heading   
      client.println("<body style='margin:0px;'><div class='container'>");
      client.println("<div style='grid-column-start: 1; grid-column-end: 1;'><ul class='nav'><div><a href='/' class='logo'>Logger</a></div><li><a id='lnkDownload' href='/Files'>Files</a></li><li><a id='lnkSettings' href='/Settings'>Settings</a></li>");
      client.println("</ul></div>");
}

void renderPage(WiFiClient client) {
      addHeader(client);

      renderMenu(client);

      client.println("<div id=\"stats\"><ul>");
      client.println("<li>Parts processed: "+String(partCount)+";</li>");
      if (idleTime+procTime !=0) {
        client.println("<li>Idle time: "+String(idleTime/(1000.0*60.0))+" minutes ("+String((float(idleTime)/float(idleTime+procTime))*100.0)+"%);</li>");
      }
      else {
        client.println("<li>Idle time: "+String(idleTime/(1000.0*60.0))+" minutes;</li>");
      }
      if (idleTime+procTime !=0) {
        client.println("<li>Processing time: "+String(procTime/(1000.0*60.0))+" minutes ("+String((procTime/float(idleTime+procTime))*100.0)+"%)</li>");
      }
      else {
         client.println("<li>Processing time: "+String(procTime/(1000.0*60.0))+" minutes;</li>");
      }
      client.println("<li>Total time: "+String((idleTime+procTime)/(1000.0*60.0))+" minutes;</li>");
      client.println("<li>Longest idle time: "+String(maxIdle/(1000.0))+" sec; </li>");
      if (minProc!=999999) {
        client.println("<li>Shortest idle time: "+String(minIdle/(1000.0))+" sec; </li>");
      }
      else {
        client.println("<li>Shortest idle time: 0 sec; </li>");
      } 
      client.println("<li>Longest processing time: "+String(maxProc/(1000.0))+" sec; </li>");
      if (minProc!=999999) {
        client.println("<li>Shortest processing time: "+String(minProc/(1000.0))+" sec; </li>");
      }
      else {
         client.println("<li>Shortest processing time: 0 sec; </li>");
      } 
      if (partCount > 0) { 
        client.println("<li>Average processing time: "+String((procTime/(1000.0))/float(partCount))+" sec; </li>");    
      }
      else {
        client.println("<li>Average processing time: 0 sec; </li>");   
      }
      if (hasSD) { 
        client.println("<li>SD Card : Available </li>");    
      }
      else {
        client.println("<li style='text-color:red;'>SD Card: Not Found </li>");   
      }
      client.println("<li>File Name: "+fileName+"</li>");   
      client.println("<li>Updated at: "+getTime()+"</li>");  
      if (prevState) { 
        client.println("<li>State: Processing</li>");    
      }
      else {
        client.println("<li>State: Idle</li>");   
      }
      client.println("<li>Shift ID: "+currShiftId+"</li>");
      client.println("</ul><hr/>");  

      client.println("</div>");

      
      client.println("<script language='javascript'> var millisBase ="+String(millis())+";");   
      client.println("const offsetMs = (new Date()).getTimezoneOffset() * 60 * 1000; const baseDate = Date.now();");
      client.println("function millsToDateString(mills) { return (new Date( baseDate - (millisBase-mills) -offsetMs)).toISOString().slice(0, 23).replace('T', ' '); }; ");
      client.println(" document.getElementById('files').onchange = function(){var e = document.getElementById('files');   var file = e.options[e.selectedIndex].value; ");
      client.println(" var download = document.getElementById('lnkDownload'); download.setAttribute('href', '/Download/'+file); download.setAttribute('download', file); ");
      client.println(" var downloadStats = document.getElementById('lnkDownloadStats'); downloadStats.setAttribute('href', '/Download/summary_'+file); downloadStats.setAttribute('download', 'summary_'+file); }; ");
      client.println("function toDateFormat(mills) { return (new Date(mills)).toISOString().slice(11, 23); }; ");
   
      client.println("</script></body></html>");
      
      // The HTTP response ends with another blank line
      client.println();
}


void renderFiles(WiFiClient client) {
      addHeader(client);

      renderMenu(client);

      client.println("<div id=\"stats\"><ul>"); 

      client.println("<h3>Download</h3>");

      listFiles();
      client.println("<ul>");
      for (int i=0; i<fileIdx; i++) {
        String selected = "";
        if (files[i] == fileName) {
          selected = " selected";
        }
        client.println("<li><a id='lnkDownload' href='/Download/"+files[i]+"' target='_blank' download='"+files[i]+"'>"+files[i]+"</a></li>");
      }
      client.println("</ul></div>");
   
      client.println("</body></html>");
      
      // The HTTP response ends with another blank line
      client.println();
}

void serialPrintln(String text) {
  if (enableSerial) {
    Serial.println(text);
  }
}

void serialPrint(String text) {
  if (enableSerial) {
    Serial.print(text);
  }
}

void renderSettings(WiFiClient client) {

      addHeader(client);
      
      renderMenu(client);

      client.println("<div id=\"stats\">");
      client.println("<h4 style='color:red;'>To apply settings please click Apply and restart the device.</h4>");
      client.println("<h3>WiFi Settings</h3><p style='text-align:right;'><form method='GET' action='/SetWifi'>");
      client.println("<p>SSID: <input type='text' style='display:inline;' id='ssid' name='ssid' value='"+ssid+"'/></p>");
      client.println("<p>Password: <input type='text' style='display:inline;' id='pwd' name='pwd'/></p>");
      client.println("<input type='submit' value='Apply'></form></p>");

      client.println("<h3>Delay Settings</h3><p style='text-align:right;'><form method='GET' action='/SetDelay'>");
      client.println("<p>Repeat delay ms: <input type='text' style='display:inline;' id='delay' name='delay' value='"+String(repeatDelay)+"'/></p>");
      client.println("<input type='submit' value='Apply'></form></p>");

      client.println("<h3>Time Settings</h3><p  style='text-align:right;'><form method='GET' action='/SetTime'>");
      client.println("<p>Date: <input type='datetime-local' style='display:inline;' id='date' name='date'/></p>");
      client.println("<input type='submit' value='Apply'></form></p>");
      client.println("</div>");

      client.println("<h3>Shifts</h3><p  style='text-align:right;'><form method='GET' action='/SetShifts'>");
      client.println("<p>Shift 1: <input type='time' style='display:inline;' id='shift1' name='shift1'/></p>");
      client.println("<p>Shift 2: <input type='time' style='display:inline;' id='shift2' name='shift2'/></p>");
      client.println("<input type='submit' value='Apply'></form></p>");
      client.println("</div>");

      client.println("</body></html>");
      
      // The HTTP response ends with another blank line
      client.println();
}
