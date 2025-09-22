/*** includes ***/
#include <asm-generic/errno-base.h>
#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <unistd.h>
#include<ctype.h>
#include <errno.h>
#include <termios.h>
#include<stdlib.h>
#include<sys/ioctl.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct editorConfig {
    struct termios old;
    int screenrows;
    int screencols;

};
struct editorConfig E;

/*** terminal ***/

void die(const char *s){
    write(STDOUT_FILENO,"\x1b[2J",4);
    write(STDOUT_FILENO,"\x1b[H",3);

    perror(s);
    exit(1);
}
void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.old) == -1)
        die("tcesetattr");
}

void enableRawMode(){
    atexit(disableRawMode);
    if (tcgetattr(STDIN_FILENO, &E.old)== -1)
        die("tcgetattr");
    struct termios raw = E.old;
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

int getWindowSize(int* rows, int* cols){
    struct winsize ws;
    if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) || ws.ws_col == 0 || ws.ws_row == 0) return -1;
    else {
        *rows = ws.ws_row;
        *cols = ws.ws_col;
        return 0;
    }
}

/*** input ***/

void editorProcessKeypress(){
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO,"\x1b[2J",4);
            write(STDOUT_FILENO,"\x1b[H",3);
            exit(0);
            break;
    }
}

/*** output ***/

void editorDrawRows(){
    for(int y = 0 ; y<E.screenrows ; y++){
        write(STDOUT_FILENO,"~\r\n",3);
    }
}

void editorRefreshScreen(){
    write(STDOUT_FILENO,"\x1b[2J",4);  // \x1b[2J is an espace sequence , any esc sequence starts with \x1b , which is the esc charector followed by a [ charector
                                       // we are using J command which clears the screen with arguement 2 that clears entire screen.
                                       // 4 in write(args) means we are writing 4 bytes to STDOUT_FILENO
    write(STDOUT_FILENO,"\x1b[H",3); // move cursor to top of screen at each refresh
    
    editorDrawRows();
    write(STDOUT_FILENO,"\x1b[H",3);
}



/*** init ***/

int initEditor(){
    if (getWindowSize(&E.screenrows,&E.screencols) == -1) die("getWindowSize");
}

int main(){
    enableRawMode();
    initEditor();

    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}

