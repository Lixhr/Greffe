CXX      := g++
CC       := gcc
AR       := ar

SRC_DIR  := src
INC_DIR  := include
BUILD    := build
OBJ_DIR  := $(BUILD)/obj

TARGET   := $(BUILD)/btrace

FRIDA_DIR     := third_party/frida-gum
FRIDA_LIB     := $(FRIDA_DIR)/build/gum/libfrida-gum-1.0.a
FRIDA_LDFLAGS := -lresolv -lm -ldl -pthread

# Sources
SRCS := $(shell find $(SRC_DIR) -type f \( -name '*.cpp' -o -name '*.cc' \))
OBJS := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%.o,$(SRCS))

STD      := -std=c++17
WARNS    := -Wall -Wextra -Werror

FRIDA_DEPS     := $(FRIDA_DIR)/deps/sdk-linux-x86_64
FRIDA_INCS     := -I$(FRIDA_DIR) \
                  -I$(FRIDA_DIR)/build \
                  -I$(FRIDA_DEPS)/include \
                  -I$(FRIDA_DEPS)/include/glib-2.0 \
                  -I$(FRIDA_DEPS)/include/capstone \
                  -I$(FRIDA_DEPS)/lib/glib-2.0/include

INCS     := -I$(INC_DIR) -I$(INC_DIR)/CLI $(FRIDA_INCS)

CXXFLAGS := $(STD) $(WARNS) $(INCS)
LDFLAGS  := $(PKG_LIBS) -lreadline -lzmq $(FRIDA_LIB) $(FRIDA_LDFLAGS)

all: $(TARGET)

$(FRIDA_LIB):
	@$(MAKE) -C $(FRIDA_DIR)

$(TARGET): $(OBJS) $(FRIDA_LIB) | dirs
	@echo "[LD]  $@"
	@$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp | dirs
	@mkdir -p $(dir $@)
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)

dirs:
	@mkdir -p $(OBJ_DIR) $(BUILD)

clean:
	@echo "[RM]  $(OBJ_DIR)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "[RM]  $(BUILD)"
	@rm -rf $(BUILD)
	@$(MAKE) -C $(FRIDA_DIR) fclean

re: fclean all

.PHONY: all clean fclean re dirs