CC = g++
TARGET = chat
CFLAGS = -c -Wall

$(TARGET): main.o
	$(CC) -o $(TARGET) main.o Chat.o User.o Message.o sha1.o

main.o: main.cpp Chat.o
	$(CC) $(CFLAGS) main.cpp

Chat.o: Chat.cpp User.o sha1.o Message.o
	$(CC) $(CFLAGS) Chat.cpp

User.o: User.cpp
	$(CC) $(CFLAGS) User.cpp

sha1.o: sha1.cpp
	$(CC) $(CFLAGS) sha1.cpp

Message.o: Message.cpp
	$(CC) $(CFLAGS) Message.cpp

clean:
	rm -rf *.o
