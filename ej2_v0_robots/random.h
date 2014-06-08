#pragma once

#include <cstdlib>

namespace Random{
    inline bool BooleanoAleatorio(){
        return (rand() % 2 == 0);
    }

    inline int NumeroAleatorio( int min, int max ){
        return min + rand() % (max-min+1);
    }

    inline void DemoraAleatoriaEnMilis( std::pair<int,int> minYMaxMilis ){
        int demora = minYMaxMilis.first + rand() % ( minYMaxMilis.second - minYMaxMilis.first );
        usleep( demora * 1000 );
    }
}
