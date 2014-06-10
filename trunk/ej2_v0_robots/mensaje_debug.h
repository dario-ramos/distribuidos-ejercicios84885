#pragma once

#include <stdio.h>
#include <cerrno>
#include <cstdarg>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

namespace Utils{

    const int NEGRO = 30;
    const int ROJO = 31;
    const int VERDE = 32;
    const int AMARILLO = 33;
    const int AZUL = 34;
    const int MAGENTA = 35;
    const int CIAN = 36;
    const int BLANCO = 37;
    const int MAX_LONG_MENSAJE = 1024;
    const std::string INICIO_HEADER_COLOR_VT100 = "\x1b[";
    const std::string FIN_HEADER_COLOR_VT100 = "m";
    const std::string FOOTER_COLOR_VT100 = "\x1b[0m\n";

    inline std::string StringHoraActual(){
      time_t rawtime;
      struct tm * timeinfo;
      char buffer [128];
      time ( &rawtime );
      timeinfo = localtime ( &rawtime );
      strftime (buffer,16,"%I:%M:%S",timeinfo);
      return std::string( buffer );
    }

    //TODO: No compila
    //inline void MensajeDebug( const std::string& nombreProceso, const char* fmt, ...){
	//	va_list args;
    //    va_start(args,fmt);
    //    MensajeDebug( nombreProceso, BLANCO, fmt, args);
    //    va_end(args);
    //}

    inline void MensajeDebug( const std::string& nombreProceso, int color, const char* fmt, ...){
        va_list args;
        va_start(args,fmt);
        char buffer[MAX_LONG_MENSAJE];
        vsprintf(buffer, fmt, args);
        std::string mensaje = INICIO_HEADER_COLOR_VT100 +
                              std::to_string(color) + FIN_HEADER_COLOR_VT100 + 
                              StringHoraActual() +
                              " [" + nombreProceso + "]: " + std::string( buffer ) +
                              FOOTER_COLOR_VT100;
        write( fileno(stdout), mensaje.c_str(), mensaje.length() );
        va_end(args);
    }
    
    inline void MensajeError( const std::string& nombreProceso, const char* fmt, ... ){
        va_list args;
        va_start(args,fmt);
        char buffer[MAX_LONG_MENSAJE];
        vsprintf(buffer, fmt, args);
        std::string mensaje = INICIO_HEADER_COLOR_VT100 + std::to_string(ROJO) +
                              FIN_HEADER_COLOR_VT100 + StringHoraActual() +
                               " [" + nombreProceso + "] " + std::string( buffer ) +
                                ". Errno:" + std::string(strerror(errno)) +
                                FOOTER_COLOR_VT100;
        write( fileno(stderr), mensaje.c_str(), mensaje.length() );
        va_end(args);
    }
}
