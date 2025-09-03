/*** includes ***/
#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <unistd.h>
#include<ctype.h>
#include <errno.h>
#include <termios.h>
#include<stdlib.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios old;

/*** terminal ***/

void die(const char *s){
    perror(s);
    exit(1);
}
void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &old) == -1)
        die("tcesetattr");
}

void enableRawMode(){
    atexit(disableRawMode);
    if (tcgetattr(STDIN_FILENO, &old)== -1)
        die("tcgetattr");
    struct termios raw = old;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN |ISIG  );
    raw.c_iflag &= ~(IXON | ICRNL | INPCK | ISTRIP | BRKINT);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
}

char editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO,&c,1)) != 1){
        if(nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

/*** input ***/

void editorProcessKeypress(){
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

/*** init ***/

int main(){
    enableRawMode();

    while(1){
        editorProcessKeypress();
    }

    return 0;
}

