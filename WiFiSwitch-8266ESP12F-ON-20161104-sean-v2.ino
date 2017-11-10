/*********
 20161012 Create by IT Andy Chin 
          NodeMCU with Wifi8266 for Web Power Control to the IPCamera
 20161019 Modify for Relay of Low-level Start up  
 20161101 Modify for SSID Password DeivceID Not Fixed
 20161104 add software reset and ap+client mode function      //tag sean
*********/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP.h>
#include <EEPROM.h>

MDNSResponder mdns;

// Replace with your network credentials
//const char* ssid = "MIS-16";  //for test
//const char* ssid = "MIS_4";  //for 109
//const char* ssid = "RTK 11n AP 03"; //for 201,202,203,204
//const char* ssid = "RTK 11n AP 01";  //for 101,102
//const char* password = "T69924056onic";
const char* key = "magtonic";// 家祥的參數 wait for test

const char* ssid = "MIS-16";
const char* password = "T69924056onic";
String st;
String content;
int statusCode;

ESP8266WebServer server(8085);

String Relay_Type="LOW";      //低電頻觸發 NC=HIGH  NO=LOW 
String Relay_Connect="NC";    //低電頻觸發 NC=HIGH  NO=LOW 
String Button_ON="ON";
String Button_OFF="OFF";
//int DeviceNo=101;              //電源控制設備編號,等同IP位置
String webPage = "";
String webPage_Response = "";

String esid;
String epass;
String eDevID;

int gpioLED_pin = 2; //Arduino Board LED
int gpio0_pin = D5;  //D5 for backup pin
int gpio2_pin = D6;  //D6 is default pin


void setup(void){
  Serial.begin(115200);
  // sean  //=================================================setup AP //call ESP8266WIFI
  delay(200);
  WiFi.softAP("APesp01","12345678");
  WiFi.mode(WIFI_AP_STA);
  
  // preparing GPIOs
  pinMode(gpioLED_pin, OUTPUT);      
  digitalWrite(gpioLED_pin, LOW);   // LOW is LED ON
  pinMode(gpio0_pin, OUTPUT);
  digitalWrite(gpio0_pin, HIGH);    // SET gpio0_pin ON
  pinMode(gpio2_pin, OUTPUT);
  digitalWrite(gpio2_pin, HIGH);    // SET gpio2_pin ON
  
  int gpio0_pin_status = digitalRead(gpio0_pin); // 讀取gpio0_pin狀態
  int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態

  //依Relay Type調整Web buttom說明文字
  if (Relay_Type=="HIGH") {
      Button_ON="OFF";
      Button_OFF="ON";
  }

  //基本Web內容
  webPage = webPage + "<a href=/ style='text-decoration: none'><h1>Magtonic Power Control System</h1></a>";
  webPage = webPage + "<p>Relay Type :"+ Relay_Type + "  " + Relay_Connect + "</p>";
  
  delay(500);
  
  EEPROM.begin(512);
  delay(10);
  WiFi.begin(ssid, password);
  Serial.println("");
  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  
  for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  Serial.print("SSID: ");
  Serial.println(esid);
  
  Serial.println("Reading EEPROM pass");
  
  for (int i = 32; i < 90; ++i)
    {
      epass += char(EEPROM.read(i));
    }
  Serial.print("PASS: ");
  Serial.println(epass); 

  Serial.println("Reading EEPROM DeviceID");
  
  for (int i = 90; i < 96; ++i)
    {
      eDevID += char(EEPROM.read(i));
    }
  Serial.print("DevID: ");
  Serial.println(eDevID);

  if ( esid.length() > 1 ) {
     WiFi.begin(esid.c_str(), epass.c_str());   

       // config static IP
          IPAddress ip(192, 1, 16, 123); // IP Address
          IPAddress gateway(192, 1, 16, 1); // set gateway   
          IPAddress subnet(255, 255, 255, 0); // set subnet mask 
          WiFi.config(ip, gateway, subnet);

          //Wait for connection
          //while (WiFi.status() != WL_CONNECTED) {
          //  delay(500);
          //  Serial.print(".");
          //}  
          Serial.println("");
          Serial.print("Connected to ");
          Serial.println(ssid);
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
  
      if (mdns.begin("esp8266", WiFi.localIP())) {
        Serial.println("MDNS responder started");
      }
  
  
  }
  else
  {
    setupAP();
  }
  


  
  
  server.on("/", [](){
    int gpio0_pin_status = digitalRead(gpio0_pin); // 讀取gpio0_pin狀態
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態
    if (gpio0_pin_status==0){
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button  style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button>"+Button_OFF+"</button></a> </p>";
    }
    if (gpio2_pin_status==0){
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button>"+Button_OFF+"</button></a> </p>";
    }
    server.send(200, "text/html", webPage_Response);
  });
  server.on("/Power14On", [](){
    digitalWrite(gpio0_pin, HIGH);
    delay(1000);
    int gpio0_pin_status = digitalRead(gpio0_pin); // 讀取gpio0_pin狀態
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態
    if (gpio0_pin_status==0){
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button  style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button>"+Button_OFF+"</button></a> </p>";
    }
    if (gpio2_pin_status==0){
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button>"+Button_OFF+"</button></a> </p>";
    }
    server.send(200, "text/html", webPage_Response);    
  });
  server.on("/Power14Off", [](){
    digitalWrite(gpio0_pin, LOW);
    delay(1000); 
    int gpio0_pin_status = digitalRead(gpio0_pin); // 讀取gpio0_pin狀態
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態
    if (gpio0_pin_status==0){
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button  style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button>"+Button_OFF+"</button></a> </p>";
    }
    if (gpio2_pin_status==0){
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button>"+Button_OFF+"</button></a> </p>";
    }    
    server.send(200, "text/html", webPage_Response);    
  });
  server.on("/Power12On", [](){
    digitalWrite(gpio2_pin, HIGH);
    delay(1000);
    int gpio0_pin_status = digitalRead(gpio0_pin); // 讀取gpio0_pin狀態
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態
    if (gpio0_pin_status==0){
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button  style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button>"+Button_OFF+"</button></a> </p>";
    }
    if (gpio2_pin_status==0){
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button>"+Button_OFF+"</button></a> </p>";
    }    
    server.send(200, "text/html", webPage_Response);    
  });
  server.on("/Power12Off", [](){
    digitalWrite(gpio2_pin, LOW);
    delay(1000);
    int gpio0_pin_status = digitalRead(gpio0_pin); // 讀取gpio0_pin狀態
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態
    if (gpio0_pin_status==0){
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button  style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage + "<p>D5 Socket #1 <a href=\"Power14On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power14Off\"><button>"+Button_OFF+"</button></a> </p>";
    }
    if (gpio2_pin_status==0){
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_OFF+"</button></a> </p>";
        }else{
        webPage_Response = webPage_Response + "<p>*D6 Socket #2 <a href=\"Power12On\"><button style='background-color:#00FF00;color:#FFFFFF;'>"+Button_ON+"</button></a>&nbsp;<a href=\"Power12Off\"><button>"+Button_OFF+"</button></a> </p>";
    }    
    server.send(200, "text/html", webPage_Response);     
  });

  //......for 手機控制用 ....................................................
  server.on("/api/powerOn", [](){
    digitalWrite(gpio2_pin, HIGH);
    digitalWrite(gpio0_pin, HIGH);
    delay(1000);
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態
    if (gpio2_pin_status==0){
        webPage_Response = Button_OFF;;
        }else{
        webPage_Response = Button_ON;
    }
       
    server.send(200, "text/plain", webPage_Response);  //回傳大寫ON  
  });
    
  server.on("/api/powerOff", [](){
    digitalWrite(gpio2_pin, LOW);
    digitalWrite(gpio0_pin, LOW);
    delay(1000);  
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態    
    if (gpio2_pin_status==0){
        webPage_Response = Button_OFF;
    }else{
        webPage_Response = Button_ON;      
    }
    server.send(200, "text/plain", webPage_Response);  //回傳大寫OFF  
  });

  server.on("/api/getStatus", [](){    
    delay(100);  
    int gpio2_pin_status = digitalRead(gpio2_pin); // 讀取gpio2_pin狀態    
    if (gpio2_pin_status==0){
        webPage_Response = Button_OFF;
    }else{
        webPage_Response = Button_ON;      
    }
    server.send(200, "text/plain", webPage_Response);  //回傳大寫ON或OFF  
  });
  //......for 手機控制用 ....................................................

  server.on("/SetUp", []() {
        //IPAddress ip = WiFi.softAPIP();
        //String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);        
        
        int n = WiFi.scanNetworks();
         st = "<select name='ssid'>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      
      st = st + "<option value='" + WiFi.SSID(i)+ "'>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</option>";
    }
  st += "</select>";
  
        webPage_Response = webPage + "<p>SSID:" + esid.c_str() ;
        webPage_Response = webPage_Response + " PWD:" + epass.c_str() ;
        webPage_Response = webPage_Response + " DevID:" + eDevID.c_str() + "</p>";
        //webPage_Response = webPage_Response  + st ;
        webPage_Response = webPage_Response  + "<p></p>";
        //webPage_Response = webPage_Response  + "<form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><label>PWD: </label><input name='pass' length=58><label>DeviceID: </label><input name='devid' length=6><input type='submit'></form>";
        webPage_Response = webPage_Response  + "<form method='get' action='setting'><label>SSID: </label>" + st + "<label>PWD: </label><input name='pass' length=58><label>DeviceID: </label><input name='devid' length=6><input type='submit'></form>";
        
        webPage_Response = webPage_Response  + "<p><form method='get' action='cleareeprom'><label>Clear EEPROM Data : </label><input type='submit'></form></p>";
        server.send(200, "text/html", webPage_Response); 
    });
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        String qdevid = server.arg("devid");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
          
          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(i, qsid[i]);
              Serial.print("Wrote: ");
              Serial.println(qsid[i]);
            }
          Serial.println("writing eeprom pass:");
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(32+i, qpass[i]);
              Serial.print("Wrote: ");
              Serial.println(qpass[i]);
            }   
          Serial.println("writing eeprom devid:");
          for (int i = 0; i < qdevid.length(); ++i)
            {
              EEPROM.write(90+i, qdevid[i]);
              Serial.print("Wrote: ");
              Serial.println(qdevid[i]);
            } 
          EEPROM.commit();
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });

    server.on("/cleareeprom", []() {
      //content = "<!DOCTYPE HTML>\r\n<html>";
      //content += "<p>Clearing the EEPROM</p></html>";
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      webPage_Response = webPage + "<p>IP:" + ipStr + "  Clearing the EEPROM ...";
      
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
      webPage_Response = webPage_Response + " OK! </p>";
      server.send(200, "text/html", webPage_Response);
    });
 //sean===================================================== software reset
  server.on("/reset",[](){
       server.send(200, "text/plain", "Reseting....");
    ESP.reset();
  });// reset
  
  server.begin();
  Serial.println("HTTP server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<li>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</li>";
    }
  st += "</ol>";
  delay(100);
  WiFi.softAP(ssid, password, 6);
  Serial.println("softap");
  //launchWeb(1);
  Serial.println("over");
}
 
void loop(void){
  server.handleClient();
} 
