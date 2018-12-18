#include <stdio.h>
#include "RingBuf.h"
#include <stdint.h>
#include <stdlib.h>

int main()
{
    RingBufCPP<int, 5> q;
    int tmp = 12;
    //q.add(tmp);

    for (uint16_t i=0; i< 100; i++) {
        int tmp = random();

        if (q.add(tmp)) {
            printf("Added %d\n", tmp);
        }
        else {
            q.pull(&tmp);
            break;
        }
    }

    while(!q.isEmpty()) {
        int pulled;
        q.pull(&pulled);
        printf("Got %d\n", pulled);
    }
}
