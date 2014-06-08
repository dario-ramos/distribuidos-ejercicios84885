CC = g++
CFLAGS = -g -std=c++11 -Wall
LDFLAGS = -lm
BINARIES = control_plataforma finalizador lanzador robot_armado robot_despacho

all : ${BINARIES}

clean :
	rm -f *.o *~ ${BINARIES}

configuracion.o : configuracion.cpp configuracion.h string_utils.h
	${CC} ${CFLAGS} -c configuracion.cpp
	
control_plataforma.o : control_plataforma.cpp mensaje_debug.h core.h configuracion.h t_sem.h random.h
	${CC} ${CFLAGS} -c control_plataforma.cpp

control_plataforma : control_plataforma.o configuracion.o
	${CC} ${CFLAGS} control_plataforma.o configuracion.o ${LDFLAGS} -o control_plataforma

finalizador.o : finalizador.cpp mensaje_debug.h configuracion.h core.h t_sem.h
	${CC} ${CFLAGS} -c finalizador.cpp

finalizador : finalizador.o configuracion.o
	${CC} ${CFLAGS} finalizador.o configuracion.o ${LDFLAGS} -o finalizador

lanzador.o : lanzador.cpp mensaje_debug.h configuracion.h core.h t_sem.h
	${CC} ${CFLAGS} -c lanzador.cpp

lanzador : lanzador.o configuracion.o
	${CC} ${CFLAGS} lanzador.o configuracion.o ${LDFLAGS} -o lanzador

plataforma.o: plataforma.cpp iplataforma_armado.h iplataforma_despacho.h mensaje_debug.h core.h
	${CC} ${CFLAGS} -c plataforma.cpp

robot_armado.o : robot_armado.cpp mensaje_debug.h core.h plataforma.h configuracion.h random.h
	${CC} ${CFLAGS} -c robot_armado.cpp

robot_armado : robot_armado.o plataforma.o configuracion.o
	${CC} ${CFLAGS} robot_armado.o plataforma.o configuracion.o ${LDFLAGS} -o robot_armado

robot_despacho.o : robot_despacho.cpp mensaje_debug.h core.h configuracion.h plataforma.h random.h
	${CC} ${CFLAGS} -c robot_despacho.cpp

robot_despacho : robot_despacho.o configuracion.o plataforma.o
	${CC} ${CFLAGS} robot_despacho.o configuracion.o plataforma.o ${LDFLAGS} -o robot_despacho
