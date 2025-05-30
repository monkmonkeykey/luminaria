#include <bluefruit.h>

BLEClientUart clientUart;
bool conectado = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Bluefruit.begin(0, 1); // 0 periféricos, 1 central
  Bluefruit.setName("XIAO-RECEPTOR");

  clientUart.begin();

  // Asignar funciones de conexión
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  // Escaneo BLE
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.start(0); // 0 = escaneo infinito

  Serial.println("Escaneando dispositivos UART...");
}

void loop() {
  if (conectado && clientUart.available()) {
    String msg = clientUart.readStringUntil('\n');
    Serial.print("Recibido: ");
    Serial.println(msg);
  }
}

void scan_callback(ble_gap_evt_adv_report_t* report) {
  if (Bluefruit.Scanner.checkReportForUuid(report, clientUart.uuid)) {
    Serial.println("Dispositivo UART encontrado. Conectando...");
    Bluefruit.Central.connect(report);
    Bluefruit.Scanner.stop(); // detener escaneo una vez encontrado
  }
}

void connect_callback(uint16_t conn_handle) {
  Serial.println("¡Conectado al periférico!");
  if (clientUart.discover(conn_handle)) {
    conectado = true;
    Serial.println("Servicio UART detectado.");
  } else {
    Serial.println("Error: Servicio UART no detectado.");
    Bluefruit.disconnect(conn_handle);  // <- CORRECTO
  }
}


void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.print("Desconectado. Razón: ");
  Serial.println(reason);
  conectado = false;
  Bluefruit.Scanner.start(0); // reiniciar escaneo
}
