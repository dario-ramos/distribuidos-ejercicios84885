#include "core.h"
#include "configuracion.h"
#include "mensaje_debug.h"
#include "plataforma.h"
#include <sys/msg.h>

using namespace Utils;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( m_NombreProceso, m_ColorMensajes, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( m_NombreProceso, fmt, ##__VA_ARGS__ )

Plataforma::Plataforma( const Configuracion& config, const std::string& nombreProceso, int colorMensajes ) :
        m_ColorMensajes( colorMensajes ),
        m_ColaPlataforma( -1 ),
        m_NombreProceso( nombreProceso ) {
    int idCola = config.ObtenerIdColaPlataforma();
    m_ColaPlataforma = msgget( ftok( config.ObtenerDirFtok().c_str(), idCola ), 0660 );
    if( m_ColaPlataforma == -1 ){
        MENSAJE_ERROR( "Error al conectarse a la cola %d", idCola );
        exit(1);
    }
}

Plataforma::~Plataforma(){}

bool Plataforma::HayDispositivosParaArmar( int numeroRobot ) const{
      //Preguntar
      Robots2::MensajeColaPlataforma pedido = {
                    Robots2::TipoMensajes::PEDIDO_A_PLATAFORMA, numeroRobot,
                    Robots2::MensajesPlataforma::PREGUNTA_HAY_DISPOSITIVOS_PARA_ARMAR };
      int codError = msgsnd( m_ColaPlataforma, &pedido, sizeof(pedido)-sizeof(long), 0 );
      if( codError == -1 ){
            MENSAJE_ERROR( "Error al enviar PEDIDO_A_PLATAFORMA - PREGUNTA_HAY_DISPOSITIVOS_PARA_ARMAR" );
            exit( 5 );
      }
      //Esperar respuesta
      Robots2::MensajeColaPlataforma respuesta;
      //MENSAJE_DEBUG( "Esperando respuesta a PEDIDO_A_PLATAFORMA - PREGUNTA_HAY_DISPOSITIVOS_PARA_ARMAR..." );
      codError = msgrcv( m_ColaPlataforma, &respuesta, sizeof(respuesta)-sizeof(long),
                         Robots2::TipoMensajes::RespuestaARobotArmado( numeroRobot ), 0 );
      if( codError == -1 ){
            MENSAJE_ERROR( "Error al recibir respuesta " );
            exit( 5 );
      }
      return (respuesta.Msg == Robots2::MensajesPlataforma::RESPUESTA_HAY_DISPOSITIVOS_PARA_ARMAR_SI);
}

int Plataforma::IniciarArmado( int numeroRobot ){
    //Preguntar
    Robots2::MensajeColaPlataforma pedido = {
        Robots2::TipoMensajes::PEDIDO_A_PLATAFORMA, numeroRobot,
        Robots2::MensajesPlataforma::PEDIDO_INICIAR_ARMADO };
    int codError = msgsnd( m_ColaPlataforma, &pedido, sizeof(pedido)-sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar PEDIDO_A_PLATAFORMA - INICIAR_ARMADO " );
        exit( 5 );
    }
    //Esperar respuesta
    Robots2::MensajeColaPlataforma respuesta;
    MENSAJE_DEBUG( "Esperando respuesta a PEDIDO_A_PLATAFORMA - INICIAR_ARMADO..." );
    codError = msgrcv( m_ColaPlataforma, &respuesta, sizeof(respuesta)-sizeof(long),
                       Robots2::TipoMensajes::RespuestaARobotArmado( numeroRobot ), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al recibir respuesta " );
        exit( 5 );
    }
    return respuesta.DatosMsg;
}

void Plataforma::FinalizarArmado( int numeroRobot ){
      Robots2::MensajeColaPlataforma pedido = {
                    Robots2::TipoMensajes::PEDIDO_A_PLATAFORMA, numeroRobot,
                    Robots2::MensajesPlataforma::PEDIDO_FINALIZAR_ARMADO };
      int codError = msgsnd( m_ColaPlataforma, &pedido, sizeof(pedido) - sizeof(long), 0 );
      if( codError == -1 ){
            MENSAJE_ERROR( "Error al enviar PEDIDO_A_PLATAFORMA - FINALIZAR_ARMADO " );
            exit( 5 );
      }
}

int Plataforma::DetectarFrecuenciaActivacion( int numeroRobot ){
      //Preguntar
      Robots2::MensajeColaPlataforma pedido = {
                    Robots2::TipoMensajes::PEDIDO_A_PLATAFORMA, numeroRobot,
                    Robots2::MensajesPlataforma::PEDIDO_DETECTAR_FRECUENCIA };
      int codError = msgsnd( m_ColaPlataforma, &pedido, sizeof(pedido), 0 );
      if( codError == -1 ){
            MENSAJE_ERROR( "Error al enviar PEDIDO_A_PLATAFORMA - PEDIDO_DETECTAR_FRECUENCIA " );
            exit( 5 );
      }
      //Esperar respuesta
      Robots2::MensajeColaPlataforma respuesta;
      MENSAJE_DEBUG( "Esperando respuesta a PEDIDO_A_PLATAFORMA - PEDIDO_DETECTAR_FRECUENCIA..." );
      codError = msgrcv( m_ColaPlataforma, &respuesta, sizeof(respuesta),
                         Robots2::TipoMensajes::RespuestaARobotDespacho( numeroRobot ), 0 );
      if( codError == -1 ){
            MENSAJE_ERROR( "Error al recibir respuesta " );
            exit( 5 );
      }
      return respuesta.DatosMsg; // respuesta.Msg;
}

void Plataforma::DespacharDispositivo( int numeroRobot, int posDisp ){
    //Preguntar
    Robots2::MensajeColaPlataforma pedido = {
        Robots2::TipoMensajes::PEDIDO_A_PLATAFORMA, numeroRobot,
        Robots2::MensajesPlataforma::PEDIDO_DESPACHAR_DISPOSITIVO,
        posDisp
    };
    int codError = msgsnd( m_ColaPlataforma, &pedido, sizeof(pedido), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar PEDIDO_A_PLATAFORMA - PEDIDO_DESPACHAR_DISPOSITIVO " );
        exit( 5 );
    }
    //Esperar respuesta
    Robots2::MensajeColaPlataforma respuesta;
    MENSAJE_DEBUG( "Esperando respuesta a PEDIDO_A_PLATAFORMA - PEDIDO_DESPACHAR_DISPOSITIVO..." );
    codError = msgrcv( m_ColaPlataforma, &respuesta, sizeof(respuesta),
                       Robots2::TipoMensajes::RespuestaARobotDespacho( numeroRobot ), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al recibir respuesta " );
        exit( 5 );
    }
}
