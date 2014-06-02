#include "dispositivo.h"
#include "core.h"
#include "configuracion.h"
#include "mensaje_debug.h"
/*#include "random.h"
#include "t_sem.h"
#include <sys/shm.h>
#include <sys/msg.h>*/

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( nombreProceso, Utils::VERDE, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( nombreProceso, fmt, ##__VA_ARGS__ )*/

void ValidarArgumentos( int argc, char** argv ){
      if( argc < 2 ){
            MensajeError( Robots2::Constantes::NOMBRE_PROCESO_DISPOSITIVO, "Faltan parametros. Uso: %s <numDisp>", argv[0] );
            exit(-1);
      }
}

int main( int argc, char** argv ){
      //Obtener id de dispositivo
      ValidarArgumentos( argc, argv );
      int numDisp = std::stoi( argv[1] );
      string nombreProceso = Robots2::Constantes::NOMBRE_PROCESO_DISPOSITIVO + std::to_string( numDisp );
      MENSAJE_DEBUG("PROCESO INICIADO");
      Configuracion config;
      if( !config.LeerDeArchivo() ){
          MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
          MENSAJE_DEBUG("PROCESO FINALIZADO");
          exit( -1 );
      }
      std::unique_ptr<IDispositivo> pDisp( new Dispositivo( config ) );
      MENSAJE_DEBUG("En plataforma, esperando que robot arme... ");
      int idRobotArmado = pDisp->IniciarArmado();
      MENSAJE_DEBUG( "Siendo armado por robot %d...", idRobotArmado );
      pDisp->FinalizarArmado();
      MENSAJE_DEBUG( "Armado finalizando. Activando..." );
      pDisp->Activar();
      MENSAJE_DEBUG( "Activacion completa. Esperando espacho..." );
      int idRobotDespacho = pDisp->Despachar();
      MENSAJE_DEBUG( "Despachado por robot %d. Esperando empaque...", idRobotDespacho );
      int idRobotEmpaque = pDisp->Empacar();
      MENSAJE_DEBUG( "¡Empaque completado por robot %d!", idRobotEmpaque );
      MENSAJE_DEBUG("PROCESO FINALIZADO");
      return 0;
}
