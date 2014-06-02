#pragma once

#include "idispositivo.h"

class Configuracion;

class Dispositivo : public IDispositivo{
public:
    Dispositivo( const Configuracion& config );
    virtual ~Dispositivo();
    virtual int Despachar();
    virtual int Empacar();
    virtual int IniciarArmado();
    virtual void Activar();
    virtual void FinalizarArmado();
};
