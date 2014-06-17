#pragma once

#include <thread>
#include <chrono>

namespace Random{
    inline bool BooleanoAleatorio(){
        return (rand() % 2 == 0);
    }

    inline int NumeroAleatorio( int min, int max ){
        return min + rand() % (max-min+1);
    }

    inline void DemoraAleatoriaEnMilis( std::pair<int,int> minYMaxMilis ){
        int demora = minYMaxMilis.first + rand() % ( std::max( minYMaxMilis.second - minYMaxMilis.first, 1 ) );
        std::this_thread::sleep_for( std::chrono::milliseconds(demora) );
    }
}
