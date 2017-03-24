#
#    HSGW Project Build Makefile

CPP	= mips-linux-c++
GCC	= mips-linux-gcc
RM	= rm -f
INC	= -I.
LIBS 	= -lc -lm -lpthread -ldl
CP	= /bin/cp

CFLAGS = -fPIC -O2 -fno-builtin
PUBLISH_FOLDER = /space/jobs/FDTI_Linux/git/deltadore_router/CppPrjoects/mpis

VERSION = 1
TARGET = CmdClient

all:$(TARGET)
OBJECTS = TcpClient.o\
	main.o

$(TARGET):$(OBJECTS)
	$(CPP) $(ARCH) -Wl,-soname,$(TARGET) $(CFLAGS)  -O2 -o $@ $(OBJECTS) $(LIBS)
	$(CP) $(TARGET) $(PUBLISH_FOLDER)/$(TARGET)

install:$(TARGET)
	sudo $(CP) $(TARGET) /usr/lib


.cpp.o:
	$(CPP) $(ARCH) $(INC) -Wall $(CFLAGS)  -c $< -o $@

.c.o:
	$(GCC) $(ARCH) $(INC) -Wall $(CFLAGS) -c $<  -o $@

clean:
	$(RM) $(OBJECTS) $(TARGET) $(L2OBJS) $(L3OBJS) *~
