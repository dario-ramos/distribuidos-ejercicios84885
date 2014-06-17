#include "cinta_entrada.h"
#include "configuracion.h"
#include "core.h"
#include "mensaje_debug.h"
#include "t_sem.h"
#include <sys/shm.h>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( m_NombreProceso, Utils::BLANCO, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( m_NombreProceso, fmt, ##__VA_ARGS__ )

CintaEntrada::CintaEntrada( const Configuracion& config, const std::string& nombreProceso ) :
        m_pShm( nullptr ),
        m_NombreProceso( nombreProceso ),
        m_pMutex( new t_sem( ftok( config.ObtenerDirFtok().c_str(),
                                   config.ObtenerIdMutexPlataforma()  ) ) ),
        m_pSemBloquearCinta( new t_sem( ftok(config.ObtenerDirFtok().c_str(),
                                             config.ObtenerIdSemaforoCinta())) ){
    int idShmPlataforma = config.ObtenerIdShmPlataforma();
    int handleShmPlataforma = shmget( ftok(config.ObtenerDirFtok().c_str(),idShmPlataforma),
                                      sizeof(m_pShm), IPC_CREAT | 0660 );
    if( handleShmPlataforma == -1 ){
        MENSAJE_ERROR( "Error al acceder a la Shared Memory de la Plataforma, id %d", idShmPlataforma );
        exit(1);
    }
    m_pShm = static_cast<Robots2::ShmPlataforma*>( shmat(handleShmPlataforma, 0, 0) );
    if( m_pShm == reinterpret_cast<Robots2::ShmPlataforma*>(-1) ){
        MENSAJE_ERROR( "Error al atachear a Shared Memory de la Plataforma, id %d", idShmPlataforma );
        exit(1);
    }
}

CintaEntrada::~CintaEntrada(){
    /*if( shmdt(m_pShm) == -1 ){
        MENSAJE_ERROR( "Error al detachear Shared Memory de la Plataforma" );
        exit(1);
    }*/
}

void CintaEntrada::LanzarProcesoDispositivo( int iDisp ){
    char sNumDisp[8];
    sprintf( sNumDisp, "%d", iDisp );
    int pid;
    if( (pid = fork()) <0 ){
        MENSAJE_ERROR( "Error al hacer fork de proceso %s",
                       Robots2::Constantes::NOMBRE_PROCESO_DISPOSITIVO.c_str() );
        exit(-1);
    }else if( pid == 0 ){//Hijo
        execlp( string("./" + Robots2::Constantes::NOMBRE_PROCESO_DISPOSITIVO).c_str(),
                Robots2::Constantes::NOMBRE_PROCESO_DISPOSITIVO.c_str(), sNumDisp, reinterpret_cast<char*>(0) );
        MENSAJE_ERROR("Proceso %s fallo", Robots2::Constantes::NOMBRE_PROCESO_DISPOSITIVO.c_str());
        exit(-1);
    }
}

void CintaEntrada::GenerarDispositivo( int idDispositivo, int tipoDispositivo ){
    m_pMutex->P();
    int capacidad = m_pShm->Capacidad;
    //Si plataforma llena, esperar que se libere un lugar
    if( m_pShm->EspaciosOcupados == capacidad ){
        m_pMutex->V();
        MENSAJE_DEBUG( "Plataforma llena, cinta detenida" );
        m_pSemBloquearCinta->P();
        return;
    }
    //Colocar dispositivo en primera posicion libre
    for( int i=0; i < capacidad; i++ ){
        if( m_pShm->EstadoDePosiciones[i] != Robots2::EPP_LIBRE )
            continue;
        m_pShm->EstadoDePosiciones[i] = Robots2::EPP_OCUPADA_SIN_ARMAR;
        m_pShm->Dispositivos[i] = idDispositivo;
        m_pShm->TipoDispositivo[i] = tipoDispositivo;
        m_pShm->EspaciosOcupados++;
        m_pShm->DispositivosSinArmar++;
        MENSAJE_DEBUG( "Dispositivo %d de tipo %d colocado en posicion %d, espacios ocupados: %d/%d",
                       idDispositivo, tipoDispositivo, i, m_pShm->EspaciosOcupados, capacidad );
        break;
    }  
    //Lanzar proceso dispositivo
    LanzarProcesoDispositivo( idDispositivo );
    m_pMutex->V();
}
