#pragma once

#include "idispositivo.h"
#include <string>

class Configuracion;

class Dispositivo : public IDispositivo{
    int                 m_Cola;
    int                 m_Id;
    std::pair<int,int>  m_DemoraActivacion;
    std::string         m_NombreProceso;
public:
    Dispositivo( int id, const Configuracion& config, const std::string& nombreProceso );
    virtual ~Dispositivo();
    virtual int EsperarDespacho();
    virtual int EsperarEmpaque();
    virtual int EsperarInicioArmado();
    virtual void IniciarArmado( int idRobot );
    virtual void Activar();
    virtual void Despachar( int idRobot );
    virtual void Empacar( int idRobot );
    virtual void EsperarFinActivacion();
    virtual void EsperarFinArmado();
    virtual void FinalizarArmado();
};
