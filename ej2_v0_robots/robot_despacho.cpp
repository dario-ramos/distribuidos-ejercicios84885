#include "core.h"
#include "plataforma.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include "random.h"
#include <memory>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( nombreProceso, Utils::CIAN, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( nombreProceso, fmt, ##__VA_ARGS__ )

void ValidarArgumentos( int argc, char** argv ){
      if( argc < 2 ){
            MensajeError( Robots2::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO, "Faltan parametros. Uso: %s <numRobot>", argv[0] );
            exit(-1);
      }
}

int main( int argc, char** argv ){
      ValidarArgumentos( argc, argv );
      int numRobot = atoi( argv[1] );
      string nombreProceso = Robots2::Constantes::NOMBRE_PROCESO_ROBOT_DESPACHO + string( argv[1] );
      MENSAJE_DEBUG("PROCESO INICIADO");
      Configuracion config;
      if( !config.LeerDeArchivo() ){
            MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
            MENSAJE_DEBUG("PROCESO FINALIZADO");
            exit( -1 );
      }
      std::unique_ptr<IPlataformaDespacho> pPlataforma( new Plataforma( config, nombreProceso, Utils::CIAN ) );
      while( true ){
          MENSAJE_DEBUG("Esperando frecuencia de activacion...");
          int posDisp = pPlataforma->DetectarFrecuenciaActivacion( numRobot );
          MENSAJE_DEBUG("Frecuencia de activacion detectada, despachando...");
          Random::DemoraAleatoriaEnMilis( config.ObtenerDemoraDespacho() );
          pPlataforma->DespacharDispositivo( numRobot, posDisp );
          MENSAJE_DEBUG("Dispositivo despachado");
      }
      MENSAJE_DEBUG("PROCESO FINALIZADO");
      return 0;
}
