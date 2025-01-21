#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void die(const char *s)
{
  perror(s);
  exit(1);
}

void disableRawMode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1
  {
    die("tcsetattr");
  }
}

void enableRawMode()
{
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
  {
    die("tcgetattr");
  }
  atexit(disableRawMode);

  struct termios raw = orig_termios;
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

int main()
{
  enableRawMode();

  while(1) //read() and STDIN_FILENO come from <unistd.h
  {
    char c = '\0';
    if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGIN)
    {
      die("read");
    }
    if(iscntrl(c)) /*iscntrl tests whether a character is a control character (non printable characters).*/
    {
      printf("%d\r\n", c); /*printf prints multiple presentations of a byte, %d tells it to format the byte as a decimal number, %c tells it to write out the byte directly as a character*/
    }
    else
    {
      printf("%d ('%c')\r\n", c, c);
    }
    
    if(c == 'q')
    {
      break;
    }
  }
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
