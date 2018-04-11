#include <stdio.h>
#include <stdlib.h>

const char pluginName[] = "skeleton";

void setup(void **handle)
{
    int *value = (int *) malloc(sizeof(int));
    *value = 42;
    *handle = value;
}

void destroy(void **handle)
{
    free(*handle);
}
