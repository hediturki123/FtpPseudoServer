FTPCLIENT = ftpclient
FTPSERVEUR = ftpserveur

INCLUDE = csapp.h

SRCDIR = ../src
OBJSDIR = ../objs
HEADERDIR = ../header
SERVEURDIR = SERVEUR

CC = gcc

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(SRCS:$(SRCDIR)/%.c=$(OBJSDIR)/%.o)
OBJS_SERV=$(patsubst $(OBJSDIR)/$(FTPCLIENT).o,,$(OBJS))
OBJS_CLIENT=$(patsubst $(OBJSDIR)/$(FTPSERVEUR).o,,$(OBJS))

CFLAGS = -Wall -Werror -I $(HEADERDIR)
ifdef DEBUG
CFLAGS+= -g -DDEBUG=$(DEBUG)
endif
LDFLAGS =

LIBS += -lpthread

PROG = $(SERVEURDIR)/$(FTPSERVEUR) $(FTPCLIENT)

all: $(PROG)

$(OBJSDIR)/%.o:$(SRCDIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVEURDIR)/$(FTPSERVEUR):$(OBJS_SERV)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(FTPCLIENT):$(OBJS_CLIENT)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)



clean:
	rm -f $(PROGS) $(OBJSDIR)/*.o
