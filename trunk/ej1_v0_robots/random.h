#pragma once

#include <cstdlib>
#include <unistd.h>

namespace Random{
    bool BooleanoAleatorio(){
        return (rand() % 2 == 0);
    }

    void DemoraAleatoriaEnMilis( std::pair<int,int> minYMaxMilis ){
        int demora = minYMaxMilis.first + rand() % ( minYMaxMilis.second - minYMaxMilis.first );
        usleep( demora * 1000 );
    }
}
