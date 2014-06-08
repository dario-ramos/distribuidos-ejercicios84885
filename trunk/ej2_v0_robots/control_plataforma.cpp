#include "core.h"
#include "configuracion.h"
#include "dispositivo.h"
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

//Por ahora, se toma el primer dispositivo encontrado
void BuscarYReservarLugar( Robots2::ShmPlataforma* pPlataforma, int nroRobot, const std::string& nombreProceso ){
    int capacidad = pPlataforma->Capacidad;
    for( int i=0; i < capacidad; i++ ){
        if( pPlataforma->EstadoDePosiciones[i] != Robots2::EPP_OCUPADA_SIN_ARMAR )
            continue;
        pPlataforma->EstadoDePosiciones[i] = Robots2::EPP_RESERVADA;
        pPlataforma->EspaciosOcupados++;
        g_PosicionesReservadas[nroRobot] = i;
        MENSAJE_DEBUG( "Posicion %d reservada por robot %d, espacios ocupados/reservados: %d/%d",
                       i, nroRobot, pPlataforma->EspaciosOcupados, capacidad );
        break;
    }
}

void ProcesarMensajePreguntaPlataformaVacia( Robots2::ShmPlataforma* pPlataforma,
                                             t_sem& mutex,
                                             int idCola,
                                             const string& nombreProceso, 
                                             int nRobot ){
    MENSAJE_DEBUG( "Mensaje PREGUNTA_PLATAFORMA_VACIA recibido, procesando..." );
    mutex.P();
    bool vacia = (pPlataforma->EspaciosOcupados == 0);
    Robots2::MensajeColaPlataforma rta = {
        Robots2::TipoMensajes::RespuestaARobotArmado(nRobot), nRobot,
        vacia? Robots2::MensajesPlataforma::RESPUESTA_PLATAFORMA_VACIA_SI :
               Robots2::MensajesPlataforma::RESPUESTA_PLATAFORMA_VACIA_NO };
    if( !vacia )
        BuscarYReservarLugar( pPlataforma, nRobot, nombreProceso );
    int codError = msgsnd( idCola, &rta, sizeof(rta) - sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar RESPUESTA_PLATAFORMA - PREGUNTA_PLATAFORMA_LLENA" );
        exit( 5 );
    }
    mutex.V();
}

void ProcesarMensajeIniciarArmado( Robots2::ShmPlataforma* pPlataforma,
                                   t_sem& mutex,
                                   int idCola,
                                   std::vector< std::unique_ptr<t_sem> >& semaforosArmado,
                                   std::vector< std::unique_ptr<t_sem> >& semaforosDespacho,
                                   const string& nombreProceso, int nRobot ){
    MENSAJE_DEBUG( "Mensaje INICIAR_ARMADO recibido, esperando que robot despacho %d este libre...",
                   nRobot );
    t_sem* pSem = semaforosDespacho[ nRobot - 1 ].get();
    if( pSem->P() != 0){
        MENSAJE_ERROR( "Error en P de semaforosDespacho" );
        exit( 1 );
    }
    MENSAJE_DEBUG( "Tomando mutex de armado para robot %d...", nRobot );
    semaforosArmado[ nRobot - 1 ]->P();
    MENSAJE_DEBUG( "Mutex de armado para robot %d tomado", nRobot );
    //Obtener id de dispositivo a armar
    int idDisp = -1;
    mutex.P();
    int posDisp = g_PosicionesReservadas[nRobot];
    idDisp = pPlataforma->Dispositivos[posDisp];
    mutex.V();
    //Enviar mensaje a robot armado incluyendo id disp
    Robots2::MensajeColaPlataforma respuesta = {
        Robots2::TipoMensajes::RespuestaARobotArmado(nRobot),  //Tipo
        nRobot,                                                //NroRobot
        Robots2::MensajesPlataforma::RESPUESTA_INICIAR_ARMADO, //Msg
        idDisp                                                 //DatosMsg
    };
    int codError = msgsnd( idCola, &respuesta, sizeof(respuesta)-sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar RESPUESTA_PLATAFORMA - RESPUESTA_INICIAR_ARMADO " );
        exit( 5 );
    }
}

void ProcesarMensajeFinalizarArmado( Robots2::ShmPlataforma* pPlataforma,
                                     t_sem& mutex,
                                     int idCola,
                                     std::vector< std::unique_ptr<t_sem> >& semaforosArmado,
                                     std::vector< std::unique_ptr<t_sem> >& semaforosDespacho,
                                     const Configuracion& config,
                                     const string& nombreProceso,
                                     int nroRobot ){
    //Activar disp
    mutex.P();
    semaforosArmado[nroRobot - 1]->V();
    semaforosDespacho[nroRobot - 1]->V();
    MENSAJE_DEBUG( "¡Mutex de armado para robot %d liberado!", nroRobot );
    int posDisp = g_PosicionesReservadas[nroRobot];    
    pPlataforma->EstadoDePosiciones[ posDisp ] = Robots2::EPP_OCUPADA_INACTIVA;
    int idDisp = pPlataforma->Dispositivos[posDisp];
    mutex.V();
    std::unique_ptr<IDispositivo> pDispositivo( new Dispositivo(idDisp, config, nombreProceso) );
    MENSAJE_DEBUG( "Esperando activacion de dispositivo en posicion %d...", posDisp );
    pDispositivo->EsperarFinActivacion();
    mutex.P();
    pPlataforma->EstadoDePosiciones[posDisp] = Robots2::EPP_OCUPADA_ACTIVA;
    mutex.V();
    MENSAJE_DEBUG( "¡Dispositivo en posicion %d activado!", g_PosicionesReservadas[nroRobot] );
    //Seleccionar robot que despachara disp
    int posEnSet = rand() % g_RobotsDetectandoFrecuencias.size();
    std::set<int>::const_iterator it( g_RobotsDetectandoFrecuencias.begin() );
    advance( it, posEnSet );
    int robotQueDetectaFrecuencia = *it;
    //En este mensaje se usa el campo NroRobot para enviar la posicion del disp TODO <REF>
    int posDispActivado = nroRobot;
    MENSAJE_DEBUG( "Robot %d detecto frecuencia de dispositivo en posicion %d", robotQueDetectaFrecuencia, posDispActivado );
    MENSAJE_DEBUG( "Esperando que robot %d este libre para comenzar despacho", robotQueDetectaFrecuencia );
    semaforosArmado[robotQueDetectaFrecuencia - 1]->P();
    semaforosDespacho[robotQueDetectaFrecuencia - 1]->P();
    Robots2::MensajeColaPlataforma mensaje = {
    Robots2::TipoMensajes::RespuestaARobotDespacho(robotQueDetectaFrecuencia), robotQueDetectaFrecuencia, posDispActivado };
    int codError = msgsnd( idCola, &mensaje, sizeof(mensaje) - sizeof(long), 0 );
    if( codError == -1 ){
        MENSAJE_ERROR( "Error al enviar RESPUESTA_PLATAFORMA - frecuencia detectada " );
        exit( 5 );
    }
}

void ProcesarMensajeDespacharDispositivo( Robots2::ShmPlataforma* pPlataforma,
                                          t_sem& mutex, t_sem& semCintaEntrada,
                                          std::vector< std::unique_ptr<t_sem> >& semaforosArmado,
                                          std::vector< std::unique_ptr<t_sem> >& semaforosDespacho,
                                          const Configuracion& config,
                                          const string& nombreProceso,
                                          int nroRobot, int posDisp ){
    mutex.P();
    MENSAJE_DEBUG( "Robot %d despachando disp en pos %d", nroRobot, posDisp );
    pPlataforma->EstadoDePosiciones[ posDisp ] = Robots2::EPP_LIBRE;
    pPlataforma->EspaciosOcupados--;
    //Si la plataforma estaba llena, destrabar cinta entrada
    if( pPlataforma->EspaciosOcupados == pPlataforma->Capacidad-1 )
        semCintaEntrada.V();
    MENSAJE_DEBUG( "¡Dispositivo de posicion %d despachado! Espacios ocupados: %d/%d", posDisp,
                    pPlataforma->EspaciosOcupados, pPlataforma->Capacidad );
    semaforosArmado[nroRobot - 1]->V();
    semaforosDespacho[nroRobot - 1]->V();
    int idDisp = pPlataforma->Dispositivos[posDisp];
    int tipoDisp = pPlataforma->TipoDispositivo[posDisp];
    std::unique_ptr<IDispositivo> pDispositivo( new Dispositivo(idDisp, config, nombreProceso) );
    pPlataforma->Dispositivos[posDisp] = -1;
    pPlataforma->TipoDispositivo[posDisp] = -1;
    pDispositivo->Despachar( nroRobot );
    mutex.V();
}

void ProcesarMensaje( Robots2::ShmPlataforma* pPlataforma,
                      t_sem& mutex, t_sem& semCintaEntrada,
                      std::vector< std::unique_ptr<t_sem> >& semaforosArmado,
                      std::vector< std::unique_ptr<t_sem> >& semaforosDespacho,
                      int idCola,
                      const Configuracion& config,
                      const string& nombreProceso,
                      const Robots2::MensajeColaPlataforma& pedido ){
    //MENSAJE_DEBUG( "Procesando pedido %s", pedido.ToString().c_str() );
    //MensajeDebug( nombreProceso, Utils::VERDE, "Procesando pedido %s de robot %d", pedido.ToString().c_str(), pedido.NroRobot );
    switch( pedido.Msg ){
        case Robots2::MensajesPlataforma::PREGUNTA_PLATAFORMA_VACIA:
            ProcesarMensajePreguntaPlataformaVacia( pPlataforma, mutex, idCola, nombreProceso, pedido.NroRobot );
            break;
        case Robots2::MensajesPlataforma::PEDIDO_INICIAR_ARMADO:
            ProcesarMensajeIniciarArmado( pPlataforma, mutex, idCola, semaforosArmado, semaforosDespacho,
                                          nombreProceso, pedido.NroRobot );
            break;            
        case Robots2::MensajesPlataforma::PEDIDO_FINALIZAR_ARMADO:
            ProcesarMensajeFinalizarArmado( pPlataforma, mutex, idCola, semaforosArmado, semaforosDespacho,
                                            config, nombreProceso, pedido.NroRobot );
            break;
        case Robots2::MensajesPlataforma::PEDIDO_DETECTAR_FRECUENCIA:
            g_RobotsDetectandoFrecuencias.insert( pedido.NroRobot );
            MENSAJE_DEBUG( "Robot %d detectando frecuencia", pedido.NroRobot );
            break;
        case Robots2::MensajesPlataforma::PEDIDO_DESPACHAR_DISPOSITIVO:
            ProcesarMensajeDespacharDispositivo( pPlataforma, mutex, semCintaEntrada,
                                                 semaforosArmado, semaforosDespacho,
                                                 config, nombreProceso,
                                                 pedido.NroRobot, pedido.DatosMsg );
            break;
        default:
            MENSAJE_ERROR( "Mensaje desconocido recibido" );
            break;
        }
        //MensajeDebug( nombreProceso, Utils::ROJO, "FIN proceso pedido %s", pedido.ToString().c_str() );
}

int main( int argc, char** argv ){
    string nombreProceso = Robots2::Constantes::NOMBRE_PROCESO_PLATAFORMA;
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
    Robots2::ShmPlataforma* pPlataforma;
    int idShmPlataforma = config.ObtenerIdShmPlataforma();
    int handleShmPlataforma = shmget( ftok(dirFtok.c_str(),idShmPlataforma), sizeof(pPlataforma), IPC_CREAT | 0660 );
    if( handleShmPlataforma == -1 ){
        MENSAJE_ERROR( "Error al acceder a la Shared Memory de la Plataforma, id %d", idShmPlataforma );
        exit(1);
    }
    pPlataforma = static_cast<Robots2::ShmPlataforma*>( shmat(handleShmPlataforma, 0, 0) );
    if( pPlataforma == reinterpret_cast<Robots2::ShmPlataforma*>(-1) ){
        MENSAJE_ERROR( "Error al atachear a Shared Memory de la Plataforma, id %d", idShmPlataforma );
        exit(1);
    }
    //Obtener mutex y semaforos de robots y comenzar a atender pedidos
    t_sem mutex( ftok(dirFtok.c_str(),config.ObtenerIdMutexPlataforma()) );
    std::vector< std::unique_ptr<t_sem> > semaforosArmado;
    std::vector< std::unique_ptr<t_sem> > semaforosDespacho;
    for( int i=0; i<config.ObtenerCantidadRobots(); i++ ){
        semaforosArmado.push_back( std::unique_ptr<t_sem>( new t_sem( ftok( dirFtok.c_str(), config.ObtenerIdSemaforoRobotArmado(i)) ) ) );
        semaforosDespacho.push_back( std::unique_ptr<t_sem>( new t_sem( ftok( dirFtok.c_str(), config.ObtenerIdSemaforoRobotDespacho(i) ) ) ) );
    }
    //Obtener semaforo de cinta entrada
    t_sem semCintaEntrada( ftok(dirFtok.c_str(),config.ObtenerIdSemaforoCinta()) );
    //Instanciar componentes de robots empacadores
    std::vector< std::unique_ptr<IRobotEmpaque> > robotsEmpaque;
    for( int i=1; i<= config.ObtenerCantidadTiposDeDispositivo(); i++ )
        robotsEmpaque.push_back( new RobotEmpaque(i) );
    while( true ){
        //MENSAJE_DEBUG( "Esperando pedido..." );
        Robots2::MensajeColaPlataforma mensaje;
        int codError = msgrcv( cola, &mensaje, sizeof(mensaje), Robots2::TipoMensajes::PEDIDO_A_PLATAFORMA, 0 );
        if( codError == -1 ){
            MENSAJE_ERROR( "Error al recibir PEDIDO_A_PLATAFORMA" );
            exit(1);
        }
        ProcesarMensaje( pPlataforma, mutex, semCintaEntrada, semaforosArmado, semaforosDespacho,
                         cola, config, nombreProceso, mensaje );
    }
    MENSAJE_DEBUG("PROCESO FINALIZADO");
    return 0;
}
