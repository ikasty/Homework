#ifndef SERVER_H
#define SERVER_H

#define INADDR "localhost"
#define INPORT  10000
#define MAXPENDING 5
#define MAX_CLIENT 100

/**
 * 클라이언트 구조체입니다. 링크드리스트로 관리합니다.
 */
typedef struct Client Client;
struct Client {
    int clientNo;               // 클라이언트 고유 번호입니다. 접속 순서대로 할당받습니다.
    int socket;                   // 클라이언트가 접속한 소켓입니다.
    Client* nextClient;   // 링크드리스트로 연결된 다음 클라이언트 구조체의 주소입니다.
    pthread_t tid;             // thread 모델에서의 thread_id입니다.
    int pipe[2];                   // 통신에 사용되는 파이프입니다.
    bool isShell;               // 셸 모드인지의 여부를 나타냅니다.
    char* ip;                       // 접속한 ip를 나타냅니다.
    int port;                       // 접속한 포트 번호를 나타냅니다.
};

// 소켓 설정
int setSocket(int port);
Client* getClient(int socket);

// Client list 추가
Client* addClient(Client* cliSock);

// pthread로 split하기
void split(Client* newClient);
void *thread_run(void* client);

// shell 모드 클라이언트 처리
void doShell(Client* shellClient);

// normal 모드 클라이언트 처리
void doClient(Client* client);

#endif
