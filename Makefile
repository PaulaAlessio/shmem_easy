CC 	:= gcc
CCFLAGS := -Wall -Wextra


all: 
	$(CC) $(CCFLAGS) client.c -I. -o client 
	$(CC) $(CCFLAGS) device.c -I. -o device 
	$(CC) $(CCFLAGS) run_device.c -lm -I. -o run_device
