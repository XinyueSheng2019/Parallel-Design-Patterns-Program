MF= Makefile
CC=	mpicc -cc=icc

CFLAGS=	-lm -I${DIR_INC}
LFLAGS= $(CFLAGS)


DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.c)
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = simulation

BIN_TARGET = ${DIR_BIN}/${TARGET}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ)  -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) -c  $< -o $@ $(CFLAGS)

.PHONY:clean
clean:
	find ${DIR_OBJ} -name *.o 
	rm ${BIN_TARGET}

