#pragma once

class IDispositivo{
public:
    inline virtual ~IDispositivo() = 0;    
    virtual int Empacar() = 0;
    virtual int EsperarDespacho() = 0; //Devuelve id del robot que realiza despacho
    virtual int EsperarInicioArmado() = 0; //Devuelve id del robot que realiza armado
    virtual void IniciarArmado( int idRobot ) = 0;
    virtual void Activar() = 0;
    virtual void Despachar( int idRobot ) = 0;
    virtual void EsperarFinActivacion() = 0;
    virtual void EsperarFinArmado() = 0;
    virtual void FinalizarArmado() = 0;
};

IDispositivo::~IDispositivo(){} 
 
