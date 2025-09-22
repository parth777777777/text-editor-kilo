/*** includes ***/
#include <asm-generic/errno-base.h>
#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>      // for iscntrl()
#include <errno.h>      // for errno
#include <termios.h>    // terminal settings
#include <stdlib.h>
#include <sys/ioctl.h>  // get window size

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f) // turn a character into its Ctrl equivalent

/*** data ***/
struct editorConfig {
    struct termios old;  // store original terminal settings to restore on exit
    int screenrows;
    int screencols;
};
struct editorConfig E;

/*** terminal ***/

// print an error and exit
void die(const char *s){
    write(STDOUT_FILENO,"\x1b[2J",4); // clear screen
    write(STDOUT_FILENO,"\x1b[H",3);  // move cursor to top-left
    perror(s);  // print reason for failure
    exit(1);
}

// restore terminal settings
void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.old) == -1)
        die("tcsetattr");
}

// put terminal in raw mode
void enableRawMode(){
    atexit(disableRawMode); // make sure we restore terminal when program exits

    if (tcgetattr(STDIN_FILENO, &E.old) == -1) // get current settings
        die("tcgetattr");

    struct termios raw = E.old;

    // disable stuff that can interfere with raw input
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
    // ECHO: stop automatic echoing of typed characters
    // ICANON: turn off canonical mode so input is immediate
    // IEXTEN: disable Ctrl-V etc
    // ISIG: turn off Ctrl-C and Ctrl-Z signals

    raw.c_iflag &= ~(IXON | ICRNL | INPCK | ISTRIP | BRKINT);
    // IXON: disable Ctrl-S / Ctrl-Q
    // ICRNL: don’t convert CR to NL
    // INPCK, ISTRIP, BRKINT: disable input processing

    raw.c_oflag &= ~(OPOST); // disable output processing

    raw.c_cflag |= (CS8); // 8-bit characters

    raw.c_cc[VMIN] = 0;   // read returns as soon as input is available
    raw.c_cc[VTIME] = 1;  // or after 0.1s timeout

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

// read one keypress from user
char editorReadKey(){
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
        // keep trying if read is interrupted
    }
    return c;
}

// figure out the terminal window size
int getWindowSize(int* rows, int* cols){
    struct winsize ws;
    if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) || ws.ws_col == 0 || ws.ws_row == 0)
        return -1;
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}

/*** input ***/

// handle keypresses
void editorProcessKeypress(){
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'): // quit on Ctrl-Q
            write(STDOUT_FILENO,"\x1b[2J",4); // clear screen
            write(STDOUT_FILENO,"\x1b[H",3);  // move cursor top-left
            exit(0);
    }
}

/*** output ***/

// draw every row of the editor
void editorDrawRows(){
    for (int y = 0; y < E.screenrows; y++){
        write(STDOUT_FILENO,"~\r\n",3); // show ~ on empty lines
    }
}

// refresh the screen
void editorRefreshScreen(){
    write(STDOUT_FILENO,"\x1b[2J",4); // clear everything
    write(STDOUT_FILENO,"\x1b[H",3);  // move cursor to top-left

    editorDrawRows();                  // draw the rows
    write(STDOUT_FILENO,"\x1b[H",3);  // move cursor back to top-left
}

/*** init ***/

void initEditor(){
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

/*** main ***/

int main(){
    enableRawMode(); // raw mode on
    initEditor();    // figure out screen size

    while (1) {
        editorRefreshScreen(); // redraw screen
        editorProcessKeypress(); // handle keys
    }

    return 0;
}

