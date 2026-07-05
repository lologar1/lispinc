TARGET := lisp

SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj

USFLIB2_DIR := usflib2
USFLIB2_INC_DIR := $(USFLIB2_DIR)/include
USFLIB2_OBJ_DIR := $(USFLIB2_DIR)/obj

CC := gcc
CFLAGS := -Wall -Wextra -Wunused-macros -Wcast-align=strict -Wduplicated-branches -Wduplicated-cond \
		  -Wformat-signedness -Wjump-misses-init -Wlogical-op -Wsign-conversion -Wcast-qual \
		  -std=gnu23 -O3
INCLUDES := -I$(INC_DIR) -I$(USFLIB2_INC_DIR)
LINKS := -lc -lm -ledit

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
USFLIB2_OBJS := $(wildcard $(USFLIB2_OBJ_DIR)/*.o)
ALL_OBJS := $(OBJS) $(USFLIB2_OBJS)

all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LINKS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)

r:
	make clean
	make

.PHONY: all clean r
