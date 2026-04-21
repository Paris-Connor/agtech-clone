// AG Tech - Environmental Monitor Dashboard
// D1 Mini (ESP8266)
// DHT11: D5 (GPIO14) - Temperature & Humidity
// GY-30 (BH1750): D1 (SCL), D2 (SDA) - Light Intensity
// Soil Moisture: A0 - Soil Moisture %
// RGB LED: Red=D6 (GPIO12), Green=D7 (GPIO13), Blue=D8 (GPIO15)
// LED = status indicator (green=ok, yellow=warn, red=danger)
// Common cathode (long leg to GND)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

#include "config.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

#define RED_PIN   12  // D6
#define GREEN_PIN 13  // D7
#define BLUE_PIN  15  // D8
#define DHTPIN    14  // D5
#define DHTTYPE DHT11
#define SOIL_PIN  A0

// Soil calibration (adjust with your probe)
#define SOIL_DRY  1023  // raw value in air
#define SOIL_WET  300   // raw value in water

// Thresholds
#define TEMP_WARN_HIGH   30.0
#define TEMP_DANGER_HIGH 35.0
#define TEMP_WARN_LOW    10.0
#define TEMP_DANGER_LOW   5.0
#define HUM_WARN_HIGH    70.0
#define HUM_DANGER_HIGH  80.0
#define HUM_WARN_LOW     30.0
#define HUM_DANGER_LOW   20.0
#define LUX_WARN_LOW     500.0
#define LUX_DANGER_LOW   200.0
#define LUX_WARN_HIGH  50000.0
#define LUX_DANGER_HIGH 80000.0
#define SOIL_WARN_LOW    25.0
#define SOIL_DANGER_LOW  15.0
#define SOIL_WARN_HIGH   80.0
#define SOIL_DANGER_HIGH 90.0

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
ESP8266WebServer server(80);

#define MAX_READINGS 120
float tempHistory[MAX_READINGS];
float humHistory[MAX_READINGS];
float luxHistory[MAX_READINGS];
float soilHistory[MAX_READINGS];
int readIndex = 0;
int totalReadings = 0;
unsigned long lastRead = 0;
unsigned long lastThingSpeak = 0;
float currentTemp = 0, currentHum = 0, currentLux = 0, currentSoil = 0;
String currentStatus = "ok";
bool lightSensorOk = false;
bool soilSensorOk = false;

void setLED(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

float readSoilPercent() {
  int raw = analogRead(SOIL_PIN);
  // If reads max (1023), nothing is connected
  if (raw >= 1020) return -1;
  float pct = (float)(SOIL_DRY - raw) / (SOIL_DRY - SOIL_WET) * 100.0;
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}

String evaluateStatus(float temp, float hum, float lux, float soil) {
  // Danger checks
  if (temp >= TEMP_DANGER_HIGH || temp <= TEMP_DANGER_LOW ||
      hum >= HUM_DANGER_HIGH || hum <= HUM_DANGER_LOW) {
    return "danger";
  }
  if (lightSensorOk && (lux <= LUX_DANGER_LOW || lux >= LUX_DANGER_HIGH)) {
    return "danger";
  }
  if (soilSensorOk && (soil <= SOIL_DANGER_LOW || soil >= SOIL_DANGER_HIGH)) {
    return "danger";
  }
  // Warning checks
  if (temp >= TEMP_WARN_HIGH || temp <= TEMP_WARN_LOW ||
      hum >= HUM_WARN_HIGH || hum <= HUM_WARN_LOW) {
    return "warn";
  }
  if (lightSensorOk && (lux <= LUX_WARN_LOW || lux >= LUX_WARN_HIGH)) {
    return "warn";
  }
  if (soilSensorOk && (soil <= SOIL_WARN_LOW || soil >= SOIL_WARN_HIGH)) {
    return "warn";
  }
  return "ok";
}

void updateLED(String status) {
  if (status == "danger") {
    setLED(1023, 0, 0);
  } else if (status == "warn") {
    setLED(1023, 512, 0);
  } else {
    setLED(0, 1023, 0);
  }
}

void sendToThingSpeak() {
  if (strlen(THINGSPEAK_API_KEY) < 5) return;  // No key configured

  WiFiClient client;
  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=";
  url += THINGSPEAK_API_KEY;
  url += "&field1=" + String(currentTemp, 1);
  url += "&field2=" + String(currentHum, 1);
  url += "&field3=" + String(currentLux, 1);
  url += "&field4=" + String(currentSoil, 1);

  http.begin(client, url);
  http.setTimeout(5000);
  int code = http.GET();
  if (code > 0) {
    Serial.printf("ThingSpeak: OK (entry %s)\n", http.getString().c_str());
  } else {
    Serial.printf("ThingSpeak: FAILED (%s)\n", http.errorToString(code).c_str());
  }
  http.end();
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>AG Tech - Plant Monitor</title>
  <style>
    *{margin:0;padding:0;box-sizing:border-box}
    body{font-family:-apple-system,sans-serif;background:#0f172a;color:#e2e8f0;padding:16px;max-width:600px;margin:0 auto}
    h1{text-align:center;margin-bottom:4px;color:#38bdf8;font-size:1.4em}
    .subtitle{text-align:center;color:#64748b;font-size:.8em;margin-bottom:16px}
    .status-bar{border-radius:12px;padding:14px;margin-bottom:12px;text-align:center;font-weight:bold;font-size:1.1em;transition:background .5s}
    .status-ok{background:#166534;color:#4ade80}
    .status-warn{background:#854d0e;color:#fbbf24}
    .status-danger{background:#991b1b;color:#fca5a5;animation:pulse 1s infinite}
    @keyframes pulse{0%,100%{opacity:1}50%{opacity:.7}}
    .section{background:#1e293b;border-radius:12px;padding:16px;margin-bottom:12px}
    .section-title{font-size:.8em;color:#94a3b8;margin-bottom:10px;text-transform:uppercase;letter-spacing:1px}
    .cards{display:flex;gap:10px;margin-bottom:12px;flex-wrap:wrap}
    .card{flex:1;min-width:70px;background:#0f172a;border-radius:8px;padding:12px 8px;text-align:center}
    .card .value{font-size:1.5em;font-weight:bold}
    .card .label{font-size:.65em;color:#94a3b8;margin-top:2px}
    .card .range{font-size:.55em;color:#475569;margin-top:4px}
    .temp .value{color:#f97316}
    .hum .value{color:#22d3ee}
    .lux .value{color:#fbbf24}
    .soil .value{color:#4ade80}
    .na{opacity:.4}
    .chart-box{background:#0f172a;border-radius:8px;padding:12px;margin-top:10px}
    .chart-label{font-size:.75em;color:#64748b;margin-bottom:4px}
    canvas{width:100%!important;height:100px!important}
    .thresholds{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px;font-size:.75em}
    .th-item{background:#0f172a;border-radius:6px;padding:8px;display:flex;justify-content:space-between}
    .th-label{color:#94a3b8}
    .th-ok{color:#4ade80}
    .th-warn{color:#fbbf24}
    .log{background:#0f172a;border-radius:8px;padding:10px;max-height:150px;overflow-y:auto;font-family:monospace;font-size:.65em;margin-top:10px}
    .log div{padding:2px 0;border-bottom:1px solid #1e293b}
    .log .log-ok{color:#4ade80}
    .log .log-warn{color:#fbbf24}
    .log .log-danger{color:#f87171}
    .footer{text-align:center;font-size:.7em;color:#334155;margin-top:12px}
  </style>
</head>
<body>
  <h1>AG Tech Plant Monitor</h1>
  <div class="subtitle">DHT11 + GY-30 + Soil Moisture</div>
  <div class="status-bar status-ok" id="sb">Sensors OK</div>
  <div class="section">
    <div class="section-title">Live Readings</div>
    <div class="cards">
      <div class="card temp"><div class="value" id="t">--</div><div class="label">Temp C</div><div class="range">10-30</div></div>
      <div class="card hum"><div class="value" id="h">--</div><div class="label">Humidity</div><div class="range">30-70%</div></div>
      <div class="card lux"><div class="value" id="l">--</div><div class="label">Light</div><div class="range">500-50k</div></div>
      <div class="card soil"><div class="value" id="s">--</div><div class="label">Soil %</div><div class="range">25-80%</div></div>
    </div>
  </div>
  <div class="section">
    <div class="section-title">History (last 4 min)</div>
    <div class="chart-box"><div class="chart-label">Temperature (C)</div><canvas id="tc"></canvas></div>
    <div class="chart-box" style="margin-top:8px"><div class="chart-label">Humidity (%)</div><canvas id="hc"></canvas></div>
    <div class="chart-box" style="margin-top:8px"><div class="chart-label">Light (lux)</div><canvas id="lc"></canvas></div>
    <div class="chart-box" style="margin-top:8px"><div class="chart-label">Soil Moisture (%)</div><canvas id="sc"></canvas></div>
  </div>
  <div class="section">
    <div class="section-title">Thresholds</div>
    <div class="thresholds">
      <div class="th-item"><span class="th-label">Temp OK</span><span class="th-ok">10-30 C</span></div>
      <div class="th-item"><span class="th-label">Temp Warn</span><span class="th-warn">5-10 / 30-35</span></div>
      <div class="th-item"><span class="th-label">Hum OK</span><span class="th-ok">30-70%</span></div>
      <div class="th-item"><span class="th-label">Hum Warn</span><span class="th-warn">20-30 / 70-80</span></div>
      <div class="th-item"><span class="th-label">Light OK</span><span class="th-ok">500-50k lux</span></div>
      <div class="th-item"><span class="th-label">Light Warn</span><span class="th-warn">&lt;200 / &gt;80k</span></div>
      <div class="th-item"><span class="th-label">Soil OK</span><span class="th-ok">25-80%</span></div>
      <div class="th-item"><span class="th-label">Soil Warn</span><span class="th-warn">&lt;15 / &gt;90</span></div>
    </div>
  </div>
  <div class="section">
    <div class="section-title">Event Log</div>
    <div class="log" id="log"></div>
  </div>
  <div class="footer">LED: Green=OK | Yellow=Warning | Red=Danger</div>
<script>
function dc(cv,data,color,mn,mx){
  let ctx=cv.getContext('2d');let w=cv.width=cv.offsetWidth*2;let h=cv.height=cv.offsetHeight*2;
  ctx.clearRect(0,0,w,h);
  ctx.strokeStyle='#1e293b';ctx.lineWidth=1;
  for(let i=0;i<=4;i++){let y=h*i/4;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();
  ctx.fillStyle='#475569';ctx.font='18px sans-serif';
  let v=mx-(mx-mn)*i/4;
  ctx.fillText(v>=1000?(v/1000).toFixed(1)+'k':v.toFixed(1),4,y+14);}
  if(data.length<2)return;ctx.strokeStyle=color;ctx.lineWidth=2.5;ctx.beginPath();
  for(let i=0;i<data.length;i++){let x=i/(120-1)*w;let y=h-((data[i]-mn)/(mx-mn))*h;
  if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);}ctx.stroke();
}
function fs(){
  fetch('/data').then(r=>r.json()).then(d=>{
    document.getElementById('t').textContent=d.temp.toFixed(1);
    document.getElementById('h').textContent=d.hum.toFixed(1);
    let le=document.getElementById('l');
    if(d.lux>=0)le.textContent=d.lux>=1000?(d.lux/1000).toFixed(1)+'k':d.lux.toFixed(0);
    else{le.textContent='N/A';le.parentElement.classList.add('na');}
    let se=document.getElementById('s');
    if(d.soil>=0)se.textContent=d.soil.toFixed(0);
    else{se.textContent='N/A';se.parentElement.classList.add('na');}
    let sb=document.getElementById('sb');
    sb.className='status-bar status-'+d.status;
    if(d.status==='ok') sb.textContent='All Sensors OK';
    else if(d.status==='warn') sb.textContent='Warning - Check Conditions';
    else sb.textContent='DANGER - Immediate Attention Needed';
    if(d.tempHistory.length>1){
      dc(document.getElementById('tc'),d.tempHistory,'#f97316',
        Math.min(0,Math.floor(Math.min(...d.tempHistory)-5)),Math.max(40,Math.ceil(Math.max(...d.tempHistory)+5)));
      dc(document.getElementById('hc'),d.humHistory,'#22d3ee',
        Math.min(0,Math.floor(Math.min(...d.humHistory)-10)),Math.max(100,Math.ceil(Math.max(...d.humHistory)+10)));
      if(d.luxHistory&&d.luxHistory.length>1&&d.luxHistory[0]>=0)
        dc(document.getElementById('lc'),d.luxHistory,'#fbbf24',
          Math.max(0,Math.floor(Math.min(...d.luxHistory)*0.8)),Math.ceil(Math.max(...d.luxHistory)*1.2+1));
      if(d.soilHistory&&d.soilHistory.length>1&&d.soilHistory[0]>=0)
        dc(document.getElementById('sc'),d.soilHistory,'#4ade80',0,100);
    }
    let log=document.getElementById('log');
    let now=new Date().toLocaleTimeString();
    let entry=document.createElement('div');
    entry.className='log-'+d.status;
    let lx=d.lux>=0?(d.lux>=1000?(d.lux/1000).toFixed(1)+'k':d.lux.toFixed(0))+'lux':'--';
    let sl=d.soil>=0?d.soil.toFixed(0)+'%':'--';
    let msg=now+'  T:'+d.temp.toFixed(1)+'C  H:'+d.hum.toFixed(1)+'%  L:'+lx+'  S:'+sl;
    if(d.status!=='ok') msg+='  ['+d.status.toUpperCase()+']';
    entry.textContent=msg;
    log.insertBefore(entry,log.firstChild);
    if(log.children.length>50)log.removeChild(log.lastChild);
  }).catch(()=>{});
}
fs();setInterval(fs,2000);
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{\"temp\":" + String(currentTemp, 1) +
                ",\"hum\":" + String(currentHum, 1) +
                ",\"lux\":" + String(currentLux, 1) +
                ",\"soil\":" + String(currentSoil, 1) +
                ",\"status\":\"" + currentStatus + "\"" +
                ",\"tempHistory\":[";
  int count = min(totalReadings, MAX_READINGS);
  int start = (totalReadings >= MAX_READINGS) ? readIndex : 0;
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % MAX_READINGS;
    if (i > 0) json += ",";
    json += String(tempHistory[idx], 1);
  }
  json += "],\"humHistory\":[";
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % MAX_READINGS;
    if (i > 0) json += ",";
    json += String(humHistory[idx], 1);
  }
  json += "],\"luxHistory\":[";
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % MAX_READINGS;
    if (i > 0) json += ",";
    json += String(luxHistory[idx], 1);
  }
  json += "],\"soilHistory\":[";
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % MAX_READINGS;
    if (i > 0) json += ",";
    json += String(soilHistory[idx], 1);
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- AG Tech Plant Monitor ---");

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setLED(0, 0, 0);

  dht.begin();

  Wire.begin(4, 5);
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    lightSensorOk = true;
    Serial.println("GY-30 (BH1750) found!");
  } else {
    Serial.println("GY-30 not found - continuing without light sensor");
  }

  // Check soil sensor
  float soilTest = readSoilPercent();
  if (soilTest >= 0) {
    soilSensorOk = true;
    Serial.println("Soil moisture sensor found!");
  } else {
    Serial.println("Soil sensor not found - continuing without soil sensor");
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    setLED(0, 0, 512);
    delay(250);
    setLED(0, 0, 0);
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  setLED(0, 1023, 0);
  delay(500);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Dashboard: http://" + WiFi.localIP().toString());
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
}

void loop() {
  server.handleClient();

  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float lux = -1;
    float soil = -1;

    if (lightSensorOk) {
      lux = lightMeter.readLightLevel();
      if (lux < 0) lux = 0;
    }

    // Re-check soil sensor each read (allows hot-plugging)
    soil = readSoilPercent();
    soilSensorOk = (soil >= 0);

    if (!isnan(h) && !isnan(t)) {
      currentTemp = t;
      currentHum = h;
      currentLux = lux;
      currentSoil = soil;
      tempHistory[readIndex] = t;
      humHistory[readIndex] = h;
      luxHistory[readIndex] = lux;
      soilHistory[readIndex] = soil;
      readIndex = (readIndex + 1) % MAX_READINGS;
      totalReadings++;

      currentStatus = evaluateStatus(t, h, lux, soil);
      updateLED(currentStatus);

      Serial.printf("T:%.1fC H:%.1f%% L:%.0flux S:%.0f%% [%s]\n",
                     t, h, lux, soil, currentStatus.c_str());
    } else {
      setLED(1023, 0, 0);
      Serial.println("DHT11 read error!");
    }
  }

  // ThingSpeak push every 20 seconds
  if (millis() - lastThingSpeak >= 20000) {
    lastThingSpeak = millis();
    sendToThingSpeak();
  }
}
