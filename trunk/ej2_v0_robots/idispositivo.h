#pragma once

class IDispositivo{
public:
    inline virtual ~IDispositivo() = 0;    
    virtual int EsperarDespacho() = 0; //Devuelve id del robot que realiza despacho
    virtual int EsperarEmpaque() = 0; //Devuelve id del robot que realiza empaque
    virtual int EsperarInicioArmado() = 0; //Devuelve id del robot que realiza armado
    virtual void IniciarArmado( int idRobot ) = 0;
    virtual void Activar() = 0;
    virtual void Despachar( int idRobot ) = 0;
    virtual void Empacar( int idRobot ) = 0;
    virtual void EsperarFinActivacion() = 0;
    virtual void EsperarFinArmado() = 0;
    virtual void FinalizarArmado() = 0;
};

IDispositivo::~IDispositivo(){} 
 
