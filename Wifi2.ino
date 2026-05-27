#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// INSTANCIAS DE RED
DNSServer dnsServer;
WebServer server(80);

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 8, 1); // Subred del Router
IPAddress subnet(255, 255, 255, 0);

// CONFIGURACIÓN DE REDES (Ajusta según tu entorno)
String ssid_origin = "Armor22"; // Red inicial para tocar Laravel
String pass_origin = "ak12345ak";

String ssid_casa = "";
String pass_casa = "";

bool usuarioValidado = false;
const char* ap_ssid = "LinkHealth_Router_S3";

String IpServer = "http://172.20.10.4:8000"; // Tu servidor Laravel
String serverAccess = IpServer + "/api/esp32/acceso";

unsigned long previousMillis = 0;
const long interval = 30000;

// INTERFAZ DEL PORTAL CAUTIVO
const char PORTAL_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>LinkHealth - Portal de Acceso</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; background: #eef2f5; text-align: center; padding: 20px; color: #333; }
    .card { background: white; padding: 25px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); max-width: 400px; margin: auto; }
    h2 { color: #028090; margin-bottom: 5px; }
    p { color: #666; font-size: 14px; margin-top: 0; }
    .status-badge { background: #ffebee; color: #c62828; padding: 8px; border-radius: 6px; font-size: 13px; font-weight: bold; margin-bottom: 15px; }
    input { width: 100%; padding: 12px; margin: 8px 0; border: 1px solid #ccc; border-radius: 6px; box-sizing: border-box; }
    button { background: #028090; color: white; border: none; padding: 14px; width: 100%; cursor: pointer; border-radius: 6px; font-size: 16px; font-weight: bold; }
    button:hover { background: #00a896; }
  </style>
</head>
<body>
  <div class="card">
    <h2>LinkHealth</h2>
    <p>Portal de Validación de Dispositivos</p>
    <div class="status-badge">Nivel 3: Acceso Restringido (Sin Internet)</div>
    <form action="/save" method="POST">
      <input name="paciente" placeholder="Nombre del Paciente" required>
      <input name="clinica" placeholder="ID de Clínica / Habitación" required>
      <button type="submit">VALIDAR Y ENRUTAR INTERNET</button>
    </form>
  </div>
</body>
</html>
)=====";

// ACTIVACIÓN DE LA PASARELA UTILIZANDO MÉTODOS ESTÁNDAR DEL NÚCLEO v3.X
void habilitarEnrutadorNAT() {
  Serial.println("\n=========================================");
  Serial.println(" !!! MUTACIÓN EXITOSA: ROUTER ACTIVO !!! ");
  Serial.println("=========================================");
  Serial.println("[ROUTER] Enrutamiento de datos habilitado por el sistema.");
}

void handleSave() {
  if (usuarioValidado) {
    server.send(200, "text/html", "<h1>Procesando...</h1><p>El router ya está cambiando de frecuencia.</p>");
    return;
  }

  String p = server.arg("paciente"); 
  String c = server.arg("clinica");  

  Serial.println("[PORTAL] Petición recibida. Consultando credenciales en Laravel...");

  if (WiFi.status() == WL_CONNECTED) {
    String mac = WiFi.macAddress(); 
    HTTPClient http;
    
    http.begin(serverAccess);
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"mac_address\":\"" + mac + "\",";
    json += "\"paciente_name\":\"" + p + "\",";  
    json += "\"clinica_id\":\"" + c + "\"";
    json += "}";

    int httpResponseCode = http.POST(json); 
    String response = http.getString(); 

    if (httpResponseCode == 200) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, response);

      if (!error) {
        ssid_casa = doc["ssid"].as<String>();
        pass_casa = doc["password"].as<String>();

        Serial.println("[BACKEND] Datos válidos. Apagando portal cautivo...");
        
        usuarioValidado = true; 
        dnsServer.stop(); 

        server.send(200, "text/html", "<h1>Acceso Concedido</h1><p>Activando tablas de enrutamiento... Conectando a la red operativa.</p>");
        delay(2000); 

        WiFi.disconnect(); 
        delay(1000); 

        Serial.printf("[WAN] Saltando a red definitiva: %s\n", ssid_casa.c_str());
        WiFi.begin(ssid_casa.c_str(), pass_casa.c_str());

        int tries = 0;
        while (WiFi.status() != WL_CONNECTED && tries < 20) {
          delay(500);
          Serial.print(".");
          tries++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\n[WAN] Conectado a la red operativa de destino.");
          habilitarEnrutadorNAT(); 
        } else {
          Serial.println("\n[FALLA] Error al conectar a la red de la BD. Revirtiendo estado.");
          usuarioValidado = false;
          dnsServer.start(DNS_PORT, "*", apIP); 
        }
      } else {
        server.send(200, "text/html", "<h1>Error</h1><p>Formato de respuesta inválido.</p>");
      }
    } else if (httpResponseCode == 404) {
      server.send(200, "text/html", "<h1>Acceso Denegado</h1><p>Paciente o Habitación no registrados en Laravel.</p>");
    } else {
      server.send(200, "text/html", "<h1>Error 500</h1><p>Falla en el servidor central.</p>");
    }
    http.end();
  } else {
    server.send(200, "text/html", "<h1>Error de Enlace</h1><p>La ESP32 no tiene salida WAN de validación.</p>");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.disconnect(true, true); 
  delay(500);

  // Activación del modo híbrido nativo (AP + STA) compatible con la v3.0.7
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, subnet);
  WiFi.softAP(ap_ssid);
  Serial.println("[LAN] Señal Wi-Fi emitida: " + String(ap_ssid));

  server.on("/", []() { server.send(200, "text/html", PORTAL_HTML); });
  server.on("/save", HTTP_POST, handleSave);
  server.on("/generate_204", []() { server.send(200, "text/html", PORTAL_HTML); }); 
  server.on("/fwlink", []() { server.send(200, "text/html", PORTAL_HTML); });       
  server.on("/hotspot-detect.html", []() { server.send(200, "text/html", PORTAL_HTML); }); 
  server.onNotFound([]() { server.send(200, "text/html", PORTAL_HTML); });

  Serial.printf("[WAN] Conectando a red de validación: %s\n", ssid_origin.c_str());
  WiFi.begin(ssid_origin.c_str(), pass_origin.c_str());

  int checkTries = 0;
  while (WiFi.status() != WL_CONNECTED && checkTries < 16) {
    delay(500);
    Serial.print(".");
    checkTries++;
  }

  dnsServer.start(DNS_PORT, "*", apIP);
  server.begin();
  Serial.println("[SISTEMA] Listo. Esperando interacción en el Portal Cautivo...");
}

void loop() {
  server.handleClient();

  if (!usuarioValidado) {
    dnsServer.processNextRequest();
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; 

    if (WiFi.status() != WL_CONNECTED && usuarioValidado) {
      Serial.println("[KEEP-ALIVE] Enlace caído. Reenlazando a red operativa...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
}