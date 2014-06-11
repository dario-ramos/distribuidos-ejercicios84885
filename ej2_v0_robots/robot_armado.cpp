#include "core.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include "plataforma.h"
#include "dispositivo.h"
#include "random.h"
#include <memory>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( nombreProceso, Utils::MAGENTA, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( nombreProceso, fmt, ##__VA_ARGS__ )

void ValidarArgumentos( int argc, char** argv ){
    if( argc < 2 ){
        MensajeError( Robots2::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO, "Faltan parametros. Uso: %s <numRobot>", argv[0] );
        exit(-1);
    }
}

void ArmarDispositivo( IPlataformaArmado* pPlataforma,
                       int* pCantPilas,
                       int numRobot,
                       std::pair<int,int> demora,
                       const Configuracion& config,
                       const string& nombreProceso ){
    MENSAJE_DEBUG( "Lugar disponible, esperando que robot no este despachando..." );
    int idDisp = pPlataforma->IniciarArmado( numRobot );
    std::unique_ptr<IDispositivo> pDispositivo( new Dispositivo( idDisp, config, nombreProceso ) );
    pDispositivo->IniciarArmado( numRobot );    
    MENSAJE_DEBUG( "Armando..." );
    Random::DemoraAleatoriaEnMilis( demora );
    (*pCantPilas) -= 2;
    MENSAJE_DEBUG( "Dispositivo listo, colocando en plataforma" );
    pPlataforma->FinalizarArmado( numRobot );
    pDispositivo->FinalizarArmado();
}

int main( int argc, char** argv ){
    const int DEMORA_NO_HAY_DISPOSITIVOS_PARA_ARMAR = 5;
    ValidarArgumentos( argc, argv );
    int numRobot = atoi( argv[1] );
    string nombreProceso = Robots2::Constantes::NOMBRE_PROCESO_ROBOT_ARMADO + string( argv[1] );
    MENSAJE_DEBUG("PROCESO INICIADO");
    Configuracion config;
    if( !config.LeerDeArchivo() ){
        MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
        MENSAJE_DEBUG("PROCESO FINALIZADO");
        exit( -1 );
    }
    std::unique_ptr<IPlataformaArmado> pPlataforma( new Plataforma( config, nombreProceso, Utils::MAGENTA ) );
    int nPilas = config.ObtenerCantidadInicialPilas();
    while( nPilas > 1 ){
        MENSAJE_DEBUG( "Preguntando si hay dispositivos para armar" );
        if( !pPlataforma->HayDispositivosParaArmar( numRobot ) ){
            sleep( DEMORA_NO_HAY_DISPOSITIVOS_PARA_ARMAR );
            continue;
        }
        ArmarDispositivo( pPlataforma.get(), &nPilas,
                          numRobot, config.ObtenerDemoraArmado(), config, nombreProceso );
    }
    MENSAJE_DEBUG("PROCESO FINALIZADO");
    return 0;
}
