/**
 * shared.c
 * 2009147045 강대연
 * 서버와 클라이언트에서 공통적으로 사용되는 코드가 들어 있습니다.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shared.h"

/**
 * 데이터를 기록합니다. 소켓과 파이프, 파일에 공통적으로 사용됩니다.
 * @param (int)     fd: 디스크립퍼 번호입니다.
 * @param (char*)   data: 기록할 데이터입니다. 다른 함수와의 호환을 위해 반드시 길이를 FILE_BUFFER_SIZE로 설정해야 합니다.
 */
int writeData(int fd, char* data) {
    FDEBUG(("writeData: %s/\n", data));
    return write(fd, data, strlen(data));
}
/**
 * 메시지를 기록합니다. 소켓과 파이프에 공통적으로 사용됩니다.
 * @param (int)     fd: 디스크립퍼 번호입니다.
 * @param (char*)   head: 메시지 헤더입니다.
 * @param (char*)   value: 메시지 내용입니다.
 */
int writeMsg(int fd, char* head, char* value) {
    char buffer[FILE_BUFFER_SIZE];
    memset(buffer, 0, FILE_BUFFER_SIZE);

    sprintf(buffer, "%s %s", head, value);
    return writeData(fd, buffer);
}

/**
 * 데이터를 읽습니다. 소켓과 파이프에 공통적으로 사용됩니다.
 * @param (int)     fd: 디스크립퍼 번호입니다.
 * @param (char*)   data: 읽을 데이터입니다. 반드시 길이를 FILE_BUFFER_SIZE로 설정해야 합니다.
 */
int readData(int fd, char* data) {
    int ret;

    memset(data, 0, FILE_BUFFER_SIZE);
    ret = read(fd, data, FILE_BUFFER_SIZE - 1);
    if ( ret == 0 ) exit(1);

    FDEBUG(("readData: %s/\n", data));
    return ret;
}

/**
 * 메시지를 읽습니다. 소켓과 파이프에 공통적으로 사용됩니다.
 * @param (int)     fd: 디스크립퍼 번호입니다.
 * @param (char*)   head: 메시지 헤더입니다.
 * @param (char*)   value: 메시지 내용입니다.
 */
int readMsg(int fd, char* head, char* value) {
    char buffer[FILE_BUFFER_SIZE];
    int ret;

    memset(buffer, 0, FILE_BUFFER_SIZE);
    ret = readData(fd, buffer);
    sscanf(buffer, "%s", head);
    strcpy(value, &buffer[ strlen(head) + 1 ]);

    return ret;
}

/**
 * timestamp와 함께 메시지를 출력해주는 매크로 함수입니다.
 */
inline void Notice(char* message) {
    timestamp();
    printf("%s", message);
}

/**
 * 현재 시간을 로그 형태로 출력해줍니다.
 */
void timestamp() {
    time_t now;
    struct tm *t;

    now = time(0);
    t = localtime(&now);

    printf(" [%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}
