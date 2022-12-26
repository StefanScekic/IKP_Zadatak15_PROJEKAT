#include <stdio.h>
#include "Interface.h"

int main()
{
    int dataSize = 5;
    void* data = &dataSize;

    ReceiveData(data, dataSize);
}

