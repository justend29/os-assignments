BUILD_DIR = build
ERR_DIR = error
CMDS_DIR = $(SHELL_DIR)/commands
SHELL_DIR = shell
FAT_DIR = fat32
DISK_IMG = diskimage

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -g
LDLIBS =
ERR_OBJS = $(BUILD_DIR)/error.o
CMDS_OBJS = $(BUILD_DIR)/cd.o $(BUILD_DIR)/dir.o $(BUILD_DIR)/info.o $(BUILD_DIR)/get.o $(BUILD_DIR)/commands.o
SHELL_OBJS = $(BUILD_DIR)/shell.o
FAT_OBJS = $(BUILD_DIR)/fat32.o $(BUILD_DIR)/fat.o
OBJS = $(CMDS_OBJS) $(SHELL_OBJS) $(ERR_OBJS) $(FAT_OBJS) $(BUILD_DIR)/main.o
EXE = $(BUILD_DIR)/fat32


all: $(EXE)

debug: CFLAGS += -g
debug: $(EXE)

run: $(EXE) $(DISK_IMG)
	$(EXE) $(DISK_IMG)

$(EXE): mkbuild $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXE) $(LDLIBS)

$(BUILD_DIR)/error.o: $(ERR_DIR)/error.c $(ERR_DIR)/error.h
	$(CC) $(CFLAGS) -c $(ERR_DIR)/error.c -o $(BUILD_DIR)/error.o

$(BUILD_DIR)/commands.o: $(CMDS_DIR)/commands.h $(CMDS_DIR)/commands.c
	$(CC) $(CFLAGS) -c $(CMDS_DIR)/commands.c -o $(BUILD_DIR)/commands.o

$(BUILD_DIR)/cd.o: $(CMDS_DIR)/cd.c $(CMDS_DIR)/commands.h $(ERR_DIR)/error.h
	$(CC) $(CFLAGS) -c $(CMDS_DIR)/cd.c -o $(BUILD_DIR)/cd.o

$(BUILD_DIR)/dir.o: $(CMDS_DIR)/dir.c $(CMDS_DIR)/commands.h
	$(CC) $(CFLAGS) -c $(CMDS_DIR)/dir.c -o $(BUILD_DIR)/dir.o

$(BUILD_DIR)/get.o: $(CMDS_DIR)/get.c $(CMDS_DIR)/commands.h
	$(CC) $(CFLAGS) -c $(CMDS_DIR)/get.c -o $(BUILD_DIR)/get.o

$(BUILD_DIR)/info.o: $(CMDS_DIR)/info.c $(CMDS_DIR)/commands.h
	$(CC) $(CFLAGS) -c $(CMDS_DIR)/info.c -o $(BUILD_DIR)/info.o

$(BUILD_DIR)/fat.o: $(FAT_DIR)/fat.h $(FAT_DIR)/fat.c
	$(CC) $(CFLAGS) -c $(FAT_DIR)/fat.c -o $(BUILD_DIR)/fat.o

$(BUILD_DIR)/fat32.o: $(FAT_DIR)/fat32.h $(FAT_DIR)/fat32.c $(FAT_DIR)/boot_sector.h $(ERR_DIR)/error.h $(FAT_DIR)/fsinfo.h $(FAT_DIR)/fat.h $(FAT_DIR)/fat32.h
	$(CC) $(CFLAGS) -c $(FAT_DIR)/fat32.c -o $(BUILD_DIR)/fat32.o

$(BUILD_DIR)/shell.o: $(CMDS_OBJS) $(CMDS_DIR)/commands.h $(SHELL_DIR)/shell.c $(SHELL_DIR)/shell.h $(FAT_DIR)/fat32.h
	$(CC) $(CFLAGS) -c $(SHELL_DIR)/shell.c -o $(BUILD_DIR)/shell.o

$(BUILD_DIR)/main.o: main.c $(SHELL_DIR)/shell.h
	$(CC) $(CFLAGS) -c main.c -o $(BUILD_DIR)/main.o

.ONESHELL:
mkbuild:
	if [ ! -d $(BUILD_DIR) ]; then
		mkdir $(BUILD_DIR)
	fi

.ONESHELL:
clean:
	if [ -d $(BUILD_DIR) ]; then
		rm -rf $(BUILD_DIR)
	fi
