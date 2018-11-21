CC = gcc
CXX = g++
CFLAGS = 
LDFLAGS = `pkg-config --libs opencv` `pkg-config --cflags opencv` 
OBJDIR = ./obj/

#SRCS = 
#EXT = .cpp
#OBJ = $(SRCS:$(EXT)=.o)

OBJ = main.o image.o cJSON.o mjpeg_streaming.o socket_server.o
OBJS = $(addprefix $(OBJDIR), $(OBJ))
EXEC = a.out

all: mkdir_obj $(EXEC)

mkdir_obj:
	mkdir -p obj

$(EXEC): $(OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

cleanall:clean cleanexe

cleanobj:
	rm -f $(OBJDIR)

cleanexe:
	rm -f $(EXEC)