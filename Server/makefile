all: server

server: FileManager.o CommunicationProtocol.o ConnectionNode.o Server.o ProtocolParser.o DataBaseManager.o Token.o main.o  
	gcc FileManager.o CommunicationProtocol.o ConnectionNode.o ProtocolParser.o Server.o DataBaseManager.o Token.o main.o -o server `mysql_config --cflags --libs`

FileManager.o: FileManager.h FileManager.c
	gcc -c -g -Wall FileManager.c

Token.o: Token.h Token.c
	gcc -c -g -Wall Token.c

DataBaseManager.o: DataBaseManager.h DataBaseManager.c
	gcc -c -g -Wall DataBaseManager.c

ProtocolParser.o: ProtocolParser.c ProtocolParser.h
	gcc -c -g -Wall ProtocolParser.c

CommunicationProtocol.o: CommunicationProtocol.c CommunicationProtocol.h
	gcc -c -g -Wall CommunicationProtocol.c

ConnectionNode.o: ConnectionNode.c ConnectionNode.h
	gcc -c -g -Wall ConnectionNode.c

Server.o: Server.c Server.h
	gcc -c -g -Wall Server.c
 
main.o: main.c
	gcc -c -g -Wall main.c `mysql_config --cflags --libs`

clean:
	rm server *.o
