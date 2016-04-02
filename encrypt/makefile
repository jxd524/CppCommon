# MAKEFILE for GCC
CFLAGS = -O2 -Wall -c -I./
CC=gcc
LD=ld
AR=ar

#x86 optimizations
CFLAGS += -mpentium -fomit-frame-pointer -funroll-loops

default:libcrypt.a

libcrypt.a: base64.o rsa_sys.o rsa.o yarrow.o ctr.o cbc.o hash.o tiger.o sha1.o md5.o sha256.o serpent.o safer+.o rc6.o rc5.o blowfish.o crypt.o mpi.o prime.o
	ar rs libcrypt.a base64.o rsa_sys.o rsa.o yarrow.o ctr.o cbc.o hash.o tiger.o sha1.o md5.o sha256.o serpent.o safer+.o rc6.o rc5.o blowfish.o crypt.o mpi.o prime.o

test.exe: libcrypt.a test.o
	gcc test.o libcrypt.a -o test.exe

clean:
	rm -f *.a *.o *.exe *.log *.aux *.dvi *.toc *.idx *.ilg
