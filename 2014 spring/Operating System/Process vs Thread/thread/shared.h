/**
 * 디버그 모드로 컴파일 시 디버깅 메시지를 출력해주는 매크로입니다.
 */
#ifdef DEBUG
#define FDEBUG(X) printf X
#else
#define FDEBUG(X) 0
#endif

#ifndef SHARED_H
#define SHARED_H

typedef char bool;
#define true 1
#define false 0

/**
 * 통신 규약 메시지들입니다. 서버와 클라이언트, 서버의 각 프로세스/스레드 간의 통신에 사용됩니다.
 */
// file 버퍼는 1024이고, 헤더가 20byte이다.
#define FILE_CHUNK_SIZE 512 - 20
#define FILE_BUFFER_SIZE 512

// head value 형태이며, head는 4문자 이내의 공백을 포함하지 않는 문자열, value는 길이가 239문자 이하인 문자열이어야 한다.
#define HEAD_SIZE   10+ 1
#define VALUE_SIZE  239 + 1

// MODE client_mode
#define     MODE                    "MODE"
#define         MODE_CLIENT     "client"
#define         MODE_SHELL      "shell"

// ACCEPT ok
#define         ACCEPT              "ACT"

// SET key
#define         SET_NAME            "NAME"

// ERR error_str
#define     ERR                     "ERR"
#define     PROTO_ERR           "proto_err"

// PING rand_str
#define     PING                    "PING"

// ADD filename
#define     ADD                     "ADD"
// SIZE size_int
#define     SIZE                    "SIZE"
// CHUNK chunk_msg
#define     CHUNK                   "CNK"
#define     CHUNK_MSG           "=====CHUNK====="

// DEL filename
#define     DEL                     "DEL"

/**
 * 데이터를 읽는 함수입니다. 변수의 초기화를 대신 해주며, 디버그 모드 시
 * 디버깅 출력을 해 줍니다.
 */
int readMsg(int socket, char* head, char* value);
int readData(int socket, char* data);

/**
 * 데이터를 쓰는 함수입니다. 디버그 모드 시 디버깅 출력을 해 줍니다.
 */
int writeMsg(int socket, char* head, char* buffer);
int writeData(int socket, char* data);

/**
 * timestamp와 함께 메시지를 출력해주는 매크로 함수입니다.
 */
inline void Notice(char* message);

/**
 * 현재 시간을 로그 형태로 출력해줍니다.
 */
void timestamp();


#endif
