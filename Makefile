CC = gcc
CXX = g++
#PYTHON_VERSION = 2.7

COMMON  += -DOPENCV
CFLAGS += -DOPENCV
#CXXFLAGS  += -DOPENCV

#LIBS += `pkg-config --libs opencv-3.2.0`
#COMMON  += `pkg-config --cflags opencv-3.2.0`

# link to opencv
LDFLAGS += `pkg-config --libs-only-L opencv | sed "s|^-L|-Wl,-rpath=|"`
LIBS += `pkg-config --libs opencv`
COMMON  += `pkg-config --cflags opencv`

# link to python3
LIBS += `pkg-config --libs python`
COMMON  += `pkg-config --cflags python`

#LIBS += -lpython$(PYTHON_VERSION)
#COMMON += -I/usr/include/python$(PYTHON_VERSION)

OBJDIR = ./obj/

#SRCS = 
#EXT = .cpp
#OBJ = $(SRCS:$(EXT)=.o)

OBJ = main.o image.o cJSON.o mjpeg_streaming.o socket_server.o
OBJS = $(addprefix $(OBJDIR), $(OBJ))
EXEC = opencv-draw-box

.PHONY: all mkdir_obj

all: mkdir_obj $(EXEC)

mkdir_obj:
	mkdir -p obj

$(EXEC): $(OBJS)
	$(CXX) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

$(OBJDIR)%.o: %.cpp
	$(CXX) $(COMMON) $(CFLAGS) -c $< -o $@ $(LDFLAGS) $(LIBS)

$(OBJDIR)%.o: %.c
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@ $(LDFLAGS) $(LIBS)

.PHONY: cleanall clean cleanobj cleanexe
cleanall: cleanobj cleanexe
clean: cleanall

cleanobj:
	rm -rf $(OBJDIR)

cleanexe:
	rm -f $(EXEC)
