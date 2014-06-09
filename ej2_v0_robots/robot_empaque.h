#pragma once

#include "irobot_empaque.h"
#include <string>

class Configuracion;

class RobotEmpaque : public IRobotEmpaque{
    const Configuracion&    m_Config;
    const int               m_Id;
    const std::string       m_NombreProceso;
    int                     m_Cola;
    std::pair<int,int>      m_DemoraEmpaque;
public:
    RobotEmpaque( int idRobot, const std::string& nombreProceso, const Configuracion& config );
    virtual ~RobotEmpaque();
    virtual void EmpacarDispositivo();
    virtual void IniciarEmpaqueDeDispositivo( int idDisp );
}; 
