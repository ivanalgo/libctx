CC = gcc
#CFLAGS = -Wall -Werror -g -DDEBUG
CFLAGS = -Wall -Werror -g -Iinclude
LDFLAGS = -ldl
APP_CFLAGS = -Wall -Werror -g
APP_LDFLAGS = -lpthread

SHOBJS=core/task.o core/sched.o core/makecontext.o posix/pthread.o core/lock.o posix/mutex.o
EXAMPLES=examples/thread examples/mutex
LIB=liblwt.so

all: $(LIB) $(EXAMPLES)

$(LIB): $(SHOBJS) core/context.o
	$(CC) $(LDFLAGS) -shared -o $(LIB) $(SHOBJS) core/context.o

$(SHOBJS):%.o:%.c
	$(CC) $(CFLAGS) -c -fPIC -shared -o $@ $<

core/context.o: core/context.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXAMPLES):%:%.c
	$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) -o $@ $<

clean:
	rm -rf $(SHOBJS) $(LIB) $(EXAMPLES) core/context.o
