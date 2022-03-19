all: Client

Client: ManageFileSharing.o ManageFileUploading.o MessageHandle.o ProtocolParser.o CommunicationProtocol.o Token.o Client.o main.o
	gcc ManageFileSharing.o ManageFileUploading.o MessageHandle.o ProtocolParser.o CommunicationProtocol.o Token.o Client.o main.o -o client

Token.o: Token.h Token.c
	gcc -c -g -Wall Token.c

ManageFileSharing.o: ManageFileSharing.h ManageFileSharing.c
	gcc -c -g -D_BSD_SOURCE -Wall ManageFileSharing.c

ManageFileUploading.o: ManageFileUploading.h ManageFileUploading.c
	gcc -c -g -D_BSD_SOURCE -Wall ManageFileUploading.c

MessageHandle.o: MessageHandle.h MessageHandle.c
	gcc -c -g -Wall MessageHandle.c	

ProtocolParser.o: ProtocolParser.h ProtocolParser.c
	gcc -c -g -Wall ProtocolParser.c

CommunicationProtocol.o: CommunicationProtocol.c CommunicationProtocol.h
	gcc -c -g -Wall CommunicationProtocol.c

Client.o: Client.c Client.h
	gcc -c -g -Wall Client.c

main.o: main.c
	gcc -c -g -Wall main.c 

clean:
	rm Client *.o

