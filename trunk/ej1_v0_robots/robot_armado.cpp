#include "core.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include "plataforma.h"
#include "random.h"
#include <memory>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( nombreProceso, Utils::MAGENTA, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( nombreProceso, fmt, ##__VA_ARGS__ )

void ValidarArgumentos( int argc, char** argv ){
      if( argc < 2 ){
            MensajeError( Robots::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO, "Faltan parametros. Uso: %s <numRobot>", argv[0] );
            exit(-1);
      }
}

void ArmarDispositivo( IPlataformaArmado* pPlataforma,
                       int* pCantDispSinPilas, int* pCantPilas,
                       int numRobot,
                       std::pair<int,int> demora, const string& nombreProceso ){
      MENSAJE_DEBUG( "Lugar disponible, esperando que robot no este despachando..." );
      pPlataforma->IniciarArmado( numRobot );
      MENSAJE_DEBUG( "Armando..." );
      Random::DemoraAleatoriaEnMilis( demora );
      (*pCantDispSinPilas)--;
      (*pCantPilas) -= 2;
      MENSAJE_DEBUG( "Dispositivo listo, colocando en plataforma" );
      pPlataforma->FinalizarArmado( numRobot );
}

int main( int argc, char** argv ){
      ValidarArgumentos( argc, argv );
      int numRobot = atoi( argv[1] );
      string nombreProceso = Robots::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO + string( argv[1] );
      MENSAJE_DEBUG("PROCESO INICIADO");
      Configuracion config;
      if( !config.LeerDeArchivo() ){
            MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
            MENSAJE_DEBUG("PROCESO FINALIZADO");
            exit( -1 );
      }
      std::unique_ptr<IPlataformaArmado> pPlataforma( new Plataforma( config, nombreProceso, Utils::MAGENTA ) );
      int nDispSinPilas = config.ObtenerCantidadInicialDispositivosSinPila();
      int nPilas = config.ObtenerCantidadInicialPilas();
      while( nDispSinPilas > 0 && nPilas > 1 ){
            MENSAJE_DEBUG( "Preguntando si plataforma llena" );
            if( pPlataforma->Llena( numRobot ) ){
                  sleep( 1 ); //TODO No magic numbers
                  continue;
            }
            ArmarDispositivo( pPlataforma.get(), &nDispSinPilas, &nPilas,
                              numRobot, config.ObtenerDemoraArmado(), nombreProceso );
      }
      MENSAJE_DEBUG("PROCESO FINALIZADO");
      return 0;
}
