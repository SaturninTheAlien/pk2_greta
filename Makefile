# Pekka Kana 2 by Janne Kivilahti from Piste Gamez (2003-2007)
# https://pistegamez.net/game_pk2.html
#
# Makefile command:
# "make" - Creates Pekka Kana 2 binary
# "make clean" - Removes all objects, executables and dependencies

# Compiler:
CXX = c++

# Optimization:
CXXFLAGS += -O3
#CXXFLAGS += -g

# Warnings:
CXXFLAGS += -Wall

# Standart:
CXXFLAGS += --std=c++17 -fPIC

# Compiling to dll
LDFLAGS += -shared

# SDL2:
CXXFLAGS += $(shell pkg-config sdl2 --cflags)
LDFLAGS += $(shell pkg-config sdl2 --libs) -lSDL2_mixer -lSDL2_image

# LibZip (read episodes on zip files):
CXXFLAGS += -DPK2_USE_ZIP $(shell pkg-config libzip --cflags)
LDFLAGS += $(shell pkg-config libzip --libs)

# Version string
PK2_VERSION = $(shell git log -1 --pretty=format:"%s" | grep -o 'v[0-9]\+\.[0-9]\+')
ifeq ($(PK2_VERSION),)
	PK2_VERSION = "Unknown_version"
endif


# Portable (data is stored with resorces):
CXXFLAGS += -DPK2_PORTABLE -DPK2_VERSION=\"$(PK2_VERSION)\"

# Commit hash
CXXFLAGS += -DCOMMIT_HASH='"$(shell git rev-parse --short HEAD)"'

# Detecting operating system (both MacOS and Linux supported)
UNAME_S = $(shell uname -s)

# Add java (optional), PekaEDS support
JAVA = $(shell command -v java 2> /dev/null)

ifneq ($(strip $(JAVA)),)
	ifeq ($(UNAME_S),Darwin)
		JAVA_INCLUDE= $(shell /usr/libexec/java_home)/include
		JAVA_INCLUDE_SYSTEM = $(JAVA_INCLUDE)/darwin
	else
		JAVA_INCLUDE = $(shell dirname $$(dirname $$(readlink -f $$(which java))))/include
		JAVA_INCLUDE_SYSTEM = $(JAVA_INCLUDE)/linux
	endif

	ifneq ($(wildcard $(JAVA_INCLUDE)/jni.h),)
		CXXFLAGS += -DPK2_USE_JAVA -I$(JAVA_INCLUDE) -I$(JAVA_INCLUDE_SYSTEM)
	endif
endif

#Compile command CXX and CXXFLAGS
COMPILE_COMMAND = $(CXX) $(CXXFLAGS)

# Directories:
SRC_DIR = src/
BIN_DIR = bin/
BUILD_DIR = build/

# Source files:
PK2_SRC  = *.cpp */*.cpp */*/*.cpp
PK2_SRC := $(addprefix $(SRC_DIR), $(PK2_SRC))
PK2_SRC := $(wildcard $(PK2_SRC))

# Object files:
PK2_OBJ := $(basename $(PK2_SRC))
PK2_OBJ := $(subst $(SRC_DIR), ,$(PK2_OBJ))
PK2_OBJ := $(addsuffix .o, $(PK2_OBJ))
PK2_OBJ := $(addprefix $(BUILD_DIR), $(PK2_OBJ))

# Dependency files:
DEPENDENCIES := $(PK2_OBJ)
DEPENDENCIES := $(basename $(DEPENDENCIES))
DEPENDENCIES := $(addsuffix .d, $(DEPENDENCIES))

# Binary output:
PK2_BIN = $(BIN_DIR)pk2.so
PK2_BIN_LAUNCHER = $(BIN_DIR)pekka-kana-2

LAUNCHER_SRC = launcher/launcher.cpp
LAUNCHER_OBJ = launcher/launcher.o

all: pk2 launcher

pk2: $(PK2_BIN)

launcher: $(PK2_BIN_LAUNCHER)

###########################
$(PK2_BIN): $(PK2_OBJ)
	@echo -Linking Pekka Kana 2
	@mkdir -p $(dir $@) >/dev/null
	@$(CXX) $^ $(LDFLAGS) -o $@

###########################
$(LAUNCHER_OBJ): $(LAUNCHER_SRC)
	@echo -Compiling $<
	@$(CXX) --std=c++17 -I$(SRC_DIR) $(LAUNCHER_SRC) -c -o $@

$(PK2_BIN_LAUNCHER): $(LAUNCHER_OBJ) $(PK2_BIN)
	@echo -Linking launcher
	@mkdir -p $(dir $@) >/dev/null
	@$(CXX) $(LAUNCHER_OBJ) $(PK2_BIN) -o $@

###########################
-include $(DEPENDENCIES)

$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp
	@echo -Compiling $<
	@mkdir -p $(dir $@) >/dev/null
	@$(COMPILE_COMMAND) -I$(SRC_DIR) -o $@ -c $<
	@$(COMPILE_COMMAND) -MM -MT $@ -I$(SRC_DIR) $< > $(BUILD_DIR)$*.d
###########################



clean:
	@rm -rf $(LAUNCHER_OBJ)
	@rm -rf $(BIN_DIR)
	@rm -rf $(BUILD_DIR)

test:
	@echo $(CXXFLAGS)

.PHONY: pk2 launcher clean all test
