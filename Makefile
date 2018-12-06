CC = gcc
CFLAGS = -Wall -Werror -g -O2

SHOBJS=task.o sched.o makecontext.o
EXAMPLES=main
LIB=liblwt.so

all: $(LIB) $(EXAMPLES)

$(LIB): $(SHOBJS) context.o
	$(CC) $(LDFLAGS) -shared -o $(LIB) $(SHOBJS) context.o

$(SHOBJS):%.o:%.c
	$(CC) $(CFLAGS) -c -fPIC -shared -o $@ $<

context.o:context.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXAMPLES):%:%.c
	$(CC) $(CFLAGS) $(LDFALGS) -o $@ $< -llwt -Wl,--rpath=$(PWD) -L$(PWD)

clean:
	rm -rf $(SHOBJS) $(LIB) $(EXAMPLES) context.o
