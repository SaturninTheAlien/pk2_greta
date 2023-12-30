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
CXXFLAGS += -DPK2_PORTABLE -DPK2_USE_JAVA -DPK2_VERSION=\"$(PK2_VERSION)\"

# Commit hash
CXXFLAGS += -DCOMMIT_HASH='"$(shell git rev-parse --short HEAD)"'


UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
# Support MAC OS
	JAVA_INCLUDE= $(shell /usr/libexec/java_home)/include
	CXXFLAGS += -I$(JAVA_INCLUDE)  -I$(JAVA_INCLUDE)/darwin
	CXXFLAGS += -I/opt/homebrew/include

	COMPILE_COMMAND = $(CXX) $(CXXFLAGS)
else
	JAVA_INCLUDE = $(shell dirname $$(dirname $$(readlink -f $$(which java))))/include
	CXXFLAGS += -I$(JAVA_INCLUDE)  -I$(JAVA_INCLUDE)/linux

	COMPILE_COMMAND = $(CXX)
endif




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

all: pk2 launcher

pk2: $(PK2_BIN)

launcher: $(PK2_BIN_LAUNCHER)

###########################
$(PK2_BIN): $(PK2_OBJ)
	@echo -Linking Pekka Kana 2
	@mkdir -p $(dir $@) >/dev/null
	@$(CXX) $^ $(LDFLAGS) -o $@

###########################
$(PK2_BIN_LAUNCHER): $(PK2_BIN) $(LAUNCHER_SRC)
	@echo -Compiling launcher.cpp
	@mkdir -p $(dir $@) >/dev/null
	@$(CXX) --std=c++17 -I$(SRC_DIR) $(LAUNCHER_SRC) $(PK2_BIN) -o $@

###########################
-include $(DEPENDENCIES)

$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp
	@echo -Compiling $<
	@mkdir -p $(dir $@) >/dev/null
	@$(COMPILE_COMMAND) $(CXXFLAGS) -I$(SRC_DIR) -o $@ -c $<
	@$(COMPILE_COMMAND) -MM -MT $@ -I$(SRC_DIR) $< > $(BUILD_DIR)$*.d
###########################



clean:
	@rm -rf $(BIN_DIR)
	@rm -rf $(BUILD_DIR)

version_test:
	echo $(PK2_VERSION)

.PHONY: pk2 clean all version_test
