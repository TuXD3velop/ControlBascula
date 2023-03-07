#include <Arduino.h>

#define RETARDO 1000 //Retardo para encendido y apagado del motor
#define ENCENDIDO true
#define APAGADO false

/*
 ██████╗██╗      █████╗ ███████╗███████╗    ███╗   ███╗ ██████╗ ████████╗ ██████╗ ██████╗
██╔════╝██║     ██╔══██╗██╔════╝██╔════╝    ████╗ ████║██╔═══██╗╚══██╔══╝██╔═══██╗██╔══██╗
██║     ██║     ███████║███████╗█████╗      ██╔████╔██║██║   ██║   ██║   ██║   ██║██████╔╝
██║     ██║     ██╔══██║╚════██║██╔══╝      ██║╚██╔╝██║██║   ██║   ██║   ██║   ██║██╔══██╗
╚██████╗███████╗██║  ██║███████║███████╗    ██║ ╚═╝ ██║╚██████╔╝   ██║   ╚██████╔╝██║  ██║
 ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝    ╚═╝     ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═╝

*/

/*Clase para control del inicio y parado del motor*/
class Motor
{
private:
    //estado del motor
    bool on_off;
    bool _activeLevel;
    uint8_t _portOn;
    uint8_t _portOff;
    uint8_t _portInput;

    void setMotorState(bool estado)
    {
        on_off = estado;
    }

    //Send Engine on sognal
    void motorOn()
    {
        /*Active High*/
        if (_activeLevel)
        {
            digitalWrite(_portOn, HIGH);
            delay(RETARDO);
            digitalWrite(_portOn, LOW);
        }
        else
        {
            /*Active LOW*/
            digitalWrite(_portOn, LOW);
            delay(RETARDO);
            digitalWrite(_portOn, HIGH);
        }
    }

    //Send Engine off sognal
    void motorOff()
    {
        /*ACtive High*/
        if (_activeLevel)
        {
            digitalWrite(_portOff, LOW);
            delay(RETARDO);
            digitalWrite(_portOff, HIGH);
        }
        else
        {
            /*Active LOW*/
            digitalWrite(_portOff, HIGH);
            delay(RETARDO);
            digitalWrite(_portOff, LOW);
        }
    }

public:
    /*
╔═╗╔═╗╔╗╔╔═╗╔╦╗╦═╗╦ ╦╔═╗╔╦╗╔═╗╦═╗
║  ║ ║║║║╚═╗ ║ ╠╦╝║ ║║   ║ ║ ║╠╦╝
╚═╝╚═╝╝╚╝╚═╝ ╩ ╩╚═╚═╝╚═╝ ╩ ╚═╝╩╚═
*/
    Motor(uint8_t portOn, uint8_t portOff, uint8_t portInput, bool activeLevel)
    {
        _portOn = portOn;
        _portOff = portOff;
        _activeLevel = activeLevel;
        pinMode(_portOn, OUTPUT);
        pinMode(_portOff, OUTPUT);
        pinMode(portInput, INPUT);

        //Estado inicial del motor
        if (_activeLevel)
        {
            digitalWrite(_portOff, HIGH);
            digitalWrite(_portOn, LOW);
        }
        else
        {
            digitalWrite(_portOff, LOW);
            digitalWrite(_portOn, HIGH);
        }

        setMotorState(APAGADO);
    }

    //Retorna el estado del motor
    bool getMotorState(void)
    {
        return on_off;
    }

    //Iniciar el motot
    void iniciar()
    {
        //Comprobamos si el motor ya se encuentra encendido ya no se debe
        //volver a activar el relevador
        if (getMotorState())
            return;
        //Activacion del relevador
        motorOn();
        setMotorState(ENCENDIDO);
    }

    //Detener el motor
    void parar()
    {
        //Si el motor ya ha sido apagado retornamos
        if (!getMotorState())
            return;
        //Si el motor esta encendido, procede a apagarlo
        motorOff();
        setMotorState(APAGADO);
    }
};
