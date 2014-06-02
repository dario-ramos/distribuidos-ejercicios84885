/* 
 * File:   t_sem.h
 * Author: root
 *
 * Created on August 22, 2011, 1:22 AM
 */

#pragma once

#include <sys/sem.h>

class t_sem{
    bool        m_Existe;
    int         m_Semid;
    key_t       m_Clave;    
public:
    t_sem( key_t clave ):
        m_Existe(false),
        m_Semid(-1),
        m_Clave(clave){
            m_Semid = semget( m_Clave, 1, IPC_CREAT | 0660 );
            m_Existe = (m_Semid != -1);
    }
        
    int Inicializar( int valorInicial ){
        if( !m_Existe )
            return -2;
        union semun{
            int                 val;      //For SETVAL
            struct semid_ds*    pBuf;     //For IPC_STAT and IPC_SET
            unsigned short*     pArray;   //For GETALL, SETALL
            struct seminfo*     pInfoBuf; //For IPC_INFO (Linux only)
        } arg;
        arg.val = valorInicial;
        return semctl( m_Semid, 0, SETVAL, arg );
    }

    char buffer[1024];
    
    //P (Proveeren): Wait
    int P(){
        if( !m_Existe )
            return -2;
        struct sembuf oper;
        oper.sem_num = 0;       //Index to Semaphore Array
        oper.sem_op = -1;       //-1: P
        oper.sem_flg = 0;

        //sprintf(buffer, "P sobre %d\n", m_Clave);
        //std::string mensaje = std::string( buffer );
        //write( fileno(stdout), mensaje.c_str(), mensaje.length() );

        return semop( m_Semid, &oper, 1);
    }
    
    //V (Verhoogen): Signal
    int V(){
        if( !m_Existe )
            return -2;
        struct sembuf oper;
        oper.sem_num = 0;
        oper.sem_op = 1;
        oper.sem_flg = 0;

        //sprintf(buffer, "V sobre %d\n", m_Clave);
        //std::string mensaje = std::string( buffer );
        //write( fileno(stdout), mensaje.c_str(), mensaje.length() );

        return semop( m_Semid, &oper, 1 );
    }
    
    int Destruir(){
        m_Existe = false;
        return semctl( m_Semid, 0, IPC_RMID, reinterpret_cast<struct semid_ds*>(0) );
    }
    
    bool Existe() const{
        return m_Existe;
    }
};

