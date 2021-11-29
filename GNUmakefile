UNAME_S:=	$(shell uname -s)

ifeq ($(UNAME_S),Darwin)
SUFFIX:=	dylib
CFLAGS+=	-arch i386 -arch x86_64
else
SUFFIX:=	so
endif

CFLAGS+=	-fPIC
LIBS+=		-lsqlite3

all: sqlite3_distance.$(SUFFIX)
sqlite3_distance.$(SUFFIX): sqlite3_distance.c GNUmakefile
	$(CC) -shared $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f sqlite3_distance.$(SUFFIX)
