// AG Tech - ESP32-CAM Plant Photo Feed
// Serves live JPEG snapshots for the plant monitor dashboard
// Endpoints:
//   GET /capture  - returns latest JPEG image
//   GET /stream   - MJPEG video stream
//   GET /status   - JSON with camera info

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

#include "config.h"

// AI-Thinker ESP32-CAM pin definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Built-in flash LED
#define FLASH_GPIO_NUM     4

WebServer server(80);

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Use higher resolution if PSRAM is available
  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;   // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    Serial.println("PSRAM found - using SVGA resolution");
  } else {
    config.frame_size = FRAMESIZE_VGA;    // 640x480
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
    Serial.println("No PSRAM - using VGA resolution");
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }

  // Adjust camera settings for indoor plant photography
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, 1);     // slightly brighter
  s->set_saturation(s, 1);     // slightly more vivid (good for green plants)
  s->set_whitebal(s, 1);       // auto white balance on
  s->set_awb_gain(s, 1);       // auto white balance gain on
  s->set_exposure_ctrl(s, 1);  // auto exposure on
  s->set_gain_ctrl(s, 1);      // auto gain on

  Serial.println("Camera initialized!");
}

void handleCapture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void handleStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n";
  response += "Access-Control-Allow-Origin: *\r\n\r\n";
  client.print(response);

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) break;

    String header = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: ";
    header += String(fb->len) + "\r\n\r\n";
    client.print(header);
    client.write(fb->buf, fb->len);
    client.print("\r\n");
    esp_camera_fb_return(fb);
    delay(33); // ~30fps max
  }
}

void handleStatus() {
  String json = "{\"status\":\"ok\"";
  json += ",\"psram\":" + String(psramFound() ? "true" : "false");
  json += ",\"resolution\":\"" + String(psramFound() ? "800x600" : "640x480") + "\"";
  json += ",\"uptime\":" + String(millis() / 1000);
  json += ",\"freeHeap\":" + String(ESP.getFreeHeap());
  json += "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleFlash() {
  // Toggle flash LED for dark conditions
  static bool flashOn = false;
  flashOn = !flashOn;
  digitalWrite(FLASH_GPIO_NUM, flashOn ? HIGH : LOW);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", flashOn ? "Flash ON" : "Flash OFF");
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>AG Tech Camera</title>";
  html += "<style>body{font-family:sans-serif;background:#f0f7f0;text-align:center;padding:20px}";
  html += "img{max-width:100%;border-radius:12px;border:2px solid #c8e0c8}";
  html += "h1{color:#2d6a2d}a{color:#2d6a2d;margin:0 8px}</style></head>";
  html += "<body><h1>AG Tech Camera</h1>";
  html += "<p><img src='/capture' id='img'></p>";
  html += "<p><a href='/capture'>Snapshot</a> | <a href='/stream'>Stream</a> | ";
  html += "<a href='#' onclick='fetch(\"/flash\")'>Toggle Flash</a></p>";
  html += "<script>setInterval(()=>{document.getElementById('img').src='/capture?'+Date.now()},";
  html += String(CAPTURE_INTERVAL * 1000) + ");</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- AG Tech ESP32-CAM ---");

  // Flash LED off
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  digitalWrite(FLASH_GPIO_NUM, LOW);

  initCamera();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.on("/stream", handleStream);
  server.on("/status", handleStatus);
  server.on("/flash", handleFlash);
  server.begin();

  Serial.println("Camera ready!");
  Serial.println("  Snapshot: http://" + WiFi.localIP().toString() + "/capture");
  Serial.println("  Stream:   http://" + WiFi.localIP().toString() + "/stream");
  Serial.println("  Status:   http://" + WiFi.localIP().toString() + "/status");
}

void loop() {
  server.handleClient();
}
