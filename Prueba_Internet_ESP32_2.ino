#include <WiFi.h>
#include "esp_netif.h" // NUEVA LIBRERÍA: Para manejar las redes de forma segura en el Kernel

// 1. La red de donde la ESP32 va a "ROBAR" el internet
const char* RED_CON_INTERNET = "Armor22";
const char* CONTRASEÑA_INTERNET = "ak12345ak";

// 2. La red propia que emite la ESP32
const char* AP_SSID = "Link_Health";
const char* AP_PASSWORD = "12346789"; 

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Iniciando ESP32 en Modo Repetidor con NAT ---");

  // Configurar modo híbrido
  WiFi.mode(WIFI_AP_STA);

  // Conectarse a la red con internet
  WiFi.begin(RED_CON_INTERNET, CONTRASEÑA_INTERNET);
  Serial.print("Conectando a la red con internet...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ ¡Conectado al internet de fondo!");

  // Crear la red propia
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("✅ Red propia emitida: ");
  Serial.println(AP_SSID);

  // --- SOLUCIÓN AL KERNEL PANIC (ENRUTAMIENTO SEGURO) ---
  // Obtenemos la interfaz del Access Point de forma legal en el sistema
  esp_netif_t *netif_ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  
  if (netif_ap != NULL) {
      esp_netif_napt_enable(netif_ap); // Habilitamos NAT con candado de seguridad
      Serial.println("🚀 Servidor NAT activado correctamente. ¡El internet está fluyendo!");
  } else {
      Serial.println("❌ Error crítico: No se encontró la interfaz de red.");
  }
}

void loop() {
  // El hardware de la ESP32 hace el enrutamiento de paquetes en segundo plano
}