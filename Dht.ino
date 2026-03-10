#define HTML "<!DOCTYPE html><html><head><title>Метеостанция ESP8266</title><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><style>body{font-family:Arial,sans-serif;text-align:center;margin:50px;background:#f0f0f0}.c{max-width:600px;margin:0 auto;background:#fff;padding:20px;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,.1)}.db{margin:20px;padding:20px;border-radius:5px;display:inline-block}.t{background:#ffebee;border:2px solid #f44336}.h{background:#e3f2fd;border:2px solid #2196f3}.v{font-size:2em;font-weight:700;margin:10px 0}.u{font-size:.8em;color:#666}.b{padding:10px 20px;margin:10px;background:#4caf50;color:#fff;border:none;border-radius:5px;cursor:pointer}.b:hover{background:#45a049}</style></head><body><div class=c><h1>🌡️ Метеостанция ESP8266</h1><p>Последнее обновление: <span id=lu>--</span></p><div class=\"db t\"><h3>🌡️ Температура</h3><div class=v id=t>--</div><div class=u>°C</div></div><div class=\"db h\"><h3>💧 Влажность</h3><div class=v id=h>--</div><div class=u>%</div></div><br><button class=b onclick=ud()>🔄 Обновить</button></div><script>let e={temp:document.getElementById(\"t\"),hum:document.getElementById(\"h\"),time:document.getElementById(\"lu\")},ud=()=>fetch(\"/data\").then(r=>r.status===500 ? alert(\"ERROR 500\") : r.json().then(({t:t,h:h})=>{e.time.textContent=new Date().toLocaleTimeString(),e.temp.textContent=t,e.hum.textContent=h}));ud();setInterval(ud,5e3);</script></body></html>"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define LED_BUILTIN 2 // Встроенный светодиод (Для индикации подключения)


ESP8266WebServer server(80);
DHT dht(5, DHT11); // Замените на нужный пин

struct CacheData {
  float temperature;
  float humidity;
  unsigned long lastUpdate;
  bool isValid;
} cache = {0, 0, 0, false};

bool checkAuth() {
  if (!server.authenticate("admin", "admin")) { // Замените на свои данные
    server.requestAuthentication();
    return false;
  }
  return true;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  dht.begin();

  connectWifi();
  
  server.on("/", [](){ if (checkAuth()) server.send(200, "text/html", HTML); });
  server.on("/data", handleData);
  
  server.begin();
}

void loop() {
  server.handleClient();
  updateCache();
}

void handleData() {
  if (!checkAuth()) return;

  if (!cache.isValid) {
    server.send(500);
    return;
  }

  char json[64];
  sprintf(json, R"({"t":%.1f,"h":%.1f})", cache.temperature, cache.humidity);
  server.send(200, "application/json", json);
}

void updateCache() {
  static unsigned long lastSensorRead = 0;
  const unsigned long now = millis();

  if (now - lastSensorRead >= 4000) {
    lastSensorRead = now;
    
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
      cache.temperature = t;
      cache.humidity = h;
      cache.lastUpdate = now;
      cache.isValid = true;
    } else cache.isValid = false;
  }
}

void connectWifi() {
  WiFi.begin("SSID", "password"); // Заммените на свои данные
  Serial.print("\nConnecting to WiFi ");
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(125);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125);
    digitalWrite(LED_BUILTIN, LOW);
    delay(125);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125); 
    Serial.print(".");
  }

  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
}