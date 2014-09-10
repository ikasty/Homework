#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "shared.h"
#include "server.h"
#include "getnsec.h"

Client* front;
int clientCount;
Client* shellClient;
Client* currentClient;

int serverPipe[2];

NSecond elapseTime;

int main(int argc, char* argv[]) {
    int serverSocket;

    // 초기화
    front = 0;
    clientCount = 0;
    shellClient = 0;
    currentClient = 0;
    elapseTime = 0;

    Notice("Server process mode ON\n");

    if ( -1 == pipe(serverPipe) ) {
        perror("ERROR open pipe");
        exit(1);
    }
    Notice("Server pipe created\n");

    Notice("Trying to open socket...");

    // 들어오는 소켓 설정
    serverSocket = setSocket(INPORT);

    if (serverSocket < 0) {
        Notice("Error opening socket\n");
        exit(1);
    }
    printf("done.\n");

    Notice("Now waiting clients...\n");
    while (1) {
        // 소켓으로부터 클라이언트를 받아, 클라이언트 관리 리스트에 추가하고, 새 프로세스를 만듭니다.
        split( addClient( getClient(serverSocket) ) );

        // 클라이언트 개수가 MAX_CLIENT(=100)개라면 걸린 시간을 출력합니다.
        if (clientCount == MAX_CLIENT) {
            Notice("Process creation time");
            printf("(%d) : %lldns\n", clientCount, elapseTime);
        }

    }
}

// 서버로 들어오는 소켓을 설정합니다.
int setSocket(int port) {
    int sock;
    struct sockaddr_in ServAddr;
    int nVal;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &nVal, sizeof (nVal))<0){
        perror("setsocketopt error");
        return -1;
    }

    memset(&ServAddr, 0, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServAddr.sin_port = htons(port);

    if(bind(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0){
        perror("bind error");
        return -1;
    }

    if(listen(sock, MAXPENDING) < 0){
        perror("listen error");
        return -1;
    }

    return sock;
}

// 소켓으로부터 클라이언트의 요청을 받아, 정보를 Client 구조체에 채워 리턴합니다.
Client* getClient(int socket) {
    struct sockaddr_in ClientAddr;
    int clientLen;
    int clientSocket;
    Client* newClient;

    // 소켓으로부터 클라이언트의 요청을 받습니다.
    clientLen = sizeof(ClientAddr);
    clientSocket = accept(socket, (struct sockaddr *) &ClientAddr, &clientLen);

    if (clientSocket < 0) {
        perror("accept error");
        return (Client*) 0;
    }

    // 새 구조체 영역을 할당합니다.
    newClient = (Client*) malloc( sizeof(Client) );
    memset(newClient, 0, sizeof(Client));

    // 기본 정보를 채워넣습니다.
    newClient->socket = clientSocket;
    newClient->port = ClientAddr.sin_port;
    newClient->ip = inet_ntoa(ClientAddr.sin_addr);

    if ( -1 == pipe(newClient->pipe) ) {
        perror("PIPE ERR");
        free(newClient);
        return (Client*) 0;
    }

    return newClient;
}

// 리스트에 새 클라이언트를 추가합니다.
Client* addClient(Client* newClient) {
    char buffer[256];
    char head[HEAD_SIZE], value[VALUE_SIZE];

    int n;

    if (newClient == 0) return 0;

    // 우선 클라이언트가 일반 클라이언트이지, shell 클라이언트인지를 알아냅니다.
    readMsg(newClient->socket, head, value);
    if ( !strcmp(head, MODE) && !strcmp(value, MODE_CLIENT) ) {
        // 일반 client 모드면 싱글 링크드리스트에 추가합니다.
        newClient->nextClient = 0;
        if (front != 0)
            newClient->nextClient = front;
        // 그냥 맨 앞에 넣습니다. 순서는 중요하지 않습니다.
        front = newClient;
        newClient->isShell = false;

    } else
    if ( !strcmp(head, MODE) && !strcmp(value, MODE_SHELL) ) {
        // shell client 모드면 shell 모드로 접속한 다른 클라이언트가 없는지 확인합니다.
        if (shellClient != 0) {
            writeMsg(newClient->socket, ERR, "SHELL_DUP");
            exit(1);
        }

        // shell 클라이언트는 리스트에 넣지 않고 따로 관리합니다.
        shellClient = newClient;
        newClient->isShell = true;
    } else {
        // 알 수 없는 client 접속
        writeMsg(newClient->socket, ERR, PROTO_ERR);
        return 0;
    }

    // 클라이언트의 이름을 설정합니다.
    sprintf(buffer, "Client(%d)", clientCount);
    clientCount++;
    writeMsg(newClient->socket, SET_NAME, buffer);

    // 클라이언트 정보를 출력합니다.
    Notice("Client");
    printf("(%d) connected, IP: %s, Port: %d\n", clientCount, newClient->ip, newClient->port);

    // 끝났습니다. 클라이언트에게 정상적으로 접속했다고 알립니다.
    readMsg(newClient->socket, head, value);
    writeMsg(newClient->socket, ACCEPT, "server_ok");

    return newClient;
}

void split(Client* newClient) {
    NSecond start, end;

    if (newClient == 0) return ;

    start = getNSecond();
    // process를 사용하는 서버
    newClient->pid = fork();

    end = getNSecond();

    // fork 결과에 따라 분기합니다.
    switch (newClient->pid ) {
    case -1:
        Notice("Error during fork\n");
        close(newClient->socket);
        return ;

    case 0:
        // client라면
        currentClient = newClient;
        // 쓰기 파이프를 막습니다. 읽기만 합니다.
        close(newClient->pipe[1]);

        // 클라이언트 타입에 따라 할 일을 처리합니다.
        if (newClient->isShell) doShell(newClient);
        else doClient(newClient);
        break;

    default:
        // server라면 분기하는 데 걸린 시간을 구합니다.
        elapseTime += getDiff(end, start);
        // 읽기 파이프는 막습니다. 쓰기만 합니다.
        close(newClient->pipe[0]);
        break;
    }
}

void doShell(Client* shellClient) {
    Client* p;
    char head[HEAD_SIZE], value[VALUE_SIZE];

    char fileBuffer[FILE_BUFFER_SIZE + 1];
    char fileChunk[FILE_CHUNK_SIZE];
    char filename[256];
    unsigned long fileSize;
    int fd;
    FILE* fp;

    FDEBUG(("shell start\n"));
    while (1) {
        // shell 담당 서버는 shell 클라이언트로부터 신호를 받아야만 일합니다.
        readMsg(shellClient->socket, head, value);

        if ( !strcmp(head, ADD) ) {
            // 파일 추가
            FDEBUG(("get ADD message\n"));
            fileSize = 0;

            // 우선 해당 이름으로 파일을 엽니다.
            strcpy(filename, value);
            fd = open(filename, O_WRONLY | O_CREAT, 0644);
            if (fd == -1) {
                writeMsg(shellClient->socket, ERR, "fileopen_err");
                continue;
            }

            // 다른 일반 클라이언트들에게도 이 사실을 알립니다.
            writeMsg(shellClient->socket, ACCEPT, "fileadd");
            p = front;
            while (p) {
                writeMsg(p->pipe[1], ADD, filename);
                p = p->nextClient;
            }

            // 파일 구분자를 입력받습니다.
            readMsg(shellClient->socket, head, fileChunk);
            writeMsg(shellClient->socket, ACCEPT, "filechunk");
            p = front;
            while (p) {
                writeMsg(p->pipe[1], CHUNK, fileChunk);
                p = p->nextClient;
            }

            // 파일을 읽어서 클라이언트에게 전달합니다. 서버에도 한 부 저장합니다.
            FDEBUG(("start reading file\n"));
            while (true) {
                memset(fileBuffer, 0, sizeof(fileBuffer));
                readData(shellClient->socket, fileBuffer);

                p = front;
                while (p) {
                    writeData(p->pipe[1], fileBuffer);
                    p = p->nextClient;
                }

                // 파일 전송이 끝나면 끝냅니다.
                if ( !strcmp(fileBuffer, fileChunk) ) break;
                // 아니라면 서버에 저장하고 다음 부분을 보내달라고 요청합니다.
                else writeData(fd, fileBuffer);
                writeMsg(shellClient->socket, CHUNK, fileChunk);
                fileSize += strlen(fileBuffer);
            }
            // 파일을 전부 기록했습니다. 닫고 EOF 메시지를 전달합니다.
            close(fd);
            writeMsg(shellClient->socket, ACCEPT, "eof");
            p = front;
            while (p) {
                writeMsg(p->pipe[1], ACCEPT, "eof");
                p = p->nextClient;
            }

            // 결과를 출력합니다.
            Notice("ADD ");
            printf("[%s][%ldbyte] from Client(%d)\n", filename, fileSize, shellClient->clientNo);

        } else
        if ( !strcmp(head, DEL) ) {
            // 삭제할 파일의 크기를 구합니다.
            fp = fopen(value, "r");
            fseek(fp, 0L, SEEK_END);
            fileSize = ftell(fp);
            fclose(fp);

            // 결과를 출력합니다.
            Notice ("DELETE ");
            printf("[%s][%ldbyte] from Client(%d)\n", value, fileSize, shellClient->clientNo);

            // 파일을 삭제하고 다른 클라이언트에게 알립니다.
            remove( value );

            p = front;
            while (p) {
                writeMsg(p->pipe[1], head, value);
                p = p->nextClient;
            }

        }
    }
}

void doClient(Client* client) {
    char buffer[FILE_BUFFER_SIZE + 1];

    FDEBUG(("client start\n"));
    // 여기서는 무조건 파이프에서 읽어서 클라이언트한테 전달합니다.
    while (true) {
        readData(client->pipe[0], buffer);
        writeData(client->socket, buffer);
    }
}
