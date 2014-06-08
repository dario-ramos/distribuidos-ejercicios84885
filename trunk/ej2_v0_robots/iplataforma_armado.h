#pragma once

class IPlataformaArmado{
public:
      inline virtual ~IPlataformaArmado() = 0;
      virtual bool Vacia( int numeroRobot ) const = 0;
      virtual int IniciarArmado( int numeroRobot ) = 0; //Devuelve el id del dispositivo cuyo armado se iniciara
      virtual void FinalizarArmado( int numeroRobot ) = 0;
};

IPlataformaArmado::~IPlataformaArmado(){}
