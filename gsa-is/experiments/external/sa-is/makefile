CC = g++

# For program run

CC_OPTION = -fomit-frame-pointer -W -Wall -Winline -DNDBUG -O3

# For debug
# CC_OPTION = -g -fomit-frame-pointer -W -Wall -Winline

ALL_OPS = tryall core Makeours Clear
ALL:$(ALL_OPS)

core:
	$(CC) -c sais.cpp $(CC_OPTION)
tryall:
	$(CC) -c main.cpp $(CC_OPTION)
Makeours:
	$(CC) -o sais sais.o main.o  $(CC_OPTION)
Clear:
	rm *.o
     
      
