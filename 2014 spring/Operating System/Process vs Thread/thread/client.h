#ifndef CLIENT_H
#define CLIENT_H

// 접속을 위한 기본 루틴
int init(int portno, char* hostname, bool isShell);

// 서버 명령을 처리하는 루틴
void doNormalMode(int socket);
void doShellMode(int socket);

#endif
