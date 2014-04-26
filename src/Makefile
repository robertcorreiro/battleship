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
EXEC = server

all: directories ${EXEC}

directories:
	mkdir -p ${BUILDDIR}/${MODDIR} ${BINDIR}

#../public/%: ../public/%.c
#	${CC} $< -o $@

${EXEC}: ${FRAMEWORK_OBJS} 
	${CC} ${FRAMEWORK_OBJS} -o ${BINDIR}/${EXEC}

${BUILDDIR}/server.o: ${FRAMEWORK}           \
	                    ${BUILDDIR}/request.o  \
	                    ${BUILDDIR}/response.o
	${CC} -c server.c -o ${BUILDDIR}/server.o

${BUILDDIR}/request.o: request.c request.h 
	${CC} -c request.c -o ${BUILDDIR}/request.o

${BUILDDIR}/response.o: response.c response.h
	${CC} -c response.c -o ${BUILDDIR}/response.o

#${BUILDDIR}/${MODDIR}/%.o: ${MODDIR}/%.c ${MODDIR}/%.h
#	${CC} -c $< -o $@

#test:

submit:
	svn commit -m "submitted for grade"

clean:
	-rm -rf ${BUILDDIR} ${BINDIR}
