#include "getnsec.h"

/**
 * 부팅 후 현재까지 소비된 시간을 나노초 단위로 리턴합니다.
 */
NSecond getNSecond() {
    NSecond t;
    syscall(351, &t);
    return t;
}

/**
 * 두 NSecond의 차를 계산하여 리턴합니다.
 */
NSecond getDiff(NSecond end, NSecond start) {
    NSecond diff = end - start;
    return diff;
}
