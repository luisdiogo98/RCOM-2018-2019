#include "protocol.h"

int alarm_flag = 1;

void attend()
{
    alarm_flag = 1;
}

void disableAlarm()
{
    alarm_flag = 0;
}

int read_message(int fd, char buf[])
{
    int state = BEGIN;
    int pos = 0;

    int res;
    char c;

    while(alarm_flag != 1 && state != END)
    {
        res = read(fd,&c,1);

		if(res > 0)
		{
		    switch(state)
		    {
		        case BEGIN:
				{
		            if(c == FLAG)
		            {
		                buf[pos] = c;
		                pos++;
		                state = START_MESSAGE;
		            }
		            break;
				}
		        case START_MESSAGE:
		        {
		            if(c != FLAG)
		            {
		                buf[pos] = c;
		                pos++;
		                state = MESSAGE;
		            }
		            break;
		        }
		        case MESSAGE:
		        {
					buf[pos] = c;
		            pos++;
		            if(c == FLAG)
		            {
		                state = END;
		            }
		            break;
		        }
		        default: state = END;
		    }
		}
    }

    if(alarm_flag == 1)
        return 1;

    return 0;
}

int write_message(int fd, char buf[], int size)
{

    write(fd, buf, size);

    sleep(1);

    return 0;
}

char parseMessageType(char buf[])
{
    if(buf[0] != FLAG)
        return ERROR;

    if(buf[1] != A_SENDER && buf[1] != A_RECEIVER)
        return ERROR;

    if((buf[2] ^ buf[1]) != buf[3])
        return ERROR;

    if(buf[2] == C_DISC || buf[2] == C_SET || buf[2] == C_UA)
    {
        if(buf[4] == FLAG)
            return buf[2];
        else
            return ERROR;
    }

    return ERROR;
}

char calculateBCC2(char *message, int size)
{
    char bcc2 = message[0];
    int i = 1;

    for(; i < size; i++)
        bcc2 ^= message[i];

    return bcc2;
}

char* stuffing_data_package(const char* package, const char BCC2, int* char_count)
{
    char* stuff = (char *)malloc(265 * 2 * sizeof(char));

    *char_count = 1;

    stuff[0] = package[0];

    if(package[1] == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(package[1] == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = package[1];
    }

    stuff[(*char_count)++] = package[2];

    if(package[3] == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(package[3] == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = package[3];
    }

    int count = 4;
    int i = package[2] * 256 + package[3];

    for(; count < 4 + i; count++)
    {
        if(package[count] == 0x7E)
        {
            stuff[(*char_count)++] = 0x7D;
            stuff[(*char_count)++] = 0x5E;
        }
        else if(package[count] == 0x7D)
        {
            stuff[(*char_count)++] = 0x7D;
            stuff[(*char_count)++] = 0x5D;
        }
        else
        {
            stuff[(*char_count)++] = package[count];
        }
    }

    if(BCC2 == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(BCC2 == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = BCC2;
    }

    return stuff;
}

char* stuffing_control_package(const char* package, const char BCC2, int* char_count)
{
    int size = package[2];

    char* stuff = (char *)malloc( (5 + size + package[3+size] * 256 + package[4+size]) * 2 * sizeof(char) );

    *char_count = 1;

    stuff[0] = package[0];

    if(package[1] == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(package[1] == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = package[1];
    }

    if(package[2] == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(package[2] == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = package[2];
    }

    int count = 3;

    for(; count < (3+size); count++)
    {
        if(package[count] == 0x7E)
        {
            stuff[(*char_count)++] = 0x7D;
            stuff[(*char_count)++] = 0x5E;
        }
        else if(package[count] == 0x7D)
        {
            stuff[(*char_count)++] = 0x7D;
            stuff[(*char_count)++] = 0x5D;
        }
        else
        {
            stuff[(*char_count)++] = package[count];
        }
    }

    if(package[3+size] == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(package[3+size] == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = package[3+size];
    }

    if(package[4+size] == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(package[4+size] == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = package[4+size];
    }

    count = 5 + size;
    int start_pnt = count;
    size = package[4+size];

    for(; count < (start_pnt + size); count++)
    {
        if(package[count] == 0x7E)
        {
            stuff[(*char_count)++] = 0x7D;
            stuff[(*char_count)++] = 0x5E;
        }
        else if(package[count] == 0x7D)
        {
            stuff[(*char_count)++] = 0x7D;
            stuff[(*char_count)++] = 0x5D;
        }
        else
        {
            stuff[(*char_count)++] = package[count];
        }
    }

    if(BCC2 == 0x7E)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5E;
    }
    else if(BCC2 == 0x7D)
    {
        stuff[(*char_count)++] = 0x7D;
        stuff[(*char_count)++] = 0x5D;
    }
    else
    {
        stuff[(*char_count)++] = BCC2;
    }

    return stuff;
}

char* stuffing(const char* package, const char BCC2, int* char_count)
{
    if(package[0] == C2_DATA)
    {
        return stuffing_data_package(package, BCC2, char_count);
    }
    else
    {
        return stuffing_control_package(package, BCC2, char_count);
    }
}

char* heading(char * stuff, int count, int flag)
{
    char * message = (char *)malloc( (5 + count) * sizeof(char));

    message[0] = FLAG;
    message[1] = A_SENDER;
    message[2] = (char)(flag * 64);
    message[3] = A_SENDER ^ message[2];

    int i = 4;

    for(; i < 5 + count; i++)
    {
        message[i] = stuff[i - 4];
    }

    message[i] = FLAG;

    return message;
}

int send_message(int fd, char* message)
{
    int res = write(fd,message,255);
    fflush(NULL);
    //sleep(1);
    return res;
}
