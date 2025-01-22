/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct editorConfig
{
  int screenrows;
  int screencols;

  struct termios orig_termios;
};

struct editorConfig E; /* Editor state */

/*** terminal ***/
void die(const char *s)
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

void disableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
  {
    die("tcsetattr");
  }
}

void enableRawMode()
{
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
  {
    die("tcgetattr");
  }
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_cflag |= (CS8);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
  {
    die("tcsetattr");
  }
}

char editorReadKey()
{
  int nread;
  char c;


  while((nread = read(STDIN_FILENO, &c, 1)) != 1)
  {
    if(nread == -1 && errno != EAGAIN)
    {
      die("read");
    }
  }
  return c;
}

int getCursorPosition(int *rows, int *cols) 
{
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
  {
    return -1;
  }
  printf("\r\n");
  char c;

  while (read(STDIN_FILENO, &c, 1) == 1)
  {
    if (iscntrl(c))
    {
      printf("%d\r\n", c);
    } 
    else 
    {
      printf("%d ('%c')\r\n", c, c);
    }
  }
  editorReadKey();
  return -1;
}

int getWindowSize(int *rows, int *cols)
{
  struct winsize ws;

  if(1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
  {
    if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
    {
      return -1;
    }
    return getCursorPosition(rows, cols);
  }
  else
  {
    *cols = ws.ws_col;
    *rows = ws.ws_row;

    return 0;
  }
}

/*** output ***/
void editorDrawRows()
{
  int y;
  for (y= 0; y < E.screenrows; y++)
  {
    write(STDIN_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen()
{
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();

  write(STDERR_FILENO, "\x1b[H", 3);
}

void editorProcessKeyPress()
{
  char c = editorReadKey();

  switch (c)
  {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

/*** init ***/
void initEditor()
{
  if(getWindowSize(&E.screenrows, &E.screencols) == -1)
  {
    die("getWindowSize");
  }
}

int main()
{
  enableRawMode();
  initEditor();

  while(1)
  {
    editorRefreshScreen();
    editorProcessKeyPress();
  }

  // while(1) //read() and STDIN_FILENO come from <unistd.h
  // {
  //   char c = '\0';
  //   if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
  //   {
  //     die("read");
  //   }
  //   if(iscntrl(c)) /*iscntrl tests whether a character is a control character (non printable characters).*/
  //   {
  //     printf("%d\r\n", c); /*printf prints multiple presentations of a byte, %d tells it to format the byte as a decimal number, %c tells it to write out the byte directly as a character*/
  //   }
  //   else
  //   {
  //     printf("%d ('%c')\r\n", c, c);
  //   }
    
  //   if(c == CTRL_KEY('q'))
  //   {
  //     break;
  //   }
  // }
  return 0;
}

//pt 1
//read() and STD_FILENO comes from the header library
//we are asking read() to read 1 byte from the standard input into the variable c
//and to keep doing it until there are no more bytes to read. read() also returns
//the number of bytes that it reads and returns 0 when it reaches the end of the file

/*
pt 2

ECHO feature causes each key you type to be printed to the terminal, so you can see what youre typing.
This is useful in canonical mode

termios can be read into struct by tcgetattr

ECHO is a bit flag
*/


/*
Last part left off at:

We’re going to have to parse this response. But first, let’s read it into a buffer.
We’ll keep reading characters until we get to the R character.

*/