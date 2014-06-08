#include "core.h"
#include "configuracion.h"
#include "random.h"
#include "mensaje_debug.h"
#include "t_sem.h"
#include <sys/msg.h>
#include <sys/shm.h>
#include <vector>
#include <set>
#include <memory>

using namespace Utils;
using std::string;

//Para no repetir el primer parametro en cada llamada
#define MENSAJE_DEBUG(fmt, ...)  MensajeDebug( nombreProceso, Utils::AMARILLO, fmt, ##__VA_ARGS__ )
#define MENSAJE_ERROR(fmt, ...)  MensajeError( nombreProceso, fmt, ##__VA_ARGS__ )

std::map<int,int> g_PosicionesReservadas; //Clave: numRobot, valor: posicionArmado
std::set<int>     g_RobotsDetectandoFrecuencias;

//Por ahora, se toma la primera posicion libre encontrada
void BuscarYReservarLugar( Robots::ShmPlataforma* pPlataforma, int nroRobot, const std::string& nombreProceso ){
      int capacidad = pPlataforma->Capacidad;
      for( int i=0; i < capacidad; i++ ){
            if( pPlataforma->EstadoDePosiciones[i] != Robots::EPP_LIBRE )
                continue;
            pPlataforma->EstadoDePosiciones[i] = Robots::EPP_RESERVADA;
            pPlataforma->EspaciosOcupados++;
            g_PosicionesReservadas[nroRobot] = i;
            MENSAJE_DEBUG( "Posicion %d reservada por robot %d, espacios ocupados/reservados: %d/%d",
                           i, nroRobot, pPlataforma->EspaciosOcupados, capacidad );
            break;
      }
}

void ProcesarPedido( Robots::ShmPlataforma* pPlataforma, t_sem& mutex,
                     std::vector< t_sem* >& semaforosArmado,
                     std::vector< t_sem* >& semaforosDespacho,
                     int idCola,
                     const string& nombreProceso,
                     const Robots::MensajeColaPlataforma& pedido,
                     const Configuracion& config ){
        //MENSAJE_DEBUG( "Procesando pedido %s", pedido.ToString().c_str() );
        //MensajeDebug( nombreProceso, Utils::VERDE, "Procesando pedido %s de robot %d", pedido.ToString().c_str(), pedido.NroRobot );
        switch( pedido.Msg ){
          case Robots::Mensajes::PREGUNTA_PLATAFORMA_LLENA:{
                MENSAJE_DEBUG( "Mensaje PREGUNTA_PLATAFORMA_LLENA recibido, procesando..." );
                mutex.P();
                bool llena = (pPlataforma->EspaciosOcupados == pPlataforma->Capacidad);
                Robots::MensajeColaPlataforma mensaje = {
                  Robots::TipoMensajes::RespuestaARobotArmado(pedido.NroRobot), pedido.NroRobot,
                  llena? Robots::Mensajes::RESPUESTA_PLATAFORMA_LLENA_SI : Robots::Mensajes::RESPUESTA_PLATAFORMA_LLENA_NO };
                if( !llena )
                      BuscarYReservarLugar( pPlataforma, pedido.NroRobot, nombreProceso );
                int codError = msgsnd( idCola, &mensaje, sizeof(mensaje) - sizeof(long), 0 );
                if( codError == -1 ){
                      MENSAJE_ERROR( "Error al enviar RESPUESTA_PLATAFORMA - PREGUNTA_PLATAFORMA_LLENA" );
                      exit( 5 );
                }
                mutex.V();
          }break;
          case Robots::Mensajes::PEDIDO_INICIAR_ARMADO:{
                MENSAJE_DEBUG( "Mensaje INICIAR_ARMADO recibido, esperando que robot despacho %d este libre...",
                               pedido.NroRobot );
                t_sem* pSem = semaforosDespacho[ pedido.NroRobot - 1 ];
                if( pSem->P() != 0){
                                                MENSAJE_ERROR( "Error en P de semaforosDespacho" );
                                                exit( 1 );
                                        }
                MENSAJE_DEBUG( "Tomando mutex de armado para robot %d...", pedido.NroRobot );
                semaforosArmado[ pedido.NroRobot - 1 ]->P();
                MENSAJE_DEBUG( "Mutex de armado para robot %d tomado", pedido.NroRobot );
                Robots::MensajeColaPlataforma mensaje = {
                      Robots::TipoMensajes::RespuestaARobotArmado(pedido.NroRobot), pedido.NroRobot,
                      Robots::Mensajes::RESPUESTA_INICIAR_ARMADO };
                int codError = msgsnd( idCola, &mensaje, sizeof(mensaje) - sizeof(long), 0 );
                if( codError == -1 ){
                      MENSAJE_ERROR( "Error al enviar RESPUESTA_PLATAFORMA - RESPUESTA_INICIAR_ARMADO " );
                      exit( 5 );
                }
          }break;
          case Robots::Mensajes::PEDIDO_FINALIZAR_ARMADO:{
                //Activar disp
                mutex.P();
                semaforosArmado[pedido.NroRobot - 1]->V();
                semaforosDespacho[pedido.NroRobot - 1]->V();
                MENSAJE_DEBUG( "¡Mutex de armado para robot %d liberado!", pedido.NroRobot );
                int posDisp = g_PosicionesReservadas[pedido.NroRobot];
                pPlataforma->EstadoDePosiciones[ posDisp ] = Robots::EPP_OCUPADA_INACTIVA;
                mutex.V();
                MENSAJE_DEBUG( "Activando dispositivo en posicion %d...", posDisp );
                Random::DemoraAleatoriaEnMilis( config.ObtenerDemoraActivacion() );
                mutex.P();
                pPlataforma->EstadoDePosiciones[posDisp] = Robots::EPP_OCUPADA_ACTIVA;
                mutex.V();
                MENSAJE_DEBUG( "¡Dispositivo en posicion %d activado!", g_PosicionesReservadas[pedido.NroRobot] );
                //Seleccionar robot que despachara disp
                int posEnSet = rand() % g_RobotsDetectandoFrecuencias.size();
                std::set<int>::const_iterator it( g_RobotsDetectandoFrecuencias.begin() );
                advance( it, posEnSet );
                int robotQueDetectaFrecuencia = *it;
                //En este mensaje se usa el campo NroRobot para enviar la posicion del disp TODO <REF>
                int posDispActivado = pedido.NroRobot;
                MENSAJE_DEBUG( "Robot %d detecto frecuencia de dispositivo en posicion %d", robotQueDetectaFrecuencia, posDispActivado );
                MENSAJE_DEBUG( "Esperando que robot %d este libre para comenzar despacho", robotQueDetectaFrecuencia );
                semaforosArmado[robotQueDetectaFrecuencia - 1]->P();
                semaforosDespacho[robotQueDetectaFrecuencia - 1]->P();
                Robots::MensajeColaPlataforma mensaje = {
                Robots::TipoMensajes::RespuestaARobotDespacho(robotQueDetectaFrecuencia), robotQueDetectaFrecuencia, posDispActivado };
                int codError = msgsnd( idCola, &mensaje, sizeof(mensaje) - sizeof(long), 0 );
                if( codError == -1 ){
                      MENSAJE_ERROR( "Error al enviar RESPUESTA_PLATAFORMA - frecuencia detectada " );
                      exit( 5 );
                }
          }break;
          case Robots::Mensajes::PEDIDO_DETECTAR_FRECUENCIA:
                g_RobotsDetectandoFrecuencias.insert( pedido.NroRobot );
                MENSAJE_DEBUG( "Robot %d detectando frecuencia", pedido.NroRobot );
                break;
          case Robots::Mensajes::PEDIDO_DESPACHAR_DISPOSITIVO:{
                mutex.P();
                int posDisp = pedido.DatosMsg;
                MENSAJE_DEBUG( "Robot %d despachando disp en pos %d", pedido.NroRobot, posDisp );
                pPlataforma->EstadoDePosiciones[ posDisp ] = Robots::EPP_LIBRE;
                pPlataforma->EspaciosOcupados--;
                MENSAJE_DEBUG( "¡Dispositivo de posicion %d despachado! Espacios ocupados: %d/%d", pedido.DatosMsg,
                               pPlataforma->EspaciosOcupados, pPlataforma->Capacidad );
                semaforosArmado[pedido.NroRobot - 1]->V();
                semaforosDespacho[pedido.NroRobot - 1]->V();
                mutex.V();
          } break;                  
          default:
                MENSAJE_ERROR( "Mensaje desconocido recibido" );
                break;
        }
        //MensajeDebug( nombreProceso, Utils::ROJO, "FIN proceso pedido %s", pedido.ToString().c_str() );
}

void ValidarArgumentos( int argc, char** argv ){
      if( argc < 2 ){
            MensajeError( Robots::Constantes::NOMBRE_PROCESO_PLATAFORMA, "Faltan parametros. Uso: %s <numRobot>", argv[0] );
            exit(-1);
      }
}

int main( int argc, char** argv ){
        ValidarArgumentos( argc, argv );
    string nombreProceso = Robots::Constantes::NOMBRE_PROCESO_PLATAFORMA +
						   string( argv[1] );
	MENSAJE_DEBUG("PROCESO INICIADO");
	//Obtener cola de mensajes
	Configuracion config;
	if( !config.LeerDeArchivo() ){
		MENSAJE_DEBUG( "Error al leer archivo de configuracion, revise formato" );
		MENSAJE_DEBUG("PROCESO FINALIZADO");
		exit( -1 );
	}
	int idCola = config.ObtenerIdColaPlataforma();
	string dirFtok = config.ObtenerDirFtok();
	int cola = msgget( ftok( dirFtok.c_str(), idCola ), 0660 );
	if( cola == -1 ){
		MENSAJE_ERROR( "Error al conectarse a la cola %d", idCola );
		exit(1);
	}
	//Obtener shared memory
	Robots::ShmPlataforma* pPlataforma;
	int idShmPlataforma = config.ObtenerIdShmPlataforma();
	int handleShmPlataforma = shmget( ftok(dirFtok.c_str(),idShmPlataforma), sizeof(pPlataforma), IPC_CREAT | 0660 );
	if( handleShmPlataforma == -1 ){
		MENSAJE_ERROR( "Error al acceder a la Shared Memory de la Plataforma, id %d", idShmPlataforma );
		exit(1);
	}
	pPlataforma = static_cast<Robots::ShmPlataforma*>( shmat(handleShmPlataforma, 0, 0) );
	if( pPlataforma == reinterpret_cast<Robots::ShmPlataforma*>(-1) ){
		MENSAJE_ERROR( "Error al atachear a Shared Memory de la Plataforma, id %d", idShmPlataforma );
		exit(1);
	}
	//Obtener mutex y semaforos de robots y comenzar a atender pedidos
	t_sem mutex( ftok(dirFtok.c_str(),config.ObtenerIdMutexPlataforma()) );
	std::vector< t_sem* > semaforosArmado;
	std::vector< t_sem* > semaforosDespacho;
    for( int i=0; i<config.ObtenerCantidadRobots(); i++ ){
	  semaforosArmado.push_back( new t_sem( ftok( dirFtok.c_str(), config.ObtenerIdSemaforoRobotArmado(i)) ) );
	  semaforosDespacho.push_back( new t_sem( ftok( dirFtok.c_str(), config.ObtenerIdSemaforoRobotDespacho(i) ) ) );
	}
	while( true ){
		//MENSAJE_DEBUG( "Esperando pedido..." );
		Robots::MensajeColaPlataforma mensaje;
		int codError = msgrcv( cola, &mensaje,
                                      sizeof(mensaje) - sizeof(long),
                                      Robots::TipoMensajes::PEDIDO_A_PLATAFORMA, 0 );
		if( codError == -1 ){
			  MENSAJE_ERROR( "Error al recibir PEDIDO_A_PLATAFORMA" );
			  exit(1);
		}
		ProcesarPedido( pPlataforma, mutex, semaforosArmado, semaforosDespacho, cola, nombreProceso, mensaje, config );
	}
	MENSAJE_DEBUG("PROCESO FINALIZADO");
	return 0;
}
