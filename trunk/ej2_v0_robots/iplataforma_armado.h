#pragma once

class IPlataformaArmado{
public:
      inline virtual ~IPlataformaArmado() = 0;
      virtual bool Llena( int numeroRobot ) const = 0;
      virtual void IniciarArmado( int numeroRobot ) = 0;
      virtual void FinalizarArmado( int numeroRobot ) = 0;
};

IPlataformaArmado::~IPlataformaArmado(){}
