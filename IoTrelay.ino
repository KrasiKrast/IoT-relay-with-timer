/*
 * ESP8266 IoT web Relay 
 * Designed by Krasimir Krastev 2020
 * used as IoT bathroom fan to be able for automatic shut down 
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <EEPROM.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>  //Ticker Library

//-------------------------------------------------------------------------------------------
// GPIO#0 is for Adafruit ESP8266 HUZZAH board. Your board LED might be on 13.
const int LEDPIN = 2;
const int RELAY = 0;
// Current LED status
bool boLEDStatus = false;


// Commands sent through Web Socket
const char FANON[] = "fanon";
const char FANOFF[] = "fanoff";
//-------------------------------------------------------------------------------------------
 
static const char ssid[] = "HomeNet2g";
static const char password[] = "33006655KhKadmin";

int timeDelay = 3; //minutes
int timeSeconds = 0;
MDNSResponder mdns;

static void writeLED(bool);
static void ICACHE_RAM_ATTR onTimerISR();

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <title>ESP8266 WebSocket Demo</title>
        <style>
        "body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; text-align: center}"
        </style>
        <script>
        var websock;
        function start() {
          websock = new WebSocket('ws://' + window.location.hostname + ':81/');
          websock.onmessage = function(evt) {            
            var e = document.getElementById('ledstatus');            
            if (evt.data === 'fanon') {
              e.style.color = 'green';
            }
            else if (evt.data === 'fanoff') {
              e.style.color = 'red';
            }
            else {              
              var t = document.getElementById('sliderVreme');
              t.value = evt.data;
              document.getElementById("timeLabel").innerHTML = t.value;
              
            }
          };
        }
        function buttonclick(b) {
          websock.send(b.id);
        }
        function sliderMove(s){
            document.getElementById("timeLabel").innerHTML = s.value;
            websock.send(s.value);
          }
        </script>
        </head>
        <body onload="javascript:start();">
                  <h2><button style="font-size: 16pt" onclick="goBack()"> <<Назад</button> -IoT Реле Вентилатор Етаж 2-</h2>
                  <div id="ledstatus" style="text-align: center; font-size: 16pt"><b>ВЕНТИЛАТОР БАНЯ - ЕТАЖ 2</b>
                    <p> </p>                    
                      <button id="fanon"  type="button" style="height:200px; width:65%; font-size: 24pt; margin: auto" onclick="buttonclick(this);">ВКЛЮЧИ</button>                     
                    <p> </p>                    
                      <button id="fanoff" type="button" style="height:200px; width:65%; font-size: 24pt; margin: auto" onclick="buttonclick(this);">ИЗКЛЮЧИ</button>
                    <p> </p>                    
                    <p style="font-size: 16pt">Време:</p>0
                      <pstyle="font-size: 16pt"><span id="timeLabel"></span>[мин.]</p>
                      <input  id="sliderVreme" type="range" min="1" max="60" value="15" style="height:180px;width:90%; margin: auto" onchange="sliderMove(this)">    
                    </div>
        </body>
        </html>
)rawliteral";

//*********************************************************************************************
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    /*
     case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
     */
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        //Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send the current LED status
        if (boLEDStatus) {
          webSocket.sendTXT(num, FANON, strlen(FANON));
          char buffer [4];
          itoa(timeDelay, buffer, 10);
          webSocket.sendTXT(num, buffer, strlen(buffer));
        }
        else {
          webSocket.sendTXT(num, FANOFF, strlen(FANOFF));
          char buffer [4];
          itoa(timeDelay, buffer, 10);
          webSocket.sendTXT(num, buffer, strlen(buffer));
        }
      }
      break;
    case WStype_TEXT:
      //Serial.printf("[%u] get Text: %s\r\n", num, payload);
      //========================================================= FAN ON
      if (strcmp(FANON, (const char *)payload) == 0) {        
        timeSeconds = 0;
        digitalWrite(LEDPIN,true);
        boLEDStatus = true;
        timer1_write(300000);
        
      }
      //======================================================== FAN OFF
      else if (strcmp(FANOFF, (const char *)payload) == 0) {
        timeSeconds=(timeDelay*60) + 1; 
        digitalWrite(LEDPIN,false);    
        boLEDStatus = false;
      }
      else {
        String timevalue = String((const char *)payload);
        //Serial.println(timevalue);
        timeDelay = timevalue.toInt();
        eeWriteInt(0, timeDelay);
        
      }
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
      break;
    /*
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      //hexdump(payload, length);

      // echo data back to browser
     // webSocket.sendBIN(num, payload, length);
      break;
      */
  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

static void writeLED(bool FANon)
{
  boLEDStatus = FANon;
}
//**************************** EEPROM write
void eeWriteInt(int pos, int val) {
    EEPROM.begin(512);
    byte* p = (byte*) &val;
    EEPROM.write(pos, *p);
    EEPROM.write(pos + 1, *(p + 1));
    EEPROM.write(pos + 2, *(p + 2));
    EEPROM.write(pos + 3, *(p + 3));
    EEPROM.commit();
}
//**************************** EEPROM read
int eeReadInt(int pos) {
  EEPROM.begin(512);
  int val;
  byte* p = (byte*) &val;
  *p        = EEPROM.read(pos);
  *(p + 1)  = EEPROM.read(pos + 1);
  *(p + 2)  = EEPROM.read(pos + 2);
  *(p + 3)  = EEPROM.read(pos + 3);
  return val;
}
//************************** TIMER INTERUPT
//=======================================================================
static void ICACHE_RAM_ATTR onTimerISR(){
    timeSeconds++;
    int TargetSeconds = timeDelay*60;
    if(timeSeconds > TargetSeconds) 
      {
        digitalWrite(LEDPIN,false);
        boLEDStatus = false;
      }
    else timer1_write(300000);
}
//=================================================== SETUP function  .........1)
void setup()
{
  pinMode(LEDPIN, OUTPUT);  
  pinMode(RELAY, OUTPUT);
  digitalWrite(LEDPIN,true);  //Toggle LED Pin
  digitalWrite(RELAY,true);  //Toggle LED Pin
  boLEDStatus = true;

  timeDelay = eeReadInt(0);
  //Initialize Ticker every 0.5s
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
    timer1_write(300000); //5s in us

  WiFiMulti.addAP(ssid, password);

  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  if (mdns.begin("espWebSock", WiFi.localIP())) {
    mdns.addService("http", "tcp", 80);
    mdns.addService("ws", "tcp", 81);
  }
  else {
    delay(10);
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  
}

void loop()
{
  webSocket.loop();
  server.handleClient();
}
