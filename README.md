# Detector de movimiento con ESP32 y sensor PIR SR505

Proyecto en **Arduino IDE** que lee un sensor de movimiento **PIR SR505** con una placa **ESP32-WROOM-32** y muestra por el Monitor Serial un registro de las activaciones: cuándo empezó cada una, cuánto duró y estadísticas acumuladas.

> Proyecto académico — Universidad Nacional de San Antonio Abad del Cusco (UNSAAC).
> Autor: André

---

## Contenido

- [Qué hace](#qué-hace)
- [Materiales](#materiales)
- [Conexión](#conexión)
- [Configuración de Arduino IDE](#configuración-de-arduino-ide)
- [Cómo cargar el programa](#cómo-cargar-el-programa)
- [Cómo funciona el código](#cómo-funciona-el-código)
- [Ejemplo de salida](#ejemplo-de-salida)
- [Solución de problemas](#solución-de-problemas)
- [Notas sobre el SR505](#notas-sobre-el-sr505)
- [Estructura del repositorio](#estructura-del-repositorio)

---

## Qué hace

El SR505 entrega una salida digital de un solo bit: `LOW` sin movimiento y `HIGH` cuando detecta movimiento. Con esa señal, el programa detecta los cambios de estado y, usando `millis()`, calcula:

| Dato | Cómo se obtiene |
|---|---|
| Inicio de la activación | Instante del flanco `LOW → HIGH` |
| Duración de la activación | Tiempo entre el flanco `HIGH → LOW` y el flanco de subida anterior |
| Número de detecciones | Contador de flancos de subida |
| Tiempo entre detecciones | Diferencia entre dos flancos de subida consecutivos |
| Tiempo total activo | Suma de todas las duraciones |
| Porcentaje de actividad | Tiempo total activo / tiempo total transcurrido |

> **Importante:** la duración medida es el tiempo que la salida estuvo en `HIGH`, **no** la duración real del movimiento. Tras detectar, el SR505 mantiene la salida activa unos 8 segundos, por lo que la duración mínima que verás es de aproximadamente 8 s.

---

## Materiales

- Placa **ESP32-WROOM-32**
- Sensor de movimiento **PIR SR505**
- Cable USB (de datos, no solo de carga)
- Cables Dupont y protoboard
- Computador con [Arduino IDE](https://www.arduino.cc/en/software) y el soporte para placas ESP32 instalado

---

## Conexión

| SR505 | ESP32 |
|---|---|
| `VCC` | `5V` (pin **VIN**) |
| `GND` | `GND` |
| `OUT` | `GPIO 27` |

### ⚠️ Advertencias

- **Alimenta el sensor a 5V, no a 3.3V.** El SR505 está diseñado para unos 4.5 V a 20 V; a 3.3 V da detecciones falsas constantes.
- **Verifica la polaridad antes de energizar.** Si inviertes `VCC` y `GND`, el condensador de tantalio del módulo puede calentarse y quemarse.
- **Confirma qué pin es realmente GND.** En algunas placas la serigrafía es confusa y un pin contiguo al de 5V puede no ser tierra, lo que provoca un cortocircuito (reinicios en bucle, `flash read err`, fallos de carga). Revisa el diagrama de pines de tu placa exacta.
- Al poner la placa sobre la protoboard, los nombres de los pines quedan ocultos. Una solución práctica es pegar **tiras de papel con el nombre** de los pines que usas (5V, GND y GPIO 27).
- La salida `OUT` entrega ~3.3 V, por lo que se puede conectar directo al GPIO del ESP32, **sin resistencias adicionales**.

---

## Configuración de Arduino IDE

En **Herramientas**:

| Opción | Valor |
|---|---|
| Placa (Board) | `ESP32 Dev Module` |
| Puerto (Port) | el de tu placa (en este proyecto, `COM5`) |
| Upload Speed | `115200` |
| Flash Mode | `QIO` |
| Flash Frequency | `40 MHz` |

Monitor Serial: **9600 baudios** (debe coincidir con `Serial.begin(9600)` del código).

---

## Cómo cargar el programa

1. **Deja el sensor desconectado** de la placa mientras subes el código.
2. Conecta el ESP32 por USB y pulsa **Subir** en Arduino IDE.
3. Espera a que termine la carga sin errores (`Hard resetting via RTS pin...`).
4. **Desconecta el USB**, conecta el sensor según la tabla de [Conexión](#conexión) y vuelve a energizar la placa.
5. Abre el Monitor Serial a 9600 baudios.

> Si la subida falla, mantén pulsado el botón **BOOT** del ESP32 mientras se conecta, y verifica que el Upload Speed esté en `115200`.

---

## Cómo funciona el código

Archivo: [`PIR_SR505/PIR_SR505.ino`](PIR_SR505/PIR_SR505.ino)

1. En `setup()` configura el pin como entrada y espera **30 s** de calentamiento del sensor.
2. En `loop()` lee el pin cada 50 ms y compara con el estado anterior:
   - **Flanco de subida (`LOW → HIGH`)**: aumenta el contador, guarda el instante de inicio e imprime el tiempo desde la detección anterior.
   - **Flanco de bajada (`HIGH → LOW`)**: calcula la duración, la suma al tiempo total activo e imprime las estadísticas.

```cpp
const int PIR_PIN = 27;

int estadoAnterior = LOW;
unsigned long inicioActivacion = 0;
unsigned long ultimoInicio = 0;
unsigned long tiempoTotalActivo = 0;
int contador = 0;

void setup() {
  Serial.begin(9600);
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
```

---

## Ejemplo de salida

Ejemplo ilustrativo (los valores dependerán de tus movimientos):

```text
Calentando sensor PIR (30 s)...
Sensor listo.
-----------------------------
Detección #1
Inicio: 42.3 s desde el arranque
Fin. Duración de la activación: 8.2 s
Tiempo total activo: 8.2 s (16.2 % del tiempo)
-----------------------------
Detección #2
Inicio: 61.0 s desde el arranque
Tiempo desde la detección anterior: 18.7 s
Fin. Duración de la activación: 9.1 s
Tiempo total activo: 17.3 s (24.6 % del tiempo)
```

### Capturas

Agrega tus imágenes en la carpeta `imagenes/` y estas referencias las mostrarán:

![Conexión del sistema](imagenes/conexion.jpg)
![Salida del Monitor Serial](imagenes/monitor_serial.png)

---

## Solución de problemas

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| Solo aparecen cuadrados o símbolos raros en el Monitor Serial | La velocidad del Monitor no coincide con `Serial.begin()` | Pon el Monitor a **9600** (o cambia ambos a 115200) |
| `Failed to communicate with the flash chip` / `Serial data stream stopped` al subir | Cable USB defectuoso, velocidad de carga alta o sensor conectado durante la carga | Cambia el cable, usa Upload Speed `115200` y **sube el código con el sensor desconectado** |
| El ESP32 se reinicia en bucle (`TG0WDT_SYS_RESET`, `RTCWDT_RTC_RESET`, `flash read err`) al conectar el sensor | Cortocircuito o `GND` conectado a un pin que no es tierra | Revisa el cableado cable por cable y confirma el pin GND en el diagrama de tu placa |
| Detecciones constantes aun sin movimiento | Sensor alimentado a 3.3 V o aún calentándose | Aliméntalo a **5 V** y espera ~30 s tras encender |
| Se imprime "Detección" varias veces por un solo movimiento | El sensor mantiene `HIGH` ~8 s tras detectar | Es el comportamiento normal del SR505 |
| El módulo huele a quemado o el condensador se calienta | Polaridad invertida | **Desconecta de inmediato** y revisa `VCC` y `GND` |

---

## Notas sobre el SR505

- Rango de alimentación: aproximadamente **4.5 V a 20 V**.
- Salida digital de ~3.3 V (`HIGH` con movimiento, `LOW` sin movimiento).
- Tiempo de calentamiento: unos **30 s** tras energizarlo.
- Retardo de salida: ~**8 s** en `HIGH` tras detectar, que se renueva mientras siga habiendo movimiento.
- No indica distancia, dirección ni cantidad de movimiento: solo si hubo o no movimiento.

---

## Estructura del repositorio

```text
.
├── README.md
├── PIR_SR505/
│   └── PIR_SR505.ino
└── imagenes/
    ├── conexion.jpg
    └── monitor_serial.png
```

---

## Licencia

Proyecto con fines académicos. Si lo vas a publicar abiertamente, agrega un archivo `LICENSE` (por ejemplo, MIT) y menciónalo aquí.
