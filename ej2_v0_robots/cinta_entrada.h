#pragma once

#include "icinta_entrada.h"
#include <memory>
#include <string>

namespace Robots2{ struct ShmPlataforma;  }
class Configuracion;
class t_sem;

class CintaEntrada : public ICintaEntrada{
    Robots2::ShmPlataforma* m_pShm;
    std::string             m_NombreProceso;
    std::unique_ptr<t_sem>  m_pMutex;
    std::unique_ptr<t_sem>  m_pSemBloquearCinta;
//Metodos auxiliares
    void LanzarProcesoDispositivo( int iDisp );
public:
    CintaEntrada( const Configuracion& config, const std::string& nombreProceso );
    virtual ~CintaEntrada();
    virtual void GenerarDispositivo( int idDispositivo, int tipoDispositivo );
};
