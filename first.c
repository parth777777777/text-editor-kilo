#include <stdio.h>
#include <unistd.h>
#include<ctype.h>
#include <termios.h>
#include<stdlib.h>

struct termios old;
void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old);
}

void enableRawMode(){
    atexit(disableRawMode); // atexit function is automatically called when program exits , may it be returning or main function or by calling the exit() func
    struct termios raw = old;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG); // c_lflag and many other such flags are field in struct termios , c_lflag contains common flags
                            // like ECHO , ICANON, ISIG etc each of these is a single bit inside c_lflag = 0b10111001
                            // lets assume ECHO flag is 0b00001000 
                            // to toggle ECHO flag in c_lflag we take the AND of NOT of ECHO 
                            // ~ECHO = 0b11110111
                            // c_lflag & (~ECHO) = 10111001  
                            //                   & 11110111
                            //                   = 10110001 = new c_lflag but with ECHO bit turned off.

    raw.c_iflag &= ~(IXON);
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw); //set current terminal attrributes to be same as raw 
}

int main(){
    enableRawMode();

    char c;
    while(read(STDIN_FILENO,&c,1)== 1 && c != 'q'){ // read() return 1 when succesfull read , 0 when EOF , -1 on error
                                                    // so we are saying , read input as long as its succesfull or until we read 'q'
        if(iscntrl(c)){
            printf("%d\n",c);
        }
        else{
            printf("%d (%c)\n",c,c);
        }
    }
         
    return 0;


}
