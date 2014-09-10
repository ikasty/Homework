#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include "shared.h"
#include "client.h"

char clientName[100];

int main(int argc, char *argv[]) {
    int socket;
    // 서버의 주소와 포트 번호는 입력받지 않고 기본값만 사용하도록 합니다.
    char address[] = "localhost";
    int portno = 10000;

    // shell 모드인지를 판별합니다.
    if (argc == 2 && !strcmp(argv[1], "shell") ) {
        // shell 모드로 소켓을 엽니다.
        socket = init(portno, address, true);
        FDEBUG(("shell mode enabled.\n"));
        doShellMode(socket);
    } else {
        // 일반 모드로 소켓을 엽니다.
        socket = init(portno, address, false);
        doNormalMode(socket);
    }

    return 0;
}

int init(int portno, char* hostname, bool isShell) {
    int sock_cli, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char head[HEAD_SIZE], value[VALUE_SIZE];

    Notice("Trying to make socket\n");

    // 소켓 통신을 위한 설정을 진행합니다.
    sock_cli = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_cli < 0) {
        perror("ERROR opening socket");
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        perror("ERROR: no such host");
        exit(0);
    }

    memset( (char*) &serv_addr, 0, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    bcopy( (char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length );
    serv_addr.sin_port = htons(portno);

    Notice("Trying to connect server");
    printf(" %s...", hostname);
    fflush(stdout);
    if ( connect(sock_cli, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0 ) {
        perror("ERROR connecting");
        exit(1);
    }

    // 서버 접속 완료
    printf("Connected. Initiating...");

    // shell 모드인지 아닌지를 서버에 알립니다.
    if (isShell) writeMsg(sock_cli, MODE, MODE_SHELL);
    else writeMsg(sock_cli, MODE, MODE_CLIENT);

    // 서버에서의 내 이름을 받아옵니다.
    readMsg(sock_cli, head, value);
    if ( !strcmp(head, SET_NAME) ) {
        strcpy(clientName, value);
        writeMsg(sock_cli, SET_NAME, "ok");
    } else {
        printf("Error during initiation\n");
        exit(1);
    }

    // 처리가 완료되었는지를 다시 한 번 확인합니다.
    readMsg(sock_cli, head, value);
    if ( !strcmp(head, ACCEPT) ) {
        printf("done.\n");
    } else {
        printf("Error during initiation\n");
        exit(1);
    }

    return sock_cli;
}

void doNormalMode(int socket) {
    char head[HEAD_SIZE], value[VALUE_SIZE];
    char fileBuffer[FILE_BUFFER_SIZE + 1];
    char fileChunk[FILE_CHUNK_SIZE];
    char filename[256];
    long filesize;
    int fd;

    // 일반 모드로 작동합니다...
    while (true) {
        // 서버의 메시지를 받아 작동합니다.
        readMsg(socket, head, value);

        // 파일 삭제
        if ( !strcmp(head, DEL) ) {
            FDEBUG(("get del message from server\n"));
            remove(value);
        } else
        // 파일 추가
        if ( !strcmp(head, ADD) ) {
            FDEBUG(("get add message from server\n"));

            // 파일 이름을 저장하고, 해당 이름으로 파일을 생성합니다.
            strcpy(filename, value);
            fd = open(filename, O_WRONLY | O_CREAT, 0644);

            // 파일 구분자를 받아옵니다.
            readMsg(socket, head, fileChunk);

            while(true) {
                // 데이터를 받아서 그대로 파일에 기록합니다.
                readData(socket, fileBuffer);
                // 단, 구분자만 보내는 경우는 파일 전송이 끝났다는 신호입니다.
                if ( !strcmp(fileBuffer, fileChunk) ) break;

                writeData(fd, fileBuffer);
            }

            // 작성하고 있는 파일을 닫습니다.
            close(fd);
            // EOF 메시지를 확인합니다.
            readMsg(socket, head, value);
        }
    }
}

void doShellMode(int socket) {
    char head[HEAD_SIZE], value[VALUE_SIZE];
    char buffer[VALUE_SIZE];
    char fileBuffer[FILE_BUFFER_SIZE + 1];

    int fd;

    // shell 모드로 작동합니다.
    while (true) {
        printf("SHELL #> ");
        fgets(buffer, VALUE_SIZE, stdin);

        // 기본 인터페이스
        if ( !strcmp(buffer, "usage") || !strcmp(buffer, "help") ) {
            printf("usage: (add|delete) filename\n");
            continue;
        } else
        if ( buffer[0] == '\n' ) {
            continue;
        }

        // 버퍼로부터 메시지를 분리해냅니다.
        sscanf(buffer, "%s %s", head, value);

        // 파일 추가
        if ( !strcmp(head, "add") ) {
            fd = open(value, O_RDONLY);
            // 없는 파일일 경우의 예외 처리
            if (fd == -1) {
                Notice("Cannot find ");
                printf("%s\n", value);
                continue;
            }

            // 서버에 파일을 전송할 것임을 알립니다.
            writeMsg(socket, ADD, value);
            readMsg(socket, head, value);

            // 서버에서 거부하는 경우 전송을 취소합니다.
            if ( strcmp(head, ACCEPT) != 0 ) {
                Notice("Denied by server.\n");
                continue;
            }

            // 서버에 파일 구분자를 보냅니다.
            writeMsg(socket, CHUNK, CHUNK_MSG);
            readMsg(socket, head, value);

            // 파일을 읽어서 그대로 전송합니다. 파일에서 읽을 때는 readData를 쓰지 않습니다.
            memset(fileBuffer, 0, sizeof(fileBuffer));
            while ( 0 < read(fd, fileBuffer, FILE_BUFFER_SIZE - 1) ) {
                writeData(socket, fileBuffer);
                readMsg(socket, head, value);
                memset(fileBuffer, 0, sizeof(fileBuffer));
            }

            // 전송이 완료되었으므로, 파일 구분자를 보냅니다.
            writeData(socket, CHUNK_MSG);
            close(fd);
            // eof 메시지를 확인합니다.
            readMsg(socket, head, value);

        } else
        // 파일을 삭제하는 경우
        if ( !strcmp(head, "delete") ) {
            // 서버에 파일을 삭제할 것을 요청하고, 로컬에서도 파일을 지웁니다.
            writeMsg(socket, DEL, value);
            remove (value);
        }
        else {
            printf("Unknown command\n");
            printf("usage: (add|delete) filename\n");
            continue;
        }
    }
}
