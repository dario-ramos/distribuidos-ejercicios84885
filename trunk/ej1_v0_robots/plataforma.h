#pragma once

#include "iplataforma_armado.h"
#include "iplataforma_despacho.h"

class Configuracion;

class Plataforma : public IPlataformaArmado, public IPlataformaDespacho{
      const int         m_ColorMensajes;
      int               m_ColaPlataforma;
      std::string       m_NombreProceso;
public:
      Plataforma( const Configuracion& config, const std::string& nombreProceso, int colorMensajes );
      virtual ~Plataforma();
      virtual bool Llena( int numeroRobot ) const;
      virtual int DetectarFrecuenciaActivacion( int numeroRobot );
      virtual void DespacharDispositivo( int numeroRobot, int posDisp );
      virtual void FinalizarArmado( int numeroRobot );
      virtual void IniciarArmado( int numeroRobot );
};
