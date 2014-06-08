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
                  execlp( string("./" + Robots::Constantes::NOMBRE_PROCESO_PLATAFORMA).c_str(),
                        Robots::Constantes::NOMBRE_PROCESO_PLATAFORMA.c_str(), sPlat, reinterpret_cast<char*>(0) );
                  MENSAJE_ERROR("Proceso plataforma fallo");
                  exit(1);
            }
      }
}

void LanzarProcesosDeRobots( int nRobots ){
      for( int i=0; i<nRobots; i++ ){
            char sNumRobot[8];
            sprintf( sNumRobot, "%d", i + 1 );
            int pid;
            //Robot i-control de armado
            if( (pid = fork()) <0 ){
                  MENSAJE_ERROR("Error al hacer fork de robot armado %d", i);
                  exit(-2);
            }else if( pid == 0 ){//Hijo
                  execlp( string("./" + Robots::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO).c_str(),
                        Robots::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO.c_str(), sNumRobot, reinterpret_cast<char*>(0) );
                  MENSAJE_ERROR("Proceso robot armado %d fallo", i+1);
                  exit(-3);
            }
            //Robot i-control de despacho
            if( (pid = fork()) <0 ){
                  MENSAJE_ERROR("Error al hacer fork de robot despacho %d", i);
                  exit(-4);
            }else if( pid == 0 ){//Hijo
                  execlp( string("./" + Robots::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO).c_str(),
                        Robots::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO.c_str(), sNumRobot, reinterpret_cast<char*>(0) );
                  MENSAJE_ERROR("Proceso robot despacho %d fallo", i+1);
                  exit(-5);
            }
      }
}

void InicializarColaPlataforma( const std::string& dirFtok, int idCola ){
      key_t clave = ftok( dirFtok.c_str(), idCola );
      int handleCola = msgget( clave, IPC_CREAT | IPC_EXCL | 0660 );
      if( handleCola == -1 ){
            MENSAJE_ERROR("Lanzador: Error al crear Cola %d", idCola);
            exit(1);
      }else{
            MENSAJE_DEBUG( "Cola %d creada exitosamente", handleCola );
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
      Robots::ShmPlataforma* pShmPlataforma = 0;
      int capacidad = config.ObtenerCapacidadPlataforma();
      if( capacidad > MAX_CAPACIDAD_PLATAFORMA ){
            MENSAJE_ERROR( "Capacidad de plataforma, %d, excede el máximo (%d)", capacidad, MAX_CAPACIDAD_PLATAFORMA );
            exit(1);
      }
      int handleShmPlataforma = shmget( clave, sizeof( pShmPlataforma ), IPC_CREAT | IPC_EXCL | 0660 );
      if( handleShmPlataforma == -1 ){
            MENSAJE_ERROR("Error al crear la Shared Memory con id %d", idShmPlataforma);
            exit(1);
      }else{
           MENSAJE_DEBUG( "Shared Memory creada exitosamente con id %d", idShmPlataforma );
      }
      pShmPlataforma = static_cast<Robots::ShmPlataforma*>( shmat( handleShmPlataforma, 0, 0 ) );
      if( pShmPlataforma == reinterpret_cast<Robots::ShmPlataforma*>(-1) ){
            MENSAJE_ERROR( "Error en attach de Shared Memory" );
            exit(1);
      }else{
            MENSAJE_DEBUG( "Shared Memory %d attacheada exitosamente", idShmPlataforma );
      }
      pShmPlataforma->Capacidad = capacidad;
      pShmPlataforma->EspaciosOcupados = 0;
      for( int i=0; i<capacidad; i++ ){
            pShmPlataforma->EstadoDePosiciones[i] = Robots::EPP_LIBRE;
      }
      pShmPlataforma->EstadoDePosiciones[capacidad] = Robots::EPP_FIN_LISTA;
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
      InicializarSemaforo( config.ObtenerIdMutexPlataforma(), dirFtok, 1 );
      int cantidadRobots = config.ObtenerCantidadRobots();
      InicializarSemaforosRobots( config );
      InicializarColaPlataforma( dirFtok, config.ObtenerIdColaPlataforma() );
      LanzarProcesosDePlataforma( cantidadRobots );
      LanzarProcesosDeRobots( cantidadRobots );
      MENSAJE_DEBUG("PROCESO FINALIZADO");
      return 0;
}
