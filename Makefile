CC = gcc
CFLAGS = -Wall -Werror -g

SHOBJS=task.o sched.o
EXAMPLES=main
LIB=liblwt.so

all: $(LIB) $(EXAMPLES)

$(LIB): $(SHOBJS)
	$(CC) $(LDFLAGS) -shared -o $(LIB) $(SHOBJS)

$(SHOBJS):%.o:%.c
	$(CC) $(CFLAGS) -c -fPIC -shared -o $@ $<

$(EXAMPLES):%:%.c
	$(CC) $(CFLAGS) $(LDFALGS) -o $@ $< -llwt -Wl,--rpath=$(PWD) -L$(PWD)

clean:
	rm -rf $(SHOBJS) $(LIB) $(EXAMPLES)
