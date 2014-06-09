#include "robot_empaque.h"
#include "dispositivo.h"
#include "core.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include "random.h"
#include <sys/msg.h>
#include <memory>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( m_NombreProceso, Utils::VERDE, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( m_NombreProceso, fmt, ##__VA_ARGS__ )

RobotEmpaque::RobotEmpaque( int idRobot, const std::string& nombreProceso, const Configuracion& config ):
        m_Config( config ),
        m_Id( idRobot ),
        m_NombreProceso( nombreProceso ){
    int idCola = m_Config.ObtenerIdColaRobotEmpaque( m_Id );
    m_DemoraEmpaque = m_Config.ObtenerDemoraEmpaque();
    m_Cola = msgget( ftok( m_Config.ObtenerDirFtok().c_str(), idCola ), 0660 );
    if( m_Cola == -1 ){
        MENSAJE_ERROR( "Error al conectarse a la cola %d", idCola );
        exit(1);
    }
}

RobotEmpaque::~RobotEmpaque(){}

void RobotEmpaque::EmpacarDispositivo(){
    MENSAJE_DEBUG( "Esperando que llegue dispositivo para empacar..." );
    //Esperar mensaje de inicio de empaque
    Robots2::MensajeColaRobotEmpaque msj;
    int codError = msgrcv( m_Cola, &msj, sizeof(msj)-sizeof(long),
                           Robots2::TipoMensajes::MensajeColaRobotEmpaque( m_Id ), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al recibir mensaje de Fin de Iniciar Empaque" );
        exit( 5 );
    }
    if( msj.Msg != Robots2::MensajesRobotEmpaque::INICIAR_EMPAQUE ){
        MENSAJE_ERROR( "Se esperaba un mensaje de Iniciar Empaque pero llego otro" );
        exit( 5 );
    }
    //Empacar
    int idDisp = msj.DatosMsg;
    MENSAJE_DEBUG( "¡Dispositivo %d recibido! Empacando...", idDisp );
    Random::DemoraAleatoriaEnMilis( m_DemoraEmpaque );
    MENSAJE_DEBUG( "¡Empaque de dispositivo %d finalizando!", idDisp );
    //Notificar dispositivo
    std::unique_ptr<IDispositivo> pDisp( new Dispositivo( idDisp, m_Config, m_NombreProceso ) );
    pDisp->Empacar( m_Id );
}

void RobotEmpaque::IniciarEmpaqueDeDispositivo( int idDisp ){
    Robots2::MensajeColaRobotEmpaque msj = {
        Robots2::TipoMensajes::MensajeColaRobotEmpaque( m_Id ), //Tipo
        Robots2::MensajesRobotEmpaque::INICIAR_EMPAQUE,         //Msg
        idDisp                                                  //DatosMsg
    };
    int codError = msgsnd( m_Cola, &msj, sizeof(msj)-sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar INICIAR_EMPAQUE " );
        exit( 5 );
    }
}
