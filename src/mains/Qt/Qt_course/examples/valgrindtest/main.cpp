#include <string.h>
#include <stdio.h>

int main()
{
    const char* str = "Hello World!";
    char* copy = (char*) new char[ strlen(str) ];
    strcpy( copy, str );
    printf( "copy=%s\n", copy );
    delete copy;
    printf( "str=%s\n", str );
}
