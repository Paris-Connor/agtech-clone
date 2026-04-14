// RGB LED Dashboard - Wemos D1 Mini
// Control RGB LED from a web dashboard
// Red=D6 (GPIO12), Green=D7 (GPIO13), Blue=D1 (GPIO5)
// DHT11 on D5 (GPIO14)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#include "config.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

#define RED_PIN   12
#define GREEN_PIN 13
#define BLUE_PIN   5
#define DHTPIN    14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

int curR = 0, curG = 0, curB = 0;
String curMode = "solid";
bool powerOn = true;
unsigned long lastTick = 0;
int fadeHue = 0;

// DHT history
#define MAX_READINGS 120
float tempHistory[MAX_READINGS];
float humHistory[MAX_READINGS];
int readIndex = 0;
int totalReadings = 0;
unsigned long lastRead = 0;
float currentTemp = 0, currentHum = 0;

void setColor(int r, int g, int b) {
  analogWrite(RED_PIN, powerOn ? r : 0);
  analogWrite(GREEN_PIN, powerOn ? g : 0);
  analogWrite(BLUE_PIN, powerOn ? b : 0);
}

void hsvToRgb(int h, int* r, int* g, int* b) {
  int sector = h / 60;
  float f = (h % 60) / 60.0;
  int v = 1023;
  int p = 0;
  int q = (int)(v * (1.0 - f));
  int t = (int)(v * f);
  switch (sector % 6) {
    case 0: *r = v; *g = t; *b = p; break;
    case 1: *r = q; *g = v; *b = p; break;
    case 2: *r = p; *g = v; *b = t; break;
    case 3: *r = p; *g = q; *b = v; break;
    case 4: *r = t; *g = p; *b = v; break;
    case 5: *r = v; *g = p; *b = q; break;
  }
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>AG Tech - RGB + Sensor Dashboard</title>
  <style>
    *{margin:0;padding:0;box-sizing:border-box}
    body{font-family:-apple-system,sans-serif;background:#0f172a;color:#e2e8f0;padding:16px;max-width:600px;margin:0 auto}
    h1{text-align:center;margin-bottom:16px;color:#38bdf8;font-size:1.4em}
    .section{background:#1e293b;border-radius:12px;padding:16px;margin-bottom:12px}
    .section-title{font-size:.85em;color:#94a3b8;margin-bottom:10px;text-transform:uppercase;letter-spacing:1px}
    .cards{display:flex;gap:12px;margin-bottom:12px}
    .card{flex:1;background:#0f172a;border-radius:8px;padding:14px;text-align:center}
    .card .value{font-size:2em;font-weight:bold}
    .card .label{font-size:.8em;color:#94a3b8;margin-top:2px}
    .temp .value{color:#f97316}
    .hum .value{color:#22d3ee}

    .color-picker{display:flex;align-items:center;gap:12px;margin-bottom:12px}
    .color-picker input[type=color]{width:60px;height:60px;border:none;border-radius:8px;cursor:pointer;background:none}
    .color-preview{width:60px;height:60px;border-radius:50%;border:3px solid #334155;transition:background .2s}

    .presets{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-bottom:12px}
    .preset{height:40px;border:2px solid #334155;border-radius:8px;cursor:pointer;transition:transform .1s}
    .preset:hover{transform:scale(1.1)}
    .preset.active{border-color:#38bdf8}

    .modes{display:flex;gap:8px;margin-bottom:12px;flex-wrap:wrap}
    .mode-btn{padding:8px 16px;border-radius:8px;border:2px solid #334155;background:#0f172a;color:#e2e8f0;cursor:pointer;font-size:.85em}
    .mode-btn.active{border-color:#38bdf8;background:#1e3a5f}

    .brightness{margin-bottom:12px}
    .brightness input{width:100%;accent-color:#38bdf8}

    .power-btn{width:100%;padding:12px;border-radius:8px;border:none;font-size:1em;font-weight:bold;cursor:pointer;transition:background .2s}
    .power-btn.on{background:#22c55e;color:#0f172a}
    .power-btn.off{background:#ef4444;color:white}

    .chart-box{background:#0f172a;border-radius:8px;padding:12px;margin-top:10px}
    .chart-label{font-size:.75em;color:#64748b;margin-bottom:4px}
    canvas{width:100%!important;height:120px!important}
  </style>
</head>
<body>
  <h1>AG Tech Dashboard</h1>

  <div class="section">
    <div class="section-title">Sensor Readings</div>
    <div class="cards">
      <div class="card temp"><div class="value" id="temp">--</div><div class="label">Temperature &deg;C</div></div>
      <div class="card hum"><div class="value" id="hum">--</div><div class="label">Humidity %</div></div>
    </div>
    <div class="chart-box">
      <div class="chart-label">Temperature</div>
      <canvas id="tempChart"></canvas>
    </div>
    <div class="chart-box" style="margin-top:8px">
      <div class="chart-label">Humidity</div>
      <canvas id="humChart"></canvas>
    </div>
  </div>

  <div class="section">
    <div class="section-title">RGB LED Control</div>

    <div class="color-picker">
      <input type="color" id="colorPick" value="#ff0000">
      <div class="color-preview" id="preview"></div>
      <div style="flex:1">
        <div style="font-size:.8em;color:#94a3b8">Current Color</div>
        <div id="hexLabel" style="font-size:1.2em;font-weight:bold">#FF0000</div>
      </div>
    </div>

    <div class="section-title">Presets</div>
    <div class="presets">
      <div class="preset" style="background:#ff0000" data-r="255" data-g="0" data-b="0"></div>
      <div class="preset" style="background:#00ff00" data-r="0" data-g="255" data-b="0"></div>
      <div class="preset" style="background:#0000ff" data-r="0" data-g="0" data-b="255"></div>
      <div class="preset" style="background:#ffffff" data-r="255" data-g="255" data-b="255"></div>
      <div class="preset" style="background:#ffff00" data-r="255" data-g="255" data-b="0"></div>
      <div class="preset" style="background:#00ffff" data-r="0" data-g="255" data-b="255"></div>
      <div class="preset" style="background:#ff00ff" data-r="255" data-g="0" data-b="255"></div>
      <div class="preset" style="background:#ff8800" data-r="255" data-g="136" data-b="0"></div>
      <div class="preset" style="background:#ff1493" data-r="255" data-g="20" data-b="147"></div>
      <div class="preset" style="background:#7b68ee" data-r="123" data-g="104" data-b="238"></div>
      <div class="preset" style="background:#00fa9a" data-r="0" data-g="250" data-b="154"></div>
      <div class="preset" style="background:#ff6347" data-r="255" data-g="99" data-b="71"></div>
    </div>

    <div class="section-title">Mode</div>
    <div class="modes">
      <div class="mode-btn active" data-mode="solid">Solid</div>
      <div class="mode-btn" data-mode="rainbow">Rainbow</div>
      <div class="mode-btn" data-mode="pulse">Pulse</div>
      <div class="mode-btn" data-mode="flash">Flash</div>
    </div>

    <div class="section-title">Brightness</div>
    <div class="brightness">
      <input type="range" id="bright" min="0" max="100" value="100">
    </div>

    <button class="power-btn off" id="powerBtn" onclick="togglePower()">OFF</button>
  </div>

<script>
  let curR=255,curG=0,curB=0,brightness=100,power=false,mode='solid';

  function sendColor(){
    let r=Math.round(curR*brightness/100);
    let g=Math.round(curG*brightness/100);
    let b=Math.round(curB*brightness/100);
    fetch('/set?r='+r+'&g='+g+'&b='+b+'&mode='+mode+'&power='+(power?1:0));
    updatePreview();
  }

  function updatePreview(){
    let r=Math.round(curR*brightness/100*(power?1:0));
    let g=Math.round(curG*brightness/100*(power?1:0));
    let b=Math.round(curB*brightness/100*(power?1:0));
    document.getElementById('preview').style.background='rgb('+r+','+g+','+b+')';
    let hex='#'+[curR,curG,curB].map(x=>x.toString(16).padStart(2,'0')).join('').toUpperCase();
    document.getElementById('hexLabel').textContent=hex;
  }

  document.getElementById('colorPick').addEventListener('input',function(e){
    let hex=e.target.value;
    curR=parseInt(hex.substr(1,2),16);
    curG=parseInt(hex.substr(3,2),16);
    curB=parseInt(hex.substr(5,2),16);
    mode='solid';
    document.querySelectorAll('.mode-btn').forEach(b=>b.classList.remove('active'));
    document.querySelector('[data-mode=solid]').classList.add('active');
    sendColor();
  });

  document.querySelectorAll('.preset').forEach(p=>{
    p.addEventListener('click',function(){
      curR=+this.dataset.r;curG=+this.dataset.g;curB=+this.dataset.b;
      let hex='#'+[curR,curG,curB].map(x=>x.toString(16).padStart(2,'0')).join('');
      document.getElementById('colorPick').value=hex;
      mode='solid';
      document.querySelectorAll('.mode-btn').forEach(b=>b.classList.remove('active'));
      document.querySelector('[data-mode=solid]').classList.add('active');
      sendColor();
    });
  });

  document.querySelectorAll('.mode-btn').forEach(b=>{
    b.addEventListener('click',function(){
      mode=this.dataset.mode;
      document.querySelectorAll('.mode-btn').forEach(x=>x.classList.remove('active'));
      this.classList.add('active');
      sendColor();
    });
  });

  document.getElementById('bright').addEventListener('input',function(){
    brightness=+this.value;
    sendColor();
  });

  function togglePower(){
    power=!power;
    let btn=document.getElementById('powerBtn');
    btn.textContent=power?'ON':'OFF';
    btn.className='power-btn '+(power?'on':'off');
    sendColor();
  }

  function drawChart(canvas,data,color,mn,mx){
    let ctx=canvas.getContext('2d');
    let w=canvas.width=canvas.offsetWidth*2;
    let h=canvas.height=canvas.offsetHeight*2;
    ctx.clearRect(0,0,w,h);
    ctx.strokeStyle='#1e293b';ctx.lineWidth=1;
    for(let i=0;i<=3;i++){let y=h*i/3;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();
      ctx.fillStyle='#475569';ctx.font='18px sans-serif';
      ctx.fillText((mx-(mx-mn)*i/3).toFixed(1),4,y+14);}
    if(data.length<2)return;
    ctx.strokeStyle=color;ctx.lineWidth=2.5;ctx.beginPath();
    for(let i=0;i<data.length;i++){
      let x=i/(120-1)*w;let y=h-((data[i]-mn)/(mx-mn))*h;
      if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);}
    ctx.stroke();
  }

  function fetchSensor(){
    fetch('/data').then(r=>r.json()).then(d=>{
      document.getElementById('temp').textContent=d.temp.toFixed(1);
      document.getElementById('hum').textContent=d.hum.toFixed(1);
      if(d.tempHistory.length>1){
        let tMn=Math.floor(Math.min(...d.tempHistory)-2);
        let tMx=Math.ceil(Math.max(...d.tempHistory)+2);
        let hMn=Math.floor(Math.min(...d.humHistory)-5);
        let hMx=Math.ceil(Math.max(...d.humHistory)+5);
        drawChart(document.getElementById('tempChart'),d.tempHistory,'#f97316',tMn,tMx);
        drawChart(document.getElementById('humChart'),d.humHistory,'#22d3ee',hMn,hMx);
      }
    }).catch(()=>{});
  }

  updatePreview();
  fetchSensor();
  setInterval(fetchSensor,2000);
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleSet() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    curR = server.arg("r").toInt();
    curG = server.arg("g").toInt();
    curB = server.arg("b").toInt();
  }
  if (server.hasArg("mode")) curMode = server.arg("mode");
  if (server.hasArg("power")) powerOn = server.arg("power").toInt() == 1;

  // Map 0-255 to 0-1023 for ESP8266 PWM
  if (curMode == "solid") {
    setColor(curR * 4, curG * 4, curB * 4);
  }

  server.send(200, "text/plain", "ok");
  Serial.printf("SET r=%d g=%d b=%d mode=%s power=%d\n", curR, curG, curB, curMode.c_str(), powerOn);
}

void handleData() {
  String json = "{\"temp\":" + String(currentTemp, 1) +
                ",\"hum\":" + String(currentHum, 1) +
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
  json += "]}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- AG Tech RGB + Sensor Dashboard ---");

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  // Start with LED off
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  powerOn = false;

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Dashboard: http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();

  // DHT reading every 2s
  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      currentTemp = t;
      currentHum = h;
      tempHistory[readIndex] = t;
      humHistory[readIndex] = h;
      readIndex = (readIndex + 1) % MAX_READINGS;
      totalReadings++;
    }
  }

  // LED animation modes
  if (powerOn && millis() - lastTick >= 20) {
    lastTick = millis();

    if (curMode == "rainbow") {
      fadeHue = (fadeHue + 2) % 360;
      int r, g, b;
      hsvToRgb(fadeHue, &r, &g, &b);
      setColor(r, g, b);
    }
    else if (curMode == "pulse") {
      float breath = (sin(millis() / 500.0) + 1.0) / 2.0;
      setColor((int)(curR * 4 * breath), (int)(curG * 4 * breath), (int)(curB * 4 * breath));
    }
    else if (curMode == "flash") {
      bool on = (millis() / 300) % 2 == 0;
      setColor(on ? curR * 4 : 0, on ? curG * 4 : 0, on ? curB * 4 : 0);
    }
  }
}
