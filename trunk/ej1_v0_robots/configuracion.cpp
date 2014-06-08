#include "configuracion.h"
#include "string_utils.h"
#include <fstream>

using namespace std;

const string Configuracion::DEFAULT_FILE_PATH = "config.ini";

bool Configuracion::LeerDeArchivo( std::string path ){
      ifstream archivoConfig( path.c_str() );
      if( !archivoConfig.is_open() )
            return false;
      while( !archivoConfig.eof() ){
            string linea;
            getline( archivoConfig, linea );
            vector<string> claveYValor = StringUtils::Split( linea, '=' );
            if( claveYValor.size() != 2 )
                  continue;
            m_Parametros[ claveYValor[0] ] = claveYValor[1];
      }
      return true;
}

string Configuracion::ObtenerValorParametro( const string& nombreParam ) const{
      map<string,string>::const_iterator it = m_Parametros.find( nombreParam );
      if( it == m_Parametros.end() )
            return "";
      return it->second;
}

string Configuracion::ObtenerDirFtok() const{
      return ObtenerValorParametro( "DirectorioFtok" );
}

int Configuracion::ObtenerCantidadInicialDispositivosSinPila() const{
      return StringUtils::Parse<int>( ObtenerValorParametro( "CantidadInicialDispositivosSinPila" ) );
}

int Configuracion::ObtenerCantidadInicialPilas() const{
      return StringUtils::Parse<int>( ObtenerValorParametro( "CantidadInicialPilas" ) );
}

int Configuracion::ObtenerCantidadRobots() const{
      return StringUtils::Parse<int>( ObtenerValorParametro( "CantidadRobots" ) );
}

int Configuracion::ObtenerCapacidadPlataforma() const{
      return StringUtils::Parse<int>( ObtenerValorParametro( "CapacidadPlataforma" ) );
}

int Configuracion::ObtenerIdColaPlataforma() const{
      return std::stoi( ObtenerValorParametro( "IdColaPlataforma" ) );
}

int Configuracion::ObtenerIdMutexPlataforma() const{
      return std::stoi( ObtenerValorParametro( "IdMutexPlataforma" ) );
}

int Configuracion::ObtenerIdShmPlataforma() const{
      return std::stoi( ObtenerValorParametro( "IdShmPlataforma" ) );
}

int Configuracion::ObtenerIdSemaforoRobotArmado( int nroRobot ) const{
      return std::stoi( ObtenerValorParametro( "IdBaseSemaforosRobotsArmado" ) ) + nroRobot;
}

int Configuracion::ObtenerIdSemaforoRobotDespacho( int nroRobot ) const{
      return std::stoi( ObtenerValorParametro( "IdBaseSemaforosRobotsDespacho" ) ) + nroRobot;
}

std::pair<int,int> Configuracion::ObtenerDemoraActivacion() const{
    return std::pair<int,int>( std::stoi( ObtenerValorParametro("MinDemoraActivacion") ),
                               std::stoi( ObtenerValorParametro("MaxDemoraActivacion") ) );
}

std::pair<int,int> Configuracion::ObtenerDemoraArmado() const{
    return std::pair<int,int>( std::stoi( ObtenerValorParametro("MinDemoraArmado") ),
                               std::stoi( ObtenerValorParametro("MaxDemoraArmado") ) );
}

std::pair<int,int> Configuracion::ObtenerDemoraDespacho() const{
    return std::pair<int,int>( std::stoi( ObtenerValorParametro("MinDemoraDespacho") ),
                               std::stoi( ObtenerValorParametro("MaxDemoraDespacho") ) );
}
