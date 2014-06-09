#pragma once

#include <string>

namespace Robots2{

#define MAX_CAPACIDAD_PLATAFORMA 1024

namespace Constantes{
    const std::string NOMBRE_PROCESO_PLATAFORMA = "control_plataforma";
    const std::string NOMBRE_PROCESO_ROBOT_ARMADO = "robot_armado";
    const std::string NOMBRE_PROCESO_ROBOT_DESPACHO = "robot_despacho";
    const std::string NOMBRE_PROCESO_ROBOT_EMPAQUE = "robot_empaque";
    const std::string NOMBRE_PROCESO_CINTA_ENTRADA = "control_cinta_entrada";
    const std::string NOMBRE_PROCESO_DISPOSITIVO = "dispositivo";
}

enum EstadoPosicionPlataforma{
      EPP_FIN_LISTA = 0,
      EPP_LIBRE = 1,
      EPP_RESERVADA = 2,
      EPP_OCUPADA_INACTIVA = 3,
      EPP_OCUPADA_ACTIVA = 4,
      EPP_OCUPADA_SIN_ARMAR = 5
};

struct ShmPlataforma{
      int Capacidad;
      int EspaciosOcupados;
      EstadoPosicionPlataforma EstadoDePosiciones[MAX_CAPACIDAD_PLATAFORMA];
      int Dispositivos[MAX_CAPACIDAD_PLATAFORMA];
      int TipoDispositivo[MAX_CAPACIDAD_PLATAFORMA];
};

namespace MensajesDispositivo{
    const int INICIAR_ARMADO = 3001;
    const int FINALIZAR_ARMADO = 3002;
    const int ACTIVACION_FINALIZADA = 3003;
    const int DESPACHO_FINALIZADO =  3004;
    const int EMPAQUE_FINALIZADO = 3005;
}

typedef struct{
    long Tipo;
    int Msg;
    int DatosMsg;
} MensajeColaDispositivo;

typedef struct{
    long Tipo;
    int Msg;
    int DatosMsg;
} MensajeColaRobotEmpaque;

namespace MensajesPlataforma{
    const int PREGUNTA_PLATAFORMA_VACIA = 5001;
    const int RESPUESTA_PLATAFORMA_VACIA_NO = 5002;
    const int RESPUESTA_PLATAFORMA_VACIA_SI = 5003;
    const int PEDIDO_INICIAR_ARMADO = 5004;
    const int RESPUESTA_INICIAR_ARMADO = 5005;
    const int PEDIDO_FINALIZAR_ARMADO = 5006;
    const int PEDIDO_DETECTAR_FRECUENCIA = 5007;
    const int PEDIDO_DESPACHAR_DISPOSITIVO = 5008;
    const int PEDIDO_DISPOSITIVO_ACTIVADO = 5009;
}

namespace MensajesRobotEmpaque{
    const int INICIAR_EMPAQUE = 6001;
}

namespace TipoMensajes{
      const long PEDIDO_A_PLATAFORMA = 7001;

      inline int RespuestaARobotArmado( int numeroRobot ){ return 9000 + numeroRobot; }
      inline int RespuestaARobotDespacho( int numeroRobot ){ return 12000 + numeroRobot; }
      inline int MensajeColaDispositivo( int idDisp ){ return 13000 + idDisp; }
      inline int MensajeColaRobotEmpaque( int idRobot ){ return 14000 + idRobot; }
}

typedef struct{
    long Tipo;
    long NroRobot;
    int Msg;
    int DatosMsg;

    std::string ToString() const{
        std::string s;
        if( Tipo == TipoMensajes::PEDIDO_A_PLATAFORMA ){
            s = "PEDIDO_A_PLATAFORMA-";
            switch( Msg ){
                case MensajesPlataforma::PREGUNTA_PLATAFORMA_VACIA: return s + "PREGUNTA_PLATAFORMA_VACIA";
                case MensajesPlataforma::PEDIDO_INICIAR_ARMADO: return s + "PEDIDO_INICIAR_ARMADO";
                case MensajesPlataforma::PEDIDO_FINALIZAR_ARMADO: return s + "PEDIDO_FINALIZAR_ARMADO";
                case MensajesPlataforma::PEDIDO_DETECTAR_FRECUENCIA: return s + "PEDIDO_DETECTAR_FRECUENCIA";
                case MensajesPlataforma::PEDIDO_DESPACHAR_DISPOSITIVO: return s + "PEDIDO_DESPACHAR_DISPOSITIVO";
                case MensajesPlataforma::PEDIDO_DISPOSITIVO_ACTIVADO: return s + "PEDIDO_DISPOSITIVO_ACTIVADO";
                default: return s + "NO_TIPADO";
            }
        }else if( Tipo < 12000 ){
            s = "RESPUESTA_A_ROBOT_ARMADO-";
            switch( Msg ){
               case MensajesPlataforma::RESPUESTA_PLATAFORMA_VACIA_NO: return s + "RESPUESTA_PLATAFORMA_VACIA_NO";
               case MensajesPlataforma::RESPUESTA_PLATAFORMA_VACIA_SI: return s + "RESPUESTA_PLATAFORMA_VACIA_SI";
               case MensajesPlataforma::RESPUESTA_INICIAR_ARMADO: return s + "RESPUESTA_INICIAR_ARMADO";
               default: return s + "NO_TIPADA";
            }
        }else{
            return "RESPUESTA_A_ROBOT_DESPACHO-NO_TIPADA";
        }
    }
} MensajeColaPlataforma;

}

