SRCDIR = src
STRUCTURE = $(shell cd $(SRCDIR) && find . -type d)

INCDIR = include
INCDIRNAME = homecontroller

CXX = g++
CXXFLAGS ?= -g -std=c++17 -fPIC -I$(INCDIR)

BINARYDIR = bin
OBJECTDIR = $(BINARYDIR)/obj

TARGETNAME = libhomecontroller.so
TARGET = $(BINARYDIR)/$(TARGETNAME)

LIBS += -lssl
LIBS += -lcrypto

LIBINSTALLDIR = /usr/local/lib
INCINSTALLDIR = /usr/local/include

# root
_OBJECTS += test.o
_HEADERS += test.h

# api
_OBJECTS += api/device.o
_HEADERS += api/device.h

# api/state
_OBJECTS += api/device_data/rgb_lights.o
_HEADERS += api/device_data/rgb_lights.h

# socket.io
_OBJECTS += socket.io/client.o
_HEADERS += socket.io/client.h

# util
_OBJECTS += util/logger.o
_HEADERS += util/logger.h

_OBJECTS += util/string.o
_HEADERS += util/string.h

OBJECTS = $(patsubst %,$(OBJECTDIR)/%,$(_OBJECTS))
HEADERS = $(patsubst %,$(INCDIR)/$(INCDIRNAME)/%,$(_HEADERS))

$(OBJECTDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(OBJECTDIR)
	$(CXX) $(CPPFLAGS) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJECTS)
	$(CXX) -shared -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OBJECTDIR):
	mkdir -p $(OBJECTDIR)
	mkdir -p $(addprefix $(OBJECTDIR)/,$(STRUCTURE))

install: $(TARGET)
	cp -r $(INCDIR)/$(INCDIRNAME) $(INCINSTALLDIR)
	cp $(TARGET) $(LIBINSTALLDIR)/$(TARGETNAME)
	ldconfig

uninstall:
	rm -r $(INCINSTALLDIR)/$(INCDIRNAME)
	rm $(LIBINSTALLDIR)/$(TARGETNAME)

clean:
	rm -rf bin

.PHONY: clean install uninstall