CC = g++
CFLAGS = -std=c++11 -Wall
LDFLAGS = -lm
BINARIES = lanzador finalizador control_cinta_entrada control_dispositivo robot_armado robot_despacho control_robot_empaque control_plataforma

all : ${BINARIES}

clean :
	rm -f *.o *~ ${BINARIES}

configuracion.o : configuracion.cpp configuracion.h string_utils.h
	${CC} ${CFLAGS} -c configuracion.cpp
	
control_plataforma.o : control_plataforma.cpp mensaje_debug.h core.h configuracion.h t_sem.h dispositivo.h robot_empaque.h
	${CC} ${CFLAGS} -c control_plataforma.cpp

control_plataforma : control_plataforma.o configuracion.o dispositivo.o robot_empaque.o
	${CC} ${CFLAGS} control_plataforma.o configuracion.o dispositivo.o robot_empaque.o ${LDFLAGS} -o control_plataforma

finalizador.o : finalizador.cpp mensaje_debug.h core.h configuracion.h
	${CC} ${CFLAGS} -c finalizador.cpp

finalizador : finalizador.o configuracion.o
	${CC} ${CFLAGS} finalizador.o configuracion.o ${LDFLAGS} -o finalizador

lanzador.o : lanzador.cpp mensaje_debug.h configuracion.h core.h
	${CC} ${CFLAGS} -c lanzador.cpp

lanzador : lanzador.o configuracion.o
	${CC} ${CFLAGS} lanzador.o configuracion.o ${LDFLAGS} -o lanzador

plataforma.o: plataforma.cpp iplataforma_armado.h iplataforma_despacho.h mensaje_debug.h core.h
	${CC} ${CFLAGS} -c plataforma.cpp

robot_armado.o : robot_armado.cpp mensaje_debug.h core.h plataforma.h configuracion.h random.h dispositivo.h
	${CC} ${CFLAGS} -c robot_armado.cpp

robot_armado : robot_armado.o plataforma.o configuracion.o dispositivo.o
	${CC} ${CFLAGS} robot_armado.o plataforma.o configuracion.o dispositivo.o ${LDFLAGS} -o robot_armado

robot_despacho.o : robot_despacho.cpp mensaje_debug.h core.h configuracion.h plataforma.h random.h
	${CC} ${CFLAGS} -c robot_despacho.cpp

robot_despacho : robot_despacho.o configuracion.o plataforma.o
	${CC} ${CFLAGS} robot_despacho.o configuracion.o plataforma.o ${LDFLAGS} -o robot_despacho

cinta_entrada.o: cinta_entrada.cpp icinta_entrada.h t_sem.h core.h mensaje_debug.h configuracion.h
	${CC} ${CFLAGS} -c cinta_entrada.cpp

control_cinta_entrada.o: control_cinta_entrada.cpp configuracion.h mensaje_debug.h core.h random.h
	${CC} ${CFLAGS} -c control_cinta_entrada.cpp

control_cinta_entrada: control_cinta_entrada.o cinta_entrada.o configuracion.o
	${CC} ${CFLAGS} control_cinta_entrada.o cinta_entrada.o configuracion.o ${LDFLAGS} -o control_cinta_entrada

dispositivo.o : dispositivo.cpp idispositivo.h configuracion.h mensaje_debug.h core.h random.h
	${CC} ${CFLAGS} -c dispositivo.cpp

control_dispositivo.o: control_dispositivo.cpp core.h mensaje_debug.h dispositivo.h configuracion.h
	${CC} ${CFLAGS} -c control_dispositivo.cpp

control_dispositivo: control_dispositivo.o dispositivo.o configuracion.o
	${CC} ${CFLAGS} control_dispositivo.o dispositivo.o configuracion.o ${LDFLAGS} -o control_dispositivo

robot_empaque.o: robot_empaque.cpp irobot_empaque.h configuracion.h random.h core.h dispositivo.h
	${CC} ${CFLAGS} -c robot_empaque.cpp

control_robot_empaque.o: control_robot_empaque.cpp mensaje_debug.h core.h robot_empaque.h
	${CC} ${CFLAGS} -c control_robot_empaque.cpp

control_robot_empaque: control_robot_empaque.o robot_empaque.o configuracion.o dispositivo.o
	${CC} ${CFLAGS} control_robot_empaque.o robot_empaque.o configuracion.o dispositivo.o ${LDFLAGS} -o control_robot_empaque
