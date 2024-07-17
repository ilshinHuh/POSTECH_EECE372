CC = gcc
TARGET = exec

$(TARGET) : main.c arm_perf.h
	$(CC) -mfpu=neon -o $(TARGET) main.c -O1

