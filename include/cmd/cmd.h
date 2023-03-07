#include <Arduino.h>

#include <GFXcanvas16T.h>
#include <SSD1283A.h>
#include <test_canvas.h> //Hardware-specific library

#define LF 10
#define CR 13

HardwareSerial &BasculaSP = Serial2;

/*
 ██████╗██╗      █████╗ ███████╗███████╗     ██████╗███╗   ███╗██████╗
██╔════╝██║     ██╔══██╗██╔════╝██╔════╝    ██╔════╝████╗ ████║██╔══██╗
██║     ██║     ███████║███████╗█████╗      ██║     ██╔████╔██║██║  ██║
██║     ██║     ██╔══██║╚════██║██╔══╝      ██║     ██║╚██╔╝██║██║  ██║
╚██████╗███████╗██║  ██║███████║███████╗    ╚██████╗██║ ╚═╝ ██║██████╔╝
 ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝     ╚═════╝╚═╝     ╚═╝╚═════╝

*/

class CMD
{

    /*
    Captura de trama en la consola se muestra de la siguiente froma
    6.0.....
    0.0.....
    trama capturada hexadecimal
    TX  [P]
    RX  [20|20|20|20]  [30|2E|30]  [0D|0A] [00|00|00|00|00]
        [      Identificacion   ]  [ dato ]  [LF CR]
  */

private:
    char const comando = 'P';
    String data;
    float peso;

    void setPeso(float newPeso)
    {
        peso = newPeso;
    }

public:
    //Constructor
    CMD()
    {
        //Inicializacion de variables
        data = "";
        peso = 0;
    }

    float getPeso(void)
    {
        return this->peso;
    }

    int sendCommand()
    {
        data = "";
        // Serial.println("############################");
        // Serial.println(F("Enviano comando[P]"));
        BasculaSP.print(comando);
        delay(5);
        if (BasculaSP.available() > 0)
        {
            while (BasculaSP.available())
            {
                char c = BasculaSP.read();
                data += c;
            }
        }
        else
        {
            //TimeOut
            return -2;
        }

        //Limpiamos los espacios en blanco
        data.trim();

        //Buscamos si la respuesta de la bascula es 0.00
        //Si es asi no se manda a convertir ya que string.toFloat()
        //devuelve un 0.00 si falla la conversion

        //si la lectura de la bascula es 0.00
        if (data.indexOf("0.0") == 0)
        {
            peso = data.toFloat();
            setPeso(peso);
            return 1;
        }
        //Si la lecura de la bascula no es 0.00 y la conversion de data.toFloat() devuelve
        // 0, la conversion ha fallado
        else
        {
            //Conversion a flot
            peso = data.toFloat();
            //Si la conversion retorna un 0, la conversion fallo
            if (peso == 0)
            {
                //La conversion ha fallado, salimos en estado de errro
                return -1;
            }
            //La conversion finalizo con exito, establecemos el peso
            setPeso(peso);
            return 1;
        }

        // Serial.print("Peso[");
        // Serial.print(peso);
        // Serial.println("]");
        // Serial.println("############################");
        // Serial.println();
    }
};

/*
 ██████╗██╗      █████╗ ███████╗███████╗    ██╗      ██████╗██████╗
██╔════╝██║     ██╔══██╗██╔════╝██╔════╝    ██║     ██╔════╝██╔══██╗
██║     ██║     ███████║███████╗█████╗      ██║     ██║     ██║  ██║
██║     ██║     ██╔══██║╚════██║██╔══╝      ██║     ██║     ██║  ██║
╚██████╗███████╗██║  ██║███████║███████╗    ███████╗╚██████╗██████╔╝
 ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝    ╚══════╝ ╚═════╝╚═════╝
*/

// adapt the constructor parameters to your wiring for the appropriate processor conditional,
// or add a new one or adapt the catch all other default

#if (defined(TEENSYDUINO) && (TEENSYDUINO == 147))
// for Mike's Artificial Horizon
SSD1283A tft(/*CS=*/10, /*DC=*/15, /*RST=*/14, /*LED=*/-1); //hardware spi,cs,cd,reset,led

// for my wirings used for e-paper displays:
#elif defined(ESP8266)
SSD1283A tft(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*LED=D2*/ 4); //hardware spi,cs,cd,reset,led
#elif defined(ESP32)
SSD1283A tft(/*CS=5*/ SS, /*DC=*/4, /*RST=*/2, /*LED=*/4); //hardware spi,cs,cd,reset,led
#elif defined(_BOARD_GENERIC_STM32F103C_H_)
SSD1283A tft(/*CS=4*/ SS, /*DC=*/3, /*RST=*/2, /*LED=*/1); //hardware spi,cs,cd,reset,led
#elif defined(__AVR)
SSD1283A tft(/*CS=10*/ SS, /*DC=*/8, /*RST=*/9, /*LED=*/7); //hardware spi,cs,cd,reset,led
#else
// catch all other default
SSD1283A tft(/*CS=10*/ SS, /*DC=*/8, /*RST=*/9, /*LED=*/7); //hardware spi,cs,cd,reset,led
#endif

#if !defined(ESP8266)
#define yield()
#endif

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// Clase LCD
class LCD
{

public:
    /*
╔═╗╔═╗╔╗╔╔═╗╔╦╗╦═╗╦ ╦╔═╗╔╦╗╔═╗╦═╗
║  ║ ║║║║╚═╗ ║ ╠╦╝║ ║║   ║ ║ ║╠╦╝
╚═╝╚═╝╝╚╝╚═╝ ╩ ╩╚═╚═╝╚═╝ ╩ ╚═╝╩╚═
*/
    LCD()
    {
        /*Inicializa el LCD*/
        tft.init();
        Serial.println("tft.init() done");
        /*Test de pantalla*/
        testFillScreen();
        /*Rotate screen*/
        tft.setRotation(2);
        setScreenHome();
    }

    /* Plantilla inicial*/
    void setScreenHome()
    {
        /*Dibuja el fondo de apntalla*/
        fillBackground(BLACK);
        /*Inicio de la seccion wifi*/
        seccionWifiBg();
        setTextWifi("Wifi: OFF");
        /*inicio de la seccion consola*/
        seccionConsolaBg();
        setTextConsole("Alimexa Network", "0.0.0.0", 0);
        /*Inicio de la seccion motoe */
        seccionMotorBg(RED);
        setTextMotor("OFF");
        //Set texto peso
        setTextPeso(0, 0);
    }

    /*
╔╦╗╔═╗═╗ ╦╔╦╗╔═╗╔═╗
 ║ ║╣ ╔╩╦╝ ║ ║ ║╚═╗
 ╩ ╚═╝╩ ╚═ ╩ ╚═╝╚═╝
*/
    //Establecer texto de la seccion wifi
    void setTextWifi(String text)
    {
        tft.setCursor(2, 2);
        tft.setTextColor(BLACK);
        tft.setTextSize(2);
        tft.println(text);
    }

    //Establecer texto de la seccion consola
    void setTextConsole(String red, String ip, int contador)
    {
        seccionConsolaBg();
        //Tipo de letra para consola
        tft.setCursor(2, 22);
        tft.setTextColor(WHITE);
        tft.setTextSize(1);
        tft.println(red);
        tft.println(ip);
        tft.setTextSize(2);
        tft.setTextColor(GREEN);
        tft.print("TAMBOS:");
        tft.println(contador);
    }

    //Establecer texto de la seccion motor
    void setTextMotor(String estado)
    {
        //Motor ON
        tft.setCursor(40, 65);
        tft.setTextColor(WHITE);
        tft.setTextSize(4);
        tft.print(estado);
    }

    //Establece texto en la seccion del motor Size 2
    void setTextMotor3(String estado)
    {
        //Motor ON
        tft.setCursor(10, 65);
        tft.setTextColor(BLACK);
        tft.setTextSize(2);
        tft.print(estado);
    }

    // Establece el texto del peso
    void setTextPeso(float peso, bool onOff)
    {
        //Textto Peso

        onOff ? seccionMotorBg(GREEN) : seccionMotorBg(RED);
        onOff ? setTextMotor("ON") : setTextMotor("OFF");
        tft.setCursor(30, 100);
        onOff ? tft.fillRect(30, 100, 100, 16, GREEN) : tft.fillRect(30, 100, 100, 16, RED);
        tft.setTextColor(BLACK);
        tft.setTextSize(2);
        tft.print(peso);
        tft.print("KG");
    }

    /*
╔╗ ╔═╗╔═╗╦╔═╔═╗╦═╗╔═╗╦ ╦╔╗╔╔╦╗╔═╗
╠╩╗╠═╣║  ╠╩╗║ ╦╠╦╝║ ║║ ║║║║ ║║╚═╗
╚═╝╩ ╩╚═╝╩ ╩╚═╝╩╚═╚═╝╚═╝╝╚╝═╩╝╚═╝
*/
    //Dibujado de fondo de la pantalla
    void fillBackground(uint16_t color)
    {
        tft.fillScreen(color);
    }

    //Fondo de la seccion wifi
    void seccionWifiBg()
    {
        //Dibujo de seccion de estado
        tft.fillRect(0, 0, 130, 20, YELLOW);
    }

    //Fondo de la seccion de consola
    void seccionConsolaBg()
    {
        //Dibujo de la seccion de consola 35*130px
        tft.fillRect(0, 20, 130, 35, BLUE);
    }

    //Fondo del indicador del motor
    void seccionMotorBg(uint16_t color)
    {
        //funcionamiento del motor 75*130 px
        tft.fillRect(0, 55, 130, 75, color);
    }

    void testFillScreen()
    {
        tft.fillScreen(BLACK);
        yield();
        delay(200);
        //if (controller.ID == 0x8875) delay(200); // too fast to be seen
        tft.fillScreen(RED);
        yield();
        delay(200);
        //if (controller.ID == 0x8875) delay(200); // too fast to be seen
        tft.fillScreen(GREEN);
        yield();
        delay(200);
        //if (controller.ID == 0x8875) delay(200); // too fast to be seen
        tft.fillScreen(BLUE);
        yield();
        delay(200);
        //if (controller.ID == 0x8875) delay(200); // too fast to be seen
        tft.fillScreen(BLACK);
        yield();
        delay(200);
    }

    void seccionCfg()
    {
        tft.fillRect(5, 5, 120, 120, BLACK);
    }

    void setHeaderCfg(String peso)
    {
        tft.setCursor(6, 6);
        tft.setTextColor(YELLOW);
        tft.setTextSize(2);
        tft.println("  CONFIG");
        tft.println(" PESO MAX:");
        tft.print(" ");
        tft.println(peso);
        tft.setTextColor(GREEN);
        tft.println(" NUEVO");
        tft.println(" PESO:");
        tft.fillRect(5, 90, 120, 20, WHITE);
        tft.setCursor(6, 92);
        tft.setTextColor(BLACK);
    }

    void setNuevoPeso(char *c)
    {
        setCursorPeso();
        tft.print(c);
    }

    void clearNuevoPeso()
    {
        tft.fillRect(5, 90, 120, 20, WHITE);
        setCursorPeso();
    }

    void setCursorPeso()
    {
        tft.setCursor(6, 92);
    }

    /*
    ╔═╗╔═╗╔╗╔╔╦╗╔═╗╦  ╦  ╔═╗  ╔╦╗╔═╗  ╔═╗╔═╗╔╗╔╔═╗╦╔═╗╦ ╦╦═╗╔═╗╔═╗╦╔═╗╔╗╔
    ╠═╝╠═╣║║║ ║ ╠═╣║  ║  ╠═╣   ║║║╣   ║  ║ ║║║║╠╣ ║║ ╦║ ║╠╦╝╠═╣║  ║║ ║║║║
    ╩  ╩ ╩╝╚╝ ╩ ╩ ╩╩═╝╩═╝╩ ╩  ═╩╝╚═╝  ╚═╝╚═╝╝╚╝╚  ╩╚═╝╚═╝╩╚═╩ ╩╚═╝╩╚═╝╝╚╝
    */
    void screenCfg(String peso)
    {
        //Background
        fillBackground(BLUE);
        //Dibujado de seccion de configuracion
        seccionCfg();
        //Texto de configuracion
        setHeaderCfg(peso);
    }

    //Pantalla Warning
    void setWarningScreen()
    {
        seccionMotorBg(YELLOW);
        setTextMotor3("BASCULA DESCONECTADA");
    }
};