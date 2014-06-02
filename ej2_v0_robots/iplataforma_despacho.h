#pragma once

class IPlataformaDespacho{
public:
      inline virtual ~IPlataformaDespacho() = 0;
      //Devuelve la posicion del dispositivo en la plataforma
      virtual int DetectarFrecuenciaActivacion( int numeroRobot ) = 0;
      virtual void DespacharDispositivo( int numeroRobot, int posDisp ) = 0;
};

IPlataformaDespacho::~IPlataformaDespacho(){}
