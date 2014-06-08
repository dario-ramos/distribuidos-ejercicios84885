#pragma once

#include <string>

namespace Robots{

#define MAX_CAPACIDAD_PLATAFORMA 1024

namespace Constantes{
      const std::string NOMBRE_PROCESO_PLATAFORMA = "control_plataforma";
      const std::string NOMBRE_PROCESO_ROBOT_ARMADO = "robot_armado";
      const std::string NOMBRE_PROCESO_ROBOT_DESPACHO = "robot_despacho";
      const std::string NOMBRE_PROCESO_ACTIVADOR_DISPOSITIVO = "activador_dispositivo";
}

enum EstadoPosicionPlataforma{
      EPP_FIN_LISTA = 0,
      EPP_LIBRE = 1,
      EPP_RESERVADA = 2,
      EPP_OCUPADA_INACTIVA = 3,
      EPP_OCUPADA_ACTIVA = 4
};

typedef struct{
      int Capacidad;
      int EspaciosOcupados;
      EstadoPosicionPlataforma EstadoDePosiciones[MAX_CAPACIDAD_PLATAFORMA];

      /*static size_t ObtenerTamanio( int capacidad ){
            return sizeof(int) * 2 + sizeof(EstadoPosicionPlataforma) * MAX_CAPACIDAD_PLATAFORMA;
      }*/
}ShmPlataforma;

namespace Mensajes{
      const int PREGUNTA_PLATAFORMA_LLENA = 5001;
      const int RESPUESTA_PLATAFORMA_LLENA_NO = 5002;
      const int RESPUESTA_PLATAFORMA_LLENA_SI = 5003;
      const int PEDIDO_INICIAR_ARMADO = 5004;
      const int RESPUESTA_INICIAR_ARMADO = 5005;
      const int PEDIDO_FINALIZAR_ARMADO = 5006;
      const int PEDIDO_DETECTAR_FRECUENCIA = 5007;
      const int PEDIDO_DESPACHAR_DISPOSITIVO = 5008;
}

namespace TipoMensajes{
      const long PEDIDO_A_PLATAFORMA = 7001;

      inline int RespuestaARobotArmado( int numeroRobot ){ return 9000 + numeroRobot; }
      inline int RespuestaARobotDespacho( int numeroRobot ){ return 12000 + numeroRobot; }
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
                case Mensajes::PREGUNTA_PLATAFORMA_LLENA: return s + "PREGUNTA_PLATAFORMA_LLENA";
                case Mensajes::PEDIDO_INICIAR_ARMADO: return s + "PEDIDO_INICIAR_ARMADO";
                case Mensajes::PEDIDO_FINALIZAR_ARMADO: return s + "PEDIDO_FINALIZAR_ARMADO";
                case Mensajes::PEDIDO_DETECTAR_FRECUENCIA: return s + "PEDIDO_DETECTAR_FRECUENCIA";
                case Mensajes::PEDIDO_DESPACHAR_DISPOSITIVO: return s + "PEDIDO_DESPACHAR_DISPOSITIVO";
                default: return s + "NO_TIPADO";
            }
        }else if( Tipo < 12000 ){
            s = "RESPUESTA_A_ROBOT_ARMADO-";
            switch( Msg ){
               case Mensajes::RESPUESTA_PLATAFORMA_LLENA_NO: return s + "RESPUESTA_PLATAFORMA_LLENA_NO";
               case Mensajes::RESPUESTA_PLATAFORMA_LLENA_SI: return s + "RESPUESTA_PLATAFORMA_LLENA_SI";
               case Mensajes::RESPUESTA_INICIAR_ARMADO: return s + "RESPUESTA_INICIAR_ARMADO";
               default: return s + "NO_TIPADA";
            }
        }else{
            return "RESPUESTA_A_ROBOT_DESPACHO-NO_TIPADA";
        }
    }
} MensajeColaPlataforma;

}

