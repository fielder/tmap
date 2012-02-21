SDLCFLAGS = -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
SDLLDFLAGS = -lSDL -lpthread

CC = gcc
CFLAGS = -Wall -O2 $(SDLCFLAGS)
LDFLAGS = -lm $(SDLLDFLAGS)
OBJDIR = obj
TARGET = $(OBJDIR)/tmap

OBJS =	$(OBJDIR)/vec.o \
	$(OBJDIR)/tmap.o \
	$(OBJDIR)/render.o \
	$(OBJDIR)/geom.o \
	$(OBJDIR)/view.o \
	$(OBJDIR)/rast.o \
	$(OBJDIR)/r_tex.o \
	$(OBJDIR)/pcx.o

all: $(TARGET)

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

########################################################################

$(OBJDIR)/vec.o: vec.c
	$(CC) -c $(CFLAGS) $? -o $@
$(OBJDIR)/tmap.o: tmap.c
	$(CC) -c $(CFLAGS) $? -o $@
$(OBJDIR)/render.o: render.c
	$(CC) -c $(CFLAGS) $? -o $@
$(OBJDIR)/geom.o: geom.c
	$(CC) -c $(CFLAGS) $? -o $@
$(OBJDIR)/view.o: view.c
	$(CC) -c $(CFLAGS) $? -o $@
$(OBJDIR)/rast.o: rast.c
	$(CC) -c $(CFLAGS) $? -o $@
$(OBJDIR)/r_tex.o: r_tex.c
	$(CC) -c $(CFLAGS) $? -o $@
$(OBJDIR)/pcx.o: pcx.c
	$(CC) -c $(CFLAGS) $? -o $@
