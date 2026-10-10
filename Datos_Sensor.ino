const int PIR_PIN = 27;

int estadoAnterior = LOW;
unsigned long inicioActivacion = 0;
unsigned long ultimoInicio = 0;
unsigned long tiempoTotalActivo = 0;
int contador = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  Serial.println("Calentando sensor PIR (30 s)...");
  delay(30000);
  Serial.println("Sensor listo.");
}

void loop() {
  int estado = digitalRead(PIR_PIN);
  unsigned long ahora = millis();

  // Flanco de subida: inicia la activación
  if (estado == HIGH && estadoAnterior == LOW) {
    contador++;
    inicioActivacion = ahora;

    Serial.println("-----------------------------");
    Serial.print("Detección #");
    Serial.println(contador);
    Serial.print("Inicio: ");
    Serial.print(ahora / 1000.0, 1);
    Serial.println(" s desde el arranque");

    if (contador > 1) {
      Serial.print("Tiempo desde la detección anterior: ");
      Serial.print((ahora - ultimoInicio) / 1000.0, 1);
      Serial.println(" s");
    }
    ultimoInicio = ahora;
  }

  // Flanco de bajada: termina la activación
  if (estado == LOW && estadoAnterior == HIGH) {
    unsigned long duracion = ahora - inicioActivacion;
    tiempoTotalActivo += duracion;

    Serial.print("Fin. Duración de la activación: ");
    Serial.print(duracion / 1000.0, 1);
    Serial.println(" s");
    Serial.print("Tiempo total activo: ");
    Serial.print(tiempoTotalActivo / 1000.0, 1);
    Serial.print(" s (");
    Serial.print(100.0 * tiempoTotalActivo / ahora, 1);
    Serial.println(" % del tiempo)");
  }

  estadoAnterior = estado;
  delay(50);
}