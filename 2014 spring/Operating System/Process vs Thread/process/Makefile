DEPEND_FILE = depend_file

CC = gcc

SERVER = server_p
SERVER_SRCS = server.c shared.c getnsec.c
SERVER_OBJS = $(SERVER_SRCS:%.c=%.o)

CLIENT = client_p
CLIENT_SRCS = client.c shared.c
CLIENT_OBJS = $(CLIENT_SRCS:%.c=%.o)

TARGET = $(SERVER) $(CLIENT)
SRCS = $(SERVER_SRCS) $(CLIENT_SRCS)
OBJS = $(SERVER_OBJS) $(CLIENT_OBJS)

LIBS = -lpthread

.SUFFIXES : .c .o

ifeq ($(DEBUG), 1)
DBG_FLAGS = -g -O0 -DDEBUG
else
DBG_FLAGS = -O2 -DNDEBUG
endif

all : $(TARGET)

$(SERVER) : $(SERVER_OBJS)
	@echo "linking $@ ..."
	$(CC) -o $@ $(SERVER_OBJS) $(LIBS)

$(CLIENT) : $(CLIENT_OBJS)
	@echo "linking $@ ..."
	$(CC) -o $@ $(CLIENT_OBJS) $(LIBS)

.c.o :
	@echo "Compiling $@ ..."
	$(CC) $(CFLAGS) $(DBG_FLAGS) -c $< -o $@

dep :
	@$(CC) -MM $(SRCS) > $(DEPEND_FILE)

clean :
	@rm -f $(OBJS) $(TARGET) $(DEPEND_FILE)

ifneq ($(MAKECMDGOALS), clean)
	ifneq ($(MAKECMDGOALS), dep)
		ifneq ($(SRCS),)
			-include $(DEPEND_FILE)
		endif
	endif
endif
