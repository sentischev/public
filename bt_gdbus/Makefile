
BUILD_DIR	:= ./build
OBJ_DIR		:= $(BUILD_DIR)/objects
BIN_DIR		:= $(BUILD_DIR)/bin
TARGET		:= $(BIN_DIR)/bt_gdbus

SRCS_C := $(shell find . -name "*.c")
SRCS_CPP := $(shell find . -name "*.cpp")
INC_DIRS := $(sort $(dir $(shell find . -name "*.h" -o -name "*.hpp")))
INCLUDES := $(addprefix -I, $(subst ./,,$(INC_DIRS)))

CC=gcc
CXX=g++
LD=g++

CFLAGS += -O2 -Wall -Wextra -MMD -MP $(INCLUDES)
CXXFLAGS += -O2 -Wall -Wextra -MMD -MP $(INCLUDES)
LIBS +=

CFLAGS += $(shell pkg-config --cflags glib-2.0 gio-2.0)
CXXFLAGS += $(shell pkg-config --cflags glib-2.0 gio-2.0)
LIBS += $(shell pkg-config --libs glib-2.0 gio-2.0)

debug: CFLAGS += -g -O0
debug: CXXFLAGS += -g -O0

# delete current directory path from filenames
REL_SRCS_C := $(subst ./,,$(SRCS_C))
REL_SRCS_CPP := $(subst ./,,$(SRCS_CPP))
# objects will be built in $OBJ_DIR directory
OBJS := $(addprefix $(OBJ_DIR)/, $(REL_SRCS_C:.c=.o) $(REL_SRCS_CPP:.cpp=.o))
DEPS := $(OBJS:.o=.o.d)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LIBS)

debug: $(TARGET)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MF $@.d -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MF $@.d -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean

