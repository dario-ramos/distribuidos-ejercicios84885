#pragma once

#include <string>
#include <map>

class Configuracion{
    std::map<std::string, std::string>	m_Parametros;
    std::string ObtenerValorParametro( const std::string& nombreParam ) const;
public:
    static const std::string DEFAULT_FILE_PATH;
    Configuracion(){}
    bool LeerDeArchivo( std::string path = DEFAULT_FILE_PATH );
    int ObtenerCantidadDispositivosAGenerar() const;
    int ObtenerCantidadInicialPilas() const;
    int ObtenerCantidadRobots() const;
    int ObtenerCantidadTiposDeDispositivo() const;
    int ObtenerCantMaxDispositivos() const;
    int ObtenerCapacidadPlataforma() const;
    int ObtenerIdBaseColasDispositivos() const;
    int ObtenerIdBaseColasRobotsEmpaque() const;
    int ObtenerIdColaDispositivo( int idDispositivo ) const;
    int ObtenerIdColaPlataforma() const;
    int ObtenerIdColaRobotEmpaque( int idRobot ) const;
    int ObtenerIdMutexPlataforma() const;
    int ObtenerIdSemaforoCinta() const;
    int ObtenerIdSemaforoRobotArmado( int nroRobot ) const;
    int ObtenerIdSemaforoRobotDespacho( int nroRobot ) const;
    int ObtenerIdShmPlataforma() const;
    std::pair<int,int> ObtenerDemoraActivacion() const;
    std::pair<int,int> ObtenerDemoraArmado() const;
    std::pair<int,int> ObtenerDemoraDespacho() const;
    std::pair<int,int> ObtenerDemoraEmpaque() const;
    std::string ObtenerDirFtok() const;
};

