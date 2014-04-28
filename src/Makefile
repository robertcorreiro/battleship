#
#
#  Makefile
#  ========
#    
#  Authors: Kyle Poore, Robert Correiro
# 
#

CC = gcc -O3
BUILDDIR = ../build
BINDIR = ../bin
FRAMEWORK = server.c request.c response.c
FRAMEWORK_OBJS = $(FRAMEWORK:%.c=$(BUILDDIR)/%.o)
SERVER = server
CLIENT = client

all: directories ${SERVER} ${CLIENT}

directories:
	mkdir -p ${BUILDDIR}/${MODDIR} ${BINDIR}

#../public/%: ../public/%.c
#	${CC} $< -o $@

${SERVER}: ${FRAMEWORK_OBJS} 
	${CC} ${FRAMEWORK_OBJS} -o ${BINDIR}/${SERVER}

${BUILDDIR}/server.o: ${FRAMEWORK}           \
	                    ${BUILDDIR}/request.o  \
	                    ${BUILDDIR}/response.o
	${CC} -c server.c -o ${BUILDDIR}/server.o

${BUILDDIR}/request.o: request.c request.h 
	${CC} -c request.c -o ${BUILDDIR}/request.o

${BUILDDIR}/response.o: response.c response.h
	${CC} -c response.c -o ${BUILDDIR}/response.o

${CLIENT}: ${BUILDDIR}/client.o ${BUILDDIR}/client_setup.o
	${CC} ${BUILDDIR}/client.o ${BUILDDIR}/client_setup.o -o ${BINDIR}/${CLIENT}

${BUILDDIR}/client.o: client.c client.h
	${CC} -c client.c -o ${BUILDDIR}/client.o

${BUILDDIR}/client_setup.o: client_setup.c
	${CC} -c client_setup.c -o ${BUILDDIR}/client_setup.o

#${BUILDDIR}/${MODDIR}/%.o: ${MODDIR}/%.c ${MODDIR}/%.h
#	${CC} -c $< -o $@

ts:
	../bin/server -v

tc:
	../bin/client -v 127.0.0.1

tc1:
	../bin/client -v1 127.0.0.1

tc2:
	../bin/client -v2 127.0.0.1

submit:
	svn commit -m "submitted for grade"

clean:
	-rm -rf ${BUILDDIR} ${BINDIR}
