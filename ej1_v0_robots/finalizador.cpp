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
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( "FINALIZADOR", Utils::BLANCO, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( "FINALIZADOR", fmt, ##__VA_ARGS__ )

void MatarProcesoPorNombre( const std::string& nombre ){
      int codError = system( string("pkill -9 " + nombre).c_str() );
      if( codError == -1 ){
            MENSAJE_ERROR( "Error al matar proceso %s", nombre.c_str() );
            exit(-1);
      }else{
            MENSAJE_DEBUG( "Proceso %s matado exitosamente", nombre.c_str() );
      }
}

void MatarProcesosRobots( int nRobots ){
      for( int iRobot=0; iRobot<nRobots; iRobot++ ){
            MatarProcesoPorNombre( Robots::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO );
            MatarProcesoPorNombre( Robots::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO );
      }
}

void LiberarColaDePlataforma( const std::string& dirFtok, int idCola ){
      key_t clave = ftok( dirFtok.c_str(), idCola );
      int cola = msgget( clave, IPC_CREAT | 0660 );
      if( cola == -1 ){
            MENSAJE_ERROR("Error al acceder a Cola %d para liberarla", idCola);
            exit(8);
      }
      int codError = msgctl( cola, IPC_RMID, 0 );
      if( codError == -1 ){
            MENSAJE_ERROR("Error al liberar Cola %d", idCola);
            exit(9);
      }else{
            MENSAJE_DEBUG("Cola %d liberada exitosamente", idCola);
      }
}

void LiberarMutex( const string& dirFtok, const int idMutex ){
    t_sem mutex( ftok(dirFtok.c_str(), idMutex) );
    if( !mutex.Existe() ){
        MENSAJE_ERROR("Error al acceder al mutex %d para liberarlo", idMutex);
        exit(6);
    }
    int codError = mutex.Destruir();
    if( codError == -1 ){
        MENSAJE_ERROR("Error al liberar Mutex %d", idMutex);
        exit(7);
    }else
        MENSAJE_DEBUG("Mutex %d liberado exitosamente", idMutex );
}

void LiberarSemaforosRobots( const Configuracion& config ){
    string dirFtok = config.ObtenerDirFtok();
    for( int i=0; i<config.ObtenerCantidadRobots(); i++ ){
        LiberarMutex( dirFtok, config.ObtenerIdSemaforoRobotArmado(i) );
        LiberarMutex( dirFtok, config.ObtenerIdSemaforoRobotDespacho(i) );
    }
}

void LiberarSharedMemoryPlataforma( const string& dirFtok, const Configuracion& config ){
    int idShmPlataforma = config.ObtenerIdShmPlataforma();
    Robots::ShmPlataforma* pPlataforma;
    int handleShmPlataforma = shmget( ftok(dirFtok.c_str(),idShmPlataforma),
                                      sizeof( pPlataforma ),
                                      IPC_CREAT | 0666 );
    if( handleShmPlataforma == -1 ){
        MENSAJE_ERROR( "Error al acceder a la Shared Memory %d para liberarla", idShmPlataforma ); 
        exit(2);
    }
    int codError = shmctl( handleShmPlataforma, IPC_RMID, 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al liberar la Shared Memory %d", idShmPlataforma );
        exit(3);
    }else{
        MENSAJE_DEBUG( "Shared Memory %d liberada exitosamente", idShmPlataforma );
    }
}

int main( int argc, char** argv ){
      MENSAJE_DEBUG("PROCESO INICIADO");
      Configuracion config;
      if( !config.LeerDeArchivo() ){
            MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
            MENSAJE_DEBUG("PROCESO FINALIZADO");
            exit( 1 );
      }
      int cantRobots = config.ObtenerCantidadRobots();
      MatarProcesosRobots( cantRobots );
      for( int i=1; i<=cantRobots; i++ ){
        MatarProcesoPorNombre( Robots::Constantes::NOMBRE_PROCESO_PLATAFORMA +
                               std::to_string( i ) );
      }
      string dirFtok = config.ObtenerDirFtok();
      LiberarColaDePlataforma( dirFtok, config.ObtenerIdColaPlataforma() );
      LiberarSemaforosRobots( config );
      LiberarMutex( dirFtok, config.ObtenerIdMutexPlataforma() );
      LiberarSharedMemoryPlataforma( dirFtok, config );
      MENSAJE_DEBUG("PROCESO FINALIZADO");
      return 0;
}
