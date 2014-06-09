#pragma once

class IRobotEmpaque{
public:
    inline virtual ~IRobotEmpaque() = 0;
    virtual void EmpacarDispositivo() = 0;
    virtual void IniciarEmpaqueDeDispositivo( int idDisp ) = 0;
};

IRobotEmpaque::~IRobotEmpaque(){} 
