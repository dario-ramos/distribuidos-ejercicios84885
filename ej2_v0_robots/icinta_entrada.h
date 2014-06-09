#pragma once

class ICintaEntrada{
public:
    virtual void GenerarDispositivo( int idDispositivo, int tipoDispositivo ) = 0;
    inline virtual ~ICintaEntrada() = 0;
};

ICintaEntrada::~ICintaEntrada(){}
