#pragma once

class IDispositivo{
public:
    inline virtual ~IDispositivo() = 0;
    virtual int Despachar() = 0;
    virtual int Empacar() = 0;
    virtual int IniciarArmado() = 0;
    virtual void Activar() = 0;
    virtual void FinalizarArmado() = 0;
};

IDispositivo::~IDispositivo(){} 
 
