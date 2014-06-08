#include "dispositivo.h"
#include "core.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include "random.h"
#include <sys/msg.h>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( m_NombreProceso, m_ColorMensajes, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( m_NombreProceso, fmt, ##__VA_ARGS__ )

Dispositivo::Dispositivo( int id, const Configuracion& config, const string& nombreProceso ):
        m_Id( id ),
        m_NombreProceso( nombreProceso ){
    int idCola = config.ObtenerIdColaDispositivo( m_Id );
    m_DemoraActivacion = config.ObtenerDemoraActivacion();
    m_Cola = msgget( ftok( config.ObtenerDirFtok().c_str(), idCola ), 0660 );
    if( m_Cola == -1 ){
        MENSAJE_ERROR( "Error al conectarse a la cola %d", idCola );
        exit(1);
    }
}

Dispositivo::~Dispositivo(){}

int Dispositivo::Empacar(){
    //TODO <NIM>
    return -1;
}

int Dispositivo::EsperarInicioArmado(){
    //Esperar mensaje de inicio de armado
    Robots2::MensajeColaDispositivo msj;
    int codError = msgrcv( m_Cola, &msj, sizeof(msj)-sizeof(long),
                       Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al recibir mensaje de Inicio de Armado" );
        exit( 5 );
    }
    if( msj.Msg != Robots2::MensajesDispositivo::INICIAR_ARMADO ){
        MENSAJE_ERROR( "Se esperaba un mensaje de Inicio de armado pero llego otro" );
        exit( 5 );
    }
    return msj.DatosMsg;
}

void Dispositivo::IniciarArmado( int idRobot ){
    Robots2::MensajeColaDispositivo msj = {
        Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), //Tipo
        Robots2::MensajesDispositivo::INICIAR_ARMADO,          //Msg
        idRobot                                                //DatosMsg
    };
    int codError = msgsnd( m_Cola, &msj, sizeof(msj)-sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar INICIAR_ARMADO " );
        exit( 5 );
    }
}

void Dispositivo::EsperarFinArmado(){
    //Esperar mensaje de fin de armado
    Robots2::MensajeColaDispositivo msj;
    int codError = msgrcv( m_Cola, &msj, sizeof(msj)-sizeof(long),
                           Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al recibir mensaje de Inicio de Armado" );
        exit( 5 );
    }
    if( msj.Msg != Robots2::MensajesDispositivo::FINALIZAR_ARMADO ){
        MENSAJE_ERROR( "Se esperaba un mensaje de Fin de armado pero llego otro" );
        exit( 5 );
    }
}

void Dispositivo::FinalizarArmado(){
    Robots2::MensajeColaDispositivo msj = {
        Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), //Tipo
        Robots2::MensajesDispositivo::FINALIZAR_ARMADO,        //Msg
        -1                                                     //DatosMsg
    };
    int codError = msgsnd( m_Cola, &msj, sizeof(msj)-sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar FINALIZAR_ARMADO " );
        exit( 5 );
    }
}

void Dispositivo::EsperarFinActivacion(){
    //Esperar mensaje de fin de activacion
    Robots2::MensajeColaDispositivo msj;
    int codError = msgrcv( m_Cola, &msj, sizeof(msj)-sizeof(long),
                           Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al recibir mensaje de fin de Activacion" );
        exit( 5 );
    }
    if( msj.Msg != Robots2::MensajesDispositivo::ACTIVACION_FINALIZADA ){
        MENSAJE_ERROR( "Se esperaba un mensaje de Fin de activacion pero llego otro" );
        exit( 5 );
    }
}

void Dispositivo::Activar(){
    Random::DemoraAleatoriaEnMilis( m_DemoraActivacion );
    Robots2::MensajeColaDispositivo msj = {
        Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), //Tipo
        Robots2::MensajesDispositivo::ACTIVACION_FINALIZADA,   //Msg
        -1                                                     //DatosMsg
    };
    int codError = msgsnd( m_Cola, &msj, sizeof(msj)-sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar ACTIVACION_FINALIZADA " );
        exit( 5 );
    }
}

int Dispositivo::EsperarDespacho(){
    //Esperar mensaje de fin de despacho
    Robots2::MensajeColaDispositivo msj;
    int codError = msgrcv( m_Cola, &msj, sizeof(msj)-sizeof(long),
                       Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al recibir mensaje de Fin de Despacho" );
        exit( 5 );
    }
    if( msj.Msg != Robots2::MensajesDispositivo::DESPACHO_FINALIZADO ){
        MENSAJE_ERROR( "Se esperaba un mensaje de Despacho finalizado pero llego otro" );
        exit( 5 );
    }
    return msj.DatosMsg;
}

void Dispositivo::Despachar( int idRobot ){
    Robots2::MensajeColaDispositivo msj = {
        Robots2::TipoMensajes::MensajeColaDispositivo( m_Id ), //Tipo
        Robots2::MensajesDispositivo::DESPACHO_FINALIZADO,     //Msg
        idRobot                                                //DatosMsg
    };
    int codError = msgsnd( m_Cola, &msj, sizeof(msj)-sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar DESPACHO_FINALIZADO " );
        exit( 5 );
    }
}
