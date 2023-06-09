#include <WiFi.h>
#include <MySQL_Connection.h> // Hay que instalar esta libreria (aparece como "MySQL Connector Arduino")
#include <MySQL_Cursor.h>

const char* ssid = "tu-red-wifi-chikito";
const char* password = "lolo-folla-madres-69";

IPAddress serverIP(); 
unsigned int serverPort = ; 
char* user = ;
char* passwordDB = ;
char* database = ;
char* table = ;

WiFiClient client;
MySQL_Connection conn((Client *)&client);

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  
  Serial.println("Conectado a WiFi");
  
  Serial.println("Conectando a la base de datos...");
  if (conn.connect(serverIP, serverPort, user, passwordDB)) {
    Serial.println("Conectado a la base de datos");
  } else {
    Serial.println("Error de conexión");
    while (1);
  }
}

String obtenerFechaHoraActual() {
  String fecha_hora;
  
  time_t now = time(nullptr);
  struct tm* timeinfo;
  timeinfo = localtime(&now);
  
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  
  fecha_hora = buffer;
  
  return fecha_hora;
}

void loop() {

  String fecha_hora = obtenerFechaHoraActual();


  const char* item = "valor_item";
  const char* ubicacion = "valor_ubicacion";
  
  
  char query[256];
  snprintf(query, sizeof(query), "USE %s; INSERT INTO %s (item, ubicacion, fecha_hora) VALUES ('%s', '%s', '%s')", database, table, item, ubicacion, fecha_hora.c_str());
  MySQL_Cursor* cursor = new MySQL_Cursor(&conn);

  cursor->execute(query);
  
  delete cursor;
  
  Serial.println("Consulta ejecutada");
  
  delay(10000); // Hay que cambiar esto xd
}


