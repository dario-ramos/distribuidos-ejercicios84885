#include "core.h"
#include "cinta_entrada.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include <functional>
#include <memory>

using namespace Utils;
using std::string;

const std::string NOMBRE_PROCESO = "cinta_entrada";

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( NOMBRE_PROCESO, Utils::BLANCO, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( NOMBRE_PROCESO, fmt, ##__VA_ARGS__ )

bool HayMasDispositivos_Siempre( int, int ){
    return true;
}

bool HayMasDispositivos_N( int i, int N ){
    return i <= N;
}

int main( int argc, char** argv ){
    MENSAJE_DEBUG("PROCESO INICIADO");
    Configuracion config;
    if( !config.LeerDeArchivo() ){
        MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
        MENSAJE_DEBUG("PROCESO FINALIZADO");
        exit( -1 );
    }
    //Si cantDisp >=1, se generan N dispositivos y se termina. Si no, se generan
    //dispositivos en un ciclo infinito
    int cantDisp = config.ObtenerCantidadDispositivosAGenerar();
    std::function<bool(int,int)> condicionCorte = cantDisp < 1?
                                                  HayMasDispositivos_Siempre :
                                                  HayMasDispositivos_N;
    std::unique_ptr<ICintaEntrada> pCinta( new CintaEntrada( config, NOMBRE_PROCESO ) ); 
    for( int iDisp = 1; condicionCorte( iDisp, cantDisp ); iDisp++ ){
        pCinta->GenerarDispositivo( iDisp );
    }
    MENSAJE_DEBUG("PROCESO FINALIZADO");
    return 0;
}
