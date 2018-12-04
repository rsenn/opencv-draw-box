CC = gcc
CXX = g++
#PYTHON_VERSION = 2.7

COMMON  += -DOPENCV
CFLAGS += -DOPENCV
#CXXFLAGS  += -DOPENCV

#LDFLAGS += `pkg-config --libs opencv-3.2.0`
#COMMON  += `pkg-config --cflags opencv-3.2.0`

# link to opencv
LDFLAGS += `pkg-config --libs opencv`
COMMON  += `pkg-config --cflags opencv`

# link to python3
LDFLAGS += `pkg-config --libs python`
COMMON  += `pkg-config --cflags python`

#LDFLAGS += -lpython$(PYTHON_VERSION)
#COMMON += -I/usr/include/python$(PYTHON_VERSION)

LIBS = 
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
	$(CXX) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.cpp
	$(CXX) $(COMMON) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.c
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

cleanall:clean cleanexe

cleanobj:
	rm -rf $(OBJDIR)

cleanexe:
	rm -f $(EXEC)