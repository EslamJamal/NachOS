#include "syscall.h"

void writeTooMany(){
    char s[260]; // 260 is too many. We chose 2^8 - 1 as max size.
    int i;
    
    for (i = 0; i < sizeof(s); i++){
        s[i] = '_';
    }
    s[sizeof(s) - 1] = '\n'; // puts() writes the string s and a trailing newline to stdout
    PutString(s); 
}

int main(){
    // Try one char
    PutString("a");
    // Try one int
    PutString("1");
    // Try a simple string
    PutString("Hello World!");
    // Try an empty string
    PutString("");
    // Try a non-ASCII character
    PutString("ẞ");
    // Try a (too) long string
    writeTooMany();
    Halt();
    /* not reached */
    return 0;
}

