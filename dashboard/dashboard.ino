// DHT11 Dashboard - Wemos D1 Mini
// Serves a live web dashboard with temperature & humidity graphs
// Data pin: D5 (GPIO14)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTPIN 14
#define DHTTYPE DHT11

// WiFi credentials - edit config.h (see config.example.h)
#include "config.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

// Store last 120 readings (~4 minutes at 2s interval)
#define MAX_READINGS 120
float tempHistory[MAX_READINGS];
float humHistory[MAX_READINGS];
int readIndex = 0;
int totalReadings = 0;

unsigned long lastRead = 0;
float currentTemp = 0;
float currentHum = 0;

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>AG Tech - DHT11 Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { font-family: -apple-system, sans-serif; background: #0f172a; color: #e2e8f0; padding: 20px; }
    h1 { text-align: center; margin-bottom: 20px; color: #38bdf8; font-size: 1.5em; }
    .cards { display: flex; gap: 16px; justify-content: center; margin-bottom: 24px; flex-wrap: wrap; }
    .card { background: #1e293b; border-radius: 12px; padding: 24px 32px; text-align: center; min-width: 180px; }
    .card .value { font-size: 2.5em; font-weight: bold; }
    .card .label { font-size: 0.9em; color: #94a3b8; margin-top: 4px; }
    .temp .value { color: #f97316; }
    .hum .value { color: #22d3ee; }
    .chart-container { background: #1e293b; border-radius: 12px; padding: 20px; margin-bottom: 16px; }
    .chart-title { font-size: 0.9em; color: #94a3b8; margin-bottom: 8px; }
    canvas { width: 100% !important; height: 200px !important; }
    .log { background: #1e293b; border-radius: 12px; padding: 16px; max-height: 200px; overflow-y: auto; font-family: monospace; font-size: 0.8em; }
    .log div { padding: 2px 0; border-bottom: 1px solid #334155; }
    .status { text-align: center; font-size: 0.8em; color: #64748b; margin-top: 12px; }
  </style>
</head>
<body>
  <h1>AG Tech - DHT11 Dashboard</h1>
  <div class="cards">
    <div class="card temp">
      <div class="value" id="temp">--</div>
      <div class="label">Temperature °C</div>
    </div>
    <div class="card hum">
      <div class="value" id="hum">--</div>
      <div class="label">Humidity %</div>
    </div>
  </div>
  <div class="chart-container">
    <div class="chart-title">Temperature (°C)</div>
    <canvas id="tempChart"></canvas>
  </div>
  <div class="chart-container">
    <div class="chart-title">Humidity (%)</div>
    <canvas id="humChart"></canvas>
  </div>
  <div class="chart-title" style="margin: 12px 0 8px;">Live Log</div>
  <div class="log" id="log"></div>
  <div class="status">Refreshing every 2 seconds</div>

<script>
  const maxPoints = 120;
  let tempData = [], humData = [];

  function drawChart(canvas, data, color, min, max) {
    const ctx = canvas.getContext('2d');
    const w = canvas.width = canvas.offsetWidth * 2;
    const h = canvas.height = canvas.offsetHeight * 2;
    ctx.clearRect(0, 0, w, h);

    // grid
    ctx.strokeStyle = '#334155';
    ctx.lineWidth = 1;
    for (let i = 0; i <= 4; i++) {
      let y = (h * i) / 4;
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
      ctx.fillStyle = '#64748b';
      ctx.font = '20px sans-serif';
      let val = max - ((max - min) * i / 4);
      ctx.fillText(val.toFixed(1), 4, y + 16);
    }

    if (data.length < 2) return;
    ctx.strokeStyle = color;
    ctx.lineWidth = 3;
    ctx.beginPath();
    for (let i = 0; i < data.length; i++) {
      let x = (i / (maxPoints - 1)) * w;
      let y = h - ((data[i] - min) / (max - min)) * h;
      if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    }
    ctx.stroke();
  }

  function update() {
    fetch('/data').then(r => r.json()).then(d => {
      document.getElementById('temp').textContent = d.temp.toFixed(1);
      document.getElementById('hum').textContent = d.hum.toFixed(1);

      tempData = d.tempHistory;
      humData = d.humHistory;

      let tMin = Math.floor(Math.min(...tempData) - 2);
      let tMax = Math.ceil(Math.max(...tempData) + 2);
      let hMin = Math.floor(Math.min(...humData) - 5);
      let hMax = Math.ceil(Math.max(...humData) + 5);

      drawChart(document.getElementById('tempChart'), tempData, '#f97316', tMin, tMax);
      drawChart(document.getElementById('humChart'), humData, '#22d3ee', hMin, hMax);

      let log = document.getElementById('log');
      let now = new Date().toLocaleTimeString();
      let entry = document.createElement('div');
      entry.textContent = now + '  |  Temp: ' + d.temp.toFixed(1) + '°C  |  Humidity: ' + d.hum.toFixed(1) + '%';
      log.insertBefore(entry, log.firstChild);
      if (log.children.length > 50) log.removeChild(log.lastChild);
    }).catch(e => console.error(e));
  }

  update();
  setInterval(update, 2000);
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
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
  Serial.println("\n--- AG Tech DHT11 Dashboard ---");

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Dashboard: http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();

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

      Serial.printf("Temp: %.1f°C  |  Humidity: %.1f%%\n", t, h);
    }
  }
}
