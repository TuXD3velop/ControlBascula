
/* Modified by Bodmer to be an example for TFT_HX8357 library.
   This sketch uses the GLCD font only.

   The performance for each test is reported to the serial
   port at 38400 baud.

   This test occupies the whole of the display therefore
   will take twice as long as it would on a 320 x 240
   display. Bear this in mind when making performance
   comparisons.

   Make sure all the required font is loaded by editting the
   User_Setup.h file in the TFT_HX8357 library folder.

   Original header is at the end of the sketch, some text in it is
   not applicable to the HX8357 display supported by this example.
*/

// modified by Jean-Marc Zingg to be an example for the SSD1283A library (from GxTFT library)
// original source taken from https://github.com/Bodmer/TFT_HX8357

#include <string.h>
#include <WiFi.h>
//#include <BluetoothSerial.h>
#include <Preferences.h>
#include <Keypad.h>


#include "motor/motor.h"
#include "cmd/cmd.h"

//Pins para serial2
#define RXD2 16
#define TXD2 17

//#define BT_DEBUG

/*
██████╗ ███████╗ ██████╗██╗      █████╗ ██████╗  █████╗  ██████╗██╗ ██████╗ ███╗   ██╗███████╗███████╗
██╔══██╗██╔════╝██╔════╝██║     ██╔══██╗██╔══██╗██╔══██╗██╔════╝██║██╔═══██╗████╗  ██║██╔════╝██╔════╝
██║  ██║█████╗  ██║     ██║     ███████║██████╔╝███████║██║     ██║██║   ██║██╔██╗ ██║█████╗  ███████╗
██║  ██║██╔══╝  ██║     ██║     ██╔══██║██╔══██╗██╔══██║██║     ██║██║   ██║██║╚██╗██║██╔══╝  ╚════██║
██████╔╝███████╗╚██████╗███████╗██║  ██║██║  ██║██║  ██║╚██████╗██║╚██████╔╝██║ ╚████║███████╗███████║
╚═════╝ ╚══════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝╚══════╝
*/

void IRAM_ATTR externISR();

void keypadEvent(KeypadEvent key);

/*
 ██████╗ ██╗      ██████╗ ██████╗  █████╗ ██╗     ███████╗███████╗
██╔════╝ ██║     ██╔═══██╗██╔══██╗██╔══██╗██║     ██╔════╝██╔════╝
██║  ███╗██║     ██║   ██║██████╔╝███████║██║     █████╗  ███████╗
██║   ██║██║     ██║   ██║██╔══██╗██╔══██║██║     ██╔══╝  ╚════██║
╚██████╔╝███████╗╚██████╔╝██████╔╝██║  ██║███████╗███████╗███████║
 ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝

*/
//Control de encendido
bool motorEncendido = false;
bool motorEstadoAnterior = false;
float pesoAnterior = 0.0;
uint8_t contadorErrores = 0;
//BluetoothSerial SerialBT;

//Peso a comprar
float pesoCompleto = 20;

//Contador tambos
int contadorTambos = 0;

/*
╦╔╗╔╔═╗╔╦╗╔═╗╔╗╔╔═╗╦╔═╗  ╔╦╗╔═╗╔╦╗╔═╗╦═╗
║║║║╚═╗ ║ ╠═╣║║║║  ║╠═╣  ║║║║ ║ ║ ║ ║╠╦╝
╩╝╚╝╚═╝ ╩ ╩ ╩╝╚╝╚═╝╩╩ ╩  ╩ ╩╚═╝ ╩ ╚═╝╩╚═
*/
// Intsancia de la clase que controla el motor
Motor motor = Motor(27 /*Puerto Inicio*/, 26 /*Puerto Parada*/, 34 /*Puerto Input signal*/, false /*Active Low*/);

/*
╦╔╗╔╔═╗╔╦╗╔═╗╔╗╔╔═╗╦╔═╗  ╦  ╔═╗╔╦╗
║║║║╚═╗ ║ ╠═╣║║║║  ║╠═╣  ║  ║   ║║
╩╝╚╝╚═╝ ╩ ╩ ╩╝╚╝╚═╝╩╩ ╩  ╩═╝╚═╝═╩╝
*/
/*Instancia de la clase LCD*/
LCD lcd = LCD();

/*
╦╔╗╔╔═╗╔╦╗╔═╗╔╗╔╔═╗╦╔═╗  ╔═╗╔═╗╔═╗
║║║║╚═╗ ║ ╠═╣║║║║  ║╠═╣  ║  ╠╣ ║ ╦
╩╝╚╝╚═╝ ╩ ╩ ╩╝╚╝╚═╝╩╩ ╩  ╚═╝╚  ╚═╝
*/
/* Configuracion */
Preferences cfg;

/*
╦╔═╔═╗╦ ╦╔═╗╔═╗╔╦╗  ╔═╗╔═╗╔╗╔╔═╗╦╔═╗
╠╩╗║╣ ╚╦╝╠═╝╠═╣ ║║  ║  ║ ║║║║╠╣ ║║ ╦
╩ ╩╚═╝ ╩ ╩  ╩ ╩═╩╝  ╚═╝╚═╝╝╚╝╚  ╩╚═╝
*/
/*Configuracion */
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', '.'}};
byte rowPins[ROWS] = {13, 12, 14, 25}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {33, 32, 22, 19}; //connect to the column pinouts of the kpd
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/*
███████╗███████╗████████╗██╗   ██╗██████╗
██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
███████╗█████╗     ██║   ██║   ██║██████╔╝
╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
███████║███████╗   ██║   ╚██████╔╝██║
╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝
*/

void setup()
{

  const char *ssid = "Terminales 2";
  const char *password = "TestTerminal";

  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  //Setup serial 2
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //Interrupcion externa
  attachInterrupt(34, externISR, FALLING);

#ifdef BT_DEBUG
  //Setup Bluetooth serial
  SerialBT.begin("AlimexaC001");
  Serial.println("Setup SerialBT OK");
  Serial.println("Motor Estado inicial");
  SerialBT.println("Motor Estado inicial");
  delay(8000);
#endif

  /*
╔═╗╔═╗╔╦╗╦ ╦╔═╗  ╔═╗╔═╗╔═╗
╚═╗║╣  ║ ║ ║╠═╝  ║  ╠╣ ║ ╦
╚═╝╚═╝ ╩ ╚═╝╩    ╚═╝╚  ╚═╝
*/

//! el guardado en NVS no funcoina
/*
  cfg.begin("bascula", false);
  pesoCompleto = cfg.getFloat("peso_max", 120);
  contadorTambos = cfg.getInt("contador", 0);
  */

  /*
╔═╗╔═╗╔╦╗╦ ╦╔═╗  ╦ ╦╦╔═╗╦
╚═╗║╣  ║ ║ ║╠═╝  ║║║║╠╣ ║
╚═╝╚═╝ ╩ ╚═╝╩    ╚╩╝╩╚  ╩
*/

  WiFi.begin(ssid, password);
  Serial.print(F("Conectando"));

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print(F("Conectado a la red WiFi: "));
  Serial.println(WiFi.localIP());

  /*
  ╔═╗╦  ╦╔═╗╔╗╔╔╦╗╔═╗  ╦╔═╔═╗╦ ╦╔═╗╔═╗╔╦╗
  ║╣ ╚╗╔╝║╣ ║║║ ║ ║ ║  ╠╩╗║╣ ╚╦╝╠═╝╠═╣ ║║
  ╚═╝ ╚╝ ╚═╝╝╚╝ ╩ ╚═╝  ╩ ╩╚═╝ ╩ ╩  ╩ ╩═╩╝
  */

  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
}

/*
██╗      ██████╗  ██████╗ ██████╗
██║     ██╔═══██╗██╔═══██╗██╔══██╗
██║     ██║   ██║██║   ██║██████╔╝
██║     ██║   ██║██║   ██║██╔═══╝
███████╗╚██████╔╝╚██████╔╝██║
╚══════╝ ╚═════╝  ╚═════╝ ╚═╝
*/

void loop(void)
{

#ifdef BT_DEBUG
  //Testeo de relevadores, inicio y paro del motor
  // Serial.println("Motor Encendido");
  // SerialBT.println("Motor Encendido");
  // lcd.seccionMotorBg(GREEN);
  // lcd.setTextMotor("ON");
  // motor.iniciar();
  // delay(8000);
  // Serial.println("Motor Apagado");
  // SerialBT.println("Motor Apagado");
  // lcd.seccionMotorBg(RED);
  // lcd.setTextMotor("OFF");
  // motor.parar();
  // delay(8000);

  //Estado del teclado
  char key = keypad.getKey();

  delay(200);

#else

  float pesoActual = 0;
  //Instancia de comandos para manejar la bascula
  CMD Bascula;

  //checamos si la respuesta del comando es valida
  int resultadoCmd = Bascula.sendCommand();
  if (resultadoCmd == 1)
  {
    //Reseteo de contador de errores
    contadorErrores = 0;
    //Verificacion de peso
    pesoActual = Bascula.getPeso();
    if (pesoActual != pesoAnterior)
    {
      Serial.print("Peso: ");
      Serial.print(pesoActual);
      Serial.print(" --> ");
      Serial.println(pesoCompleto);
      pesoAnterior = pesoActual;
      lcd.setTextPeso(pesoActual, motor.getMotorState());
    }

    //Indicacion de encendido demotor
    if (motorEncendido)
    {
      //Iniciar el motor
      motor.iniciar();
      lcd.seccionMotorBg(GREEN);
      lcd.setTextMotor("ON");
      motorEncendido = false;
      Serial.print("Estado del motor: ");
      Serial.println(motor.getMotorState() ? "Encendido" : "Apagado");
    }

    //LLenado en proceso
    if ((motor.getMotorState()) && (pesoActual >= pesoCompleto))
    {
      motor.parar();
      lcd.seccionMotorBg(RED);
      lcd.setTextMotor("OFF");
      contadorTambos++;
      cfg.putInt("contador", contadorTambos);
      lcd.setTextConsole("Alimexa Network", "0.0.0.0", contadorTambos);
      motorEstadoAnterior = false;
      Serial.print("Estado del motor: ");
      Serial.println(motor.getMotorState() ? "Encendido" : "Apagado");
    }
  }
  else if (resultadoCmd == -1)
  {
    contadorErrores++;
    Serial.println("La conversion ha fallado");
    if (contadorErrores > 10)
    {
      lcd.setWarningScreen();
    }
  }
  else if (resultadoCmd == -2)
  {
    contadorErrores++;
    Serial.println("Time out, Bascula desconectada");

    if (contadorErrores > 10)
    {
      lcd.setWarningScreen();
    }
  }

  //Estado del teclado
  char key = keypad.getKey();

  if (key)
  {
    Serial.println(key);
  }

  delay(500);
#endif
}

/*
██████╗ ███████╗███████╗██╗███╗   ██╗██╗ ██████╗██╗ ██████╗ ███╗   ██╗███████╗███████╗
██╔══██╗██╔════╝██╔════╝██║████╗  ██║██║██╔════╝██║██╔═══██╗████╗  ██║██╔════╝██╔════╝
██║  ██║█████╗  █████╗  ██║██╔██╗ ██║██║██║     ██║██║   ██║██╔██╗ ██║█████╗  ███████╗
██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██║██║     ██║██║   ██║██║╚██╗██║██╔══╝  ╚════██║
██████╔╝███████╗██║     ██║██║ ╚████║██║╚██████╗██║╚██████╔╝██║ ╚████║███████╗███████║
╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚═╝ ╚═════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝╚══════╝
*/

bool salirCfg = false;
// Taking care of some special events.


/*
███████╗██╗   ██╗███████╗███╗   ██╗████████╗ ██████╗     ██╗  ██╗███████╗██╗   ██╗██████╗  █████╗ ██████╗
██╔════╝██║   ██║██╔════╝████╗  ██║╚══██╔══╝██╔═══██╗    ██║ ██╔╝██╔════╝╚██╗ ██╔╝██╔══██╗██╔══██╗██╔══██╗
█████╗  ██║   ██║█████╗  ██╔██╗ ██║   ██║   ██║   ██║    █████╔╝ █████╗   ╚████╔╝ ██████╔╝███████║██║  ██║
██╔══╝  ╚██╗ ██╔╝██╔══╝  ██║╚██╗██║   ██║   ██║   ██║    ██╔═██╗ ██╔══╝    ╚██╔╝  ██╔═══╝ ██╔══██║██║  ██║
███████╗ ╚████╔╝ ███████╗██║ ╚████║   ██║   ╚██████╔╝    ██║  ██╗███████╗   ██║   ██║     ██║  ██║██████╔╝
╚══════╝  ╚═══╝  ╚══════╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝     ╚═╝  ╚═╝╚══════╝   ╚═╝   ╚═╝     ╚═╝  ╚═╝╚═════╝

*/
void keypadEvent(KeypadEvent key)
{
  switch (keypad.getState())
  {
  case PRESSED:
    if (key == '#')
    {
      Serial.println("Event Presed # [START]");
    }else if(key == '*')
    {
      Serial.println("Event Presed * [STOP]");
    }
    break;

  case RELEASED:
    if (key == '*')
    {
      Serial.println("Event Released *");
    }
    else if(key == '*')
    {
      Serial.println("Event Released *");
    }
    break;

  case HOLD:

    // if (key == '#')
    // {
    //   salirCfg = true;
    //   lcd.clearNuevoPeso();
    // }

    //Apagar el motor al entar a la configuracion
    if (motor.getMotorState())
    {
      motor.parar();
    }
    //Hold A => "F4"
    if (key == 'A')
    {
      salirCfg = false;
      //float peso = cfg.getFloat("peso_max", 0);
      Serial.println(pesoCompleto);
      lcd.screenCfg(String(pesoCompleto));
      //Limpiando la matriz
      char pesoIngresado[] = {'\0', '\0', '\0', '\0', '\0', '\0'};
      uint8_t i = 0;
      /* Esperamos el ingreso del nuevo peso*/
      do
      {
        char key = keypad.getKey();
        if (key)
        {
          //Limitamos la entrada a los digitos, start y stop
          if (key >= 46 && key <= 57)
          {
            pesoIngresado[i] = key;
            lcd.setNuevoPeso(pesoIngresado);
            //Serial.println(key);
            Serial.println(pesoIngresado);
            i++;
          }
          // Stop CANCELAR
          if (key == '#')
          {
            Serial.println("Borrar");
            lcd.clearNuevoPeso();
            for (uint8_t i = 0; i <= 5; i++)
            {
              pesoIngresado[i] = '\0';
            }

            i = 0;
          }
          // Start GUARDAR
          if (key == '*')
          {
            Serial.println("Guardar Cambios");
            String buffer = "";
            buffer.concat(pesoIngresado);
            float temp = buffer.toFloat();
            Serial.print("Nuevo Peso: ");
            Serial.println(temp);
            //cfg.putFloat("peso_max", temp);
            //pesoCompleto = cfg.getFloat("peso_max", 120);
            pesoCompleto = temp;

            if (temp > 0)
            {
              Serial.println("Cambios guardados.");
              lcd.setScreenHome();
              salirCfg = true;
            }
            else
            {
              Serial.println("Error El peso no puede ser cero");
              salirCfg = false;
            }
          }
        }
        delay(100);
      } while (salirCfg == false);
    }
    break;
  }
}

/*
██╗███╗   ██╗████████╗███████╗██████╗ ██████╗ ██╗   ██╗██████╗  ██████╗██╗ ██████╗ ███╗   ██╗███████╗███████╗
██║████╗  ██║╚══██╔══╝██╔════╝██╔══██╗██╔══██╗██║   ██║██╔══██╗██╔════╝██║██╔═══██╗████╗  ██║██╔════╝██╔════╝
██║██╔██╗ ██║   ██║   █████╗  ██████╔╝██████╔╝██║   ██║██████╔╝██║     ██║██║   ██║██╔██╗ ██║█████╗  ███████╗
██║██║╚██╗██║   ██║   ██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔═══╝ ██║     ██║██║   ██║██║╚██╗██║██╔══╝  ╚════██║
██║██║ ╚████║   ██║   ███████╗██║  ██║██║  ██║╚██████╔╝██║     ╚██████╗██║╚██████╔╝██║ ╚████║███████╗███████║
╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝      ╚═════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝╚══════╝
*/

/* Interrupcion externa, pin 34*/
void IRAM_ATTR externISR()
{
  /*Retardo antirrebote*/
  cli();
  //delay(100);
  motorEncendido = true;

  sei();
}
