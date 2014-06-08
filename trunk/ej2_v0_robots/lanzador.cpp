#include "core.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include "t_sem.h"
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( "LANZADOR", Utils::BLANCO, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( "LANZADOR", fmt, ##__VA_ARGS__ )

void LanzarProcesosDePlataforma( int cantRobots ){
      for( int i=1; i<=cantRobots; i++ ){
            char sPlat[8];
            sprintf( sPlat, "%d", i );
            int pid;
            if( (pid = fork()) <0 ){
                  MENSAJE_ERROR("Error al hacer fork de control_plataforma %d", i);
                  exit(1);
            }else if( pid == 0 ){//Hijo
                  execlp( string("./" + Robots2::Constantes::NOMBRE_PROCESO_PLATAFORMA).c_str(),
                        Robots2::Constantes::NOMBRE_PROCESO_PLATAFORMA.c_str(), sPlat, reinterpret_cast<char*>(0) );
                  MENSAJE_ERROR("Proceso plataforma fallo");
                  exit(1);
            }
      }
}

void InicializarSemaforo( const int idSem, const string& dirFtok, const int valorInicial ){
    int clave = ftok(dirFtok.c_str(),idSem);
    t_sem sem( clave );
    if( !sem.Existe() ){
        MENSAJE_ERROR("Error al crear Semaforo %d", idSem);
        exit(1);
    }else{
        MENSAJE_DEBUG("Semaforo %d creado exitosamente con clave %d", idSem, clave);
    }
    sem.Inicializar(valorInicial);
}

void InicializarSemaforosRobots( const Configuracion& config ){
    string dirFtok = config.ObtenerDirFtok();
    for( int i=0; i<config.ObtenerCantidadRobots(); i++ ){
        InicializarSemaforo( config.ObtenerIdSemaforoRobotArmado( i ), dirFtok, 1 );
        InicializarSemaforo( config.ObtenerIdSemaforoRobotDespacho( i ), dirFtok, 1 );
    }
}

void InicializarSharedMemory( const string& dirFtok, const Configuracion& config ){
      int idShmPlataforma = config.ObtenerIdShmPlataforma();
      key_t clave = ftok( dirFtok.c_str(), idShmPlataforma );
      Robots2::ShmPlataforma* pShmPlataforma = 0;
      int capacidad = config.ObtenerCapacidadPlataforma();
      if( capacidad > MAX_CAPACIDAD_PLATAFORMA ){
            MENSAJE_ERROR( "Capacidad de plataforma, %d, excede el máximo (%d)",
                           capacidad, MAX_CAPACIDAD_PLATAFORMA );
            exit(1);
      }
      int handleShmPlataforma = shmget( clave, sizeof( pShmPlataforma ), IPC_CREAT | IPC_EXCL | 0660 );
      if( handleShmPlataforma == -1 ){
            MENSAJE_ERROR("Error al crear la Shared Memory con id %d", idShmPlataforma);
            exit(1);
      }else{
           MENSAJE_DEBUG( "Shared Memory creada exitosamente con id %d", idShmPlataforma );
      }
      pShmPlataforma = static_cast<Robots2::ShmPlataforma*>( shmat( handleShmPlataforma, 0, 0 ) );
      if( pShmPlataforma == reinterpret_cast<Robots2::ShmPlataforma*>(-1) ){
            MENSAJE_ERROR( "Error en attach de Shared Memory" );
            exit(1);
      }else{
            MENSAJE_DEBUG( "Shared Memory %d attacheada exitosamente", idShmPlataforma );
      }
      pShmPlataforma->Capacidad = capacidad;
      pShmPlataforma->EspaciosOcupados = 0;
      for( int i=0; i<capacidad; i++ ){
            pShmPlataforma->EstadoDePosiciones[i] = Robots2::EPP_LIBRE;
            pShmPlataforma->Dispositivos[i] = -1;
            pShmPlataforma->TipoDispositivo[i] = -1;
      }
      pShmPlataforma->EstadoDePosiciones[capacidad] = Robots2::EPP_FIN_LISTA;
      pShmPlataforma->Dispositivos[capacidad] = 0;
}

void LanzarProcesosDeRobotsDeArmadoYDespacho( int nRobots ){
    for( int i=1; i<=nRobots; i++ ){
        char sNumRobot[8];
        sprintf( sNumRobot, "%d", i );
        int pid;
        //Robot i-control de armado
        if( (pid = fork()) <0 ){
              MENSAJE_ERROR("Error al hacer fork de %s %d", Robots2::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO.c_str(), i);
              exit(-1);
        }else if( pid == 0 ){//Hijo
              execlp( string("./" + Robots2::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO).c_str(),
                    Robots2::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO.c_str(), sNumRobot, reinterpret_cast<char*>(0) );
              MENSAJE_ERROR("Proceso robot armado %d fallo", i);
              exit(-1);
        }
        //Robot i-control de despacho
        if( (pid = fork()) <0 ){
              MENSAJE_ERROR("Error al hacer fork de %s %d", Robots2::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO.c_str(), i);
              exit(-1);
        }else if( pid == 0 ){//Hijo
              execlp( string("./" + Robots2::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO).c_str(),
                    Robots2::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO.c_str(), sNumRobot, reinterpret_cast<char*>(0) );
              MENSAJE_ERROR("Proceso robot despacho %d fallo", i+1);
              exit(-1);
        }
    }
}

void LanzarProcesosDeRobotsEmpacadores( int nRobots ){
    for( int i=1; i<=nRobots; i++ ){
        char sNumRobot[8];
        sprintf( sNumRobot, "%d", i );
        int pid;
        //Robot i-control de armado
        if( (pid = fork()) <0 ){
              MENSAJE_ERROR("Error al hacer fork de %s %d", Robots2::Constantes::NOMBRE_PROCESO_ROBOT_EMPAQUE.c_str(), i);
              exit(-1);
        }else if( pid == 0 ){//Hijo
              execlp( string("./" + Robots2::Constantes::NOMBRE_PROCESO_ROBOT_EMPAQUE).c_str(),
                    Robots2::Constantes::NOMBRE_PROCESO_ROBOT_EMPAQUE.c_str(), sNumRobot, reinterpret_cast<char*>(0) );
              MENSAJE_ERROR("Proceso robot empaque %d fallo", i);
              exit(-1);
        }
    }
}

void LanzarProcesoCintaEntrada(){
    int pid;
    if( (pid = fork()) <0 ){
        MENSAJE_ERROR("Error al hacer fork de proceso %s", Robots2::Constantes::NOMBRE_PROCESO_CINTA_ENTRADA.c_str() );
        exit(-1);
    }else if( pid == 0 ){//Hijo
        execlp( string("./" + Robots2::Constantes::NOMBRE_PROCESO_CINTA_ENTRADA).c_str(),
                Robots2::Constantes::NOMBRE_PROCESO_CINTA_ENTRADA.c_str(), reinterpret_cast<char*>(0) );
        MENSAJE_ERROR("Proceso %s fallo", Robots2::Constantes::NOMBRE_PROCESO_CINTA_ENTRADA.c_str());
        exit(-1);
    }
}

void InicializarCola( const std::string& dirFtok, int idCola ){
      key_t clave = ftok( dirFtok.c_str(), idCola );
      int handleCola = msgget( clave, IPC_CREAT | IPC_EXCL | 0660 );
      if( handleCola == -1 ){
            MENSAJE_ERROR("Lanzador: Error al crear Cola %d", idCola);
            exit(1);
      }else{
            MENSAJE_DEBUG( "Cola %d creada exitosamente", handleCola );
      }
}

void InicializarColasDispositivos( const std::string& dirFtok, int idBase, int nMaxDisp ){
    for( int i=0; i<nMaxDisp; i++ )
      InicializarCola( dirFtok, idBase + i );
}

int main( int argc, char** argv ){
    MENSAJE_DEBUG("PROCESO INICIADO");
    Configuracion config;
    if( !config.LeerDeArchivo() ){
        MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
        MENSAJE_DEBUG("PROCESO FINALIZADO");
        exit( -1 );
    }
    string dirFtok = config.ObtenerDirFtok();
    InicializarSharedMemory( dirFtok, config );
    InicializarSemaforo( config.ObtenerIdSemaforoCinta(), dirFtok, 0 );
    InicializarSemaforo( config.ObtenerIdMutexPlataforma(), dirFtok, 1 );
    InicializarSemaforosRobots( config );
    InicializarCola( dirFtok, config.ObtenerIdColaPlataforma() );
    int cantRobots = config.ObtenerCantidadRobots();
    LanzarProcesosDePlataforma( cantRobots );
    LanzarProcesosDeRobotsDeArmadoYDespacho( cantRobots );
    LanzarProcesosDeRobotsEmpacadores( config.ObtenerCantidadTiposDeDispositivo() );
    InicializarColasDispositivos( dirFtok, config.ObtenerIdBaseColasDispositivos(),
                                  config.ObtenerCantMaxDispositivos() );
    LanzarProcesoCintaEntrada();
    MENSAJE_DEBUG("PROCESO FINALIZADO");
    return 0;
}
