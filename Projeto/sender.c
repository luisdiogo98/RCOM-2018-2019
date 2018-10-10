#include "protocol.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


volatile int STOP=FALSE;

int setup()
{
    int fd;
    struct termios oldtio,newtio;

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY );
    fflush(NULL);

    if (fd <0) {perror("/dev/ttyS0"); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    return fd;
}

char *readFile(char* filename, off_t *sizeFile)
{
    FILE *file;

    struct stat fileInfo;
    char* fileContent;

    if( (file = fopen(filename, "rb")) == NULL)
    {
      perror("Error reading file.");
      exit(-1);
    }

    stat(filename, &fileInfo);
    (*sizeFile) = fileInfo.st_size;

    fileContent = (char *)malloc(fileInfo.st_size);

    fread(fileContent, sizeof(char), fileInfo.st_size, file);

    return fileContent;
}


int main(int argc, char** argv)
{
    if(argc != 2)
    {
      printf("Usage: %s <filename>\n", argv[0]);
      return -1;
    }

    int fd = setup();

    if(llopen(fd, SENDER) == 0)
    {
      printf("Connected\n");
    }
    else
    {
      printf("Failed\n");
      return -2;
    }

    //Opens the file to be sent
    off_t fileSize;
    char* fileContent;
    fileContent = readFile(argv[1],&fileSize);
    

    return 0;
}
