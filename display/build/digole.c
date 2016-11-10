/////////////////////////////////
// Digole Digital
/////////////////////////////////
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "clear-day.h"
#include "clear-night.h"
#include "cloudy.h"
#include "fog.h"
#include "partly-cloudy-day.h"
#include "partly-cloudy-night.h"
#include "rain.h"
#include "sleet.h"
#include "snow.h"
#include "wind.h"

#define _TEXT_ 0
#define _GRAPH_ 1
#define _rs 0
#define _rst 2
#define LCDWIDTH 128
#define LCDHEIGHT 64

// an abs() :)
#define abs(a) (((a) < 0) ? -(a) : (a))

// bit set
#define _BV(bit) (0x1 << (bit))
#define swap(a, b) { uint8_t t = a; a = b; b = t; }

int fd,fd2;
const unsigned char fonts[] = {6, 10, 18, 51, 120, 123};

// name of the port we will be using, Rasberry Pi model B (i2c-1)
char *fileName = "/dev/i2c-1";

// address of I2C device Digole
int  address = 0x27;

void delay (unsigned int howLong){
  struct timespec sleeper, dummy ;
  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;
  nanosleep (&sleeper, &dummy) ;
}
 int writebyte (uint8_t command) {
	    if( i2c_smbus_write_byte( fd, command ) < 0 ) {
            printf(" Error writing to i2c slave ");
			return -1;
        }
		return 0;
 }
 int writecmd(char *command) {
		char tmp[1000];
		int i;
		sprintf(tmp,"%s",command);
        printf(" Digole CMD: %s ",command);
		for (i=0;i<strlen(tmp);i++) {
			printf("%c",tmp[i]);
		    if ((writebyte(tmp[i]))==-1) {
				printf(" Error writing to i2c slave ");
				return -1;
			}
		}
		return 0;
 }
void setRot90(void) {
    writecmd("SD1");
}
void setRot180(void) {
    writecmd("SD2");
}
void setRot270(void) {
    writecmd("SD3");
}
void undoRotation(void) {
    writecmd("SD0");
}
void setRotation(uint8_t d) {
    writecmd("SD");
    writebyte(d);
}
void setContrast(uint8_t c) {
    writecmd("CT");
    writebyte(c);
}
int print(char *command) {
    char tmp[1000];
    writecmd("TT");
    writecmd(command);
    writebyte(0);
    return 0;
}
void displayStartScreen(uint8_t c) {
    writecmd("DSS");
    writebyte(c);
}
void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    writecmd("FR");
    writebyte(x);
    writebyte(y);
    writebyte(x + w);
    writebyte(y + h);
}
void drawFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    writecmd("DR");
    writebyte(x);
    writebyte(y);
    writebyte(x + w);
    writebyte(y + h);
}
void setColor(uint8_t color) {
    writecmd("SC");
    writebyte(color);
}
void drawBoxfill(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	setColor(0);
	drawBox(x, y, w,h);
	setColor(1);
	drawFrame(x, y, w,h);
}
void clearobd() {
	setColor(0);
	drawBox(0, 0, 128,57);
	setColor(1);
}
void drawCircle(uint8_t x, uint8_t y, uint8_t r, uint8_t f) {
    writecmd("CC");
    writebyte(x);
    writebyte(y);
    writebyte(r);
    writebyte(f);
}
void drawDisc(uint8_t x, uint8_t y, uint8_t r) {
    drawCircle(x, y, r, 1);
}
void drawPixel(uint8_t x, uint8_t y, uint8_t color) {
    writecmd("DP");
    writebyte(x);
    writebyte(y);
    writebyte(color);
}
void drawLine(uint8_t x, uint8_t y, uint8_t x1, uint8_t y1) {
    writecmd("LN");
    writebyte(x);
    writebyte(y);
    writebyte(x1);
    writebyte(y1);
}
void drawLineTo(uint8_t x, uint8_t y) {
    writecmd("LT");
    writebyte(x);
    writebyte(y);
}
void drawHLine(uint8_t x, uint8_t y, uint8_t w) {
    drawLine(x, y, x + w, y);
}
void drawVLine(uint8_t x, uint8_t y, uint8_t h) {
    drawLine(x, y, x, y + h);
}
void nextTextLine(void) {
    writebyte((uint8_t) 0);
    writecmd("TRT");
}
void setFont(uint8_t font) {
    writecmd("SF");
    writebyte(font);
}
void directCommand(uint8_t d) {
    writecmd("MCD");
    writebyte(d);
}
void directData(uint8_t d) {
    writecmd("MDT");
    writebyte(d);
}
void moveArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, char xoffset, char yoffset) {
    writecmd("MA");
    writebyte(x0);
    writebyte(y0);
    writebyte(x1);
    writebyte(y1);
    writebyte(xoffset);
    writebyte(yoffset);
}
void uploadStartScreen(int lon,  uint8_t *data,int xdelay) {
     int j,y;
     uint8_t c;
    writecmd("SSS");
    writebyte((uint8_t) (lon % 256));
    writebyte((uint8_t) (lon / 256));
    for (j = 0; j < lon;j++) {
        delay(xdelay);
        c=*(data+j);
        writebyte(c);
		printf(" 0x%.2X ",c);
		if ((j%16)==0) printf(" ");
    }
}
void uploadUserFont(int lon, uint8_t *data, uint8_t sect) {
     uint8_t c;
	 int j;
    writecmd("SUF");
    writebyte(sect);
    writebyte((uint8_t) (lon % 256));
    writebyte((uint8_t) (lon / 256));
    for ( j = 0; j < lon; j++) {
        if((j%32)==0) delay(10);
        delay(2);
        c=*(data+j);
        writebyte(c);
    }
}
void clearScreen(void) {
        writebyte('C');
		writebyte('L');
		writebyte(0);
		writebyte('C');
		writebyte('L');
		writebyte(0);
}
//set display mode
void setMode(uint8_t m) {
        writecmd("DM");
        writebyte(m);
}
//set text position back to previous, only one back allowed
void setTextPosBack(void) {
	writecmd("ETB");
}
void setTextPosOffset(char xoffset, char yoffset) {
	writecmd("ETO");
	writebyte(xoffset);
	writebyte(yoffset);
}
void setTextPosAbs(uint8_t x, uint8_t y) {
	writecmd("ETP");
	writebyte(x);
	writebyte(y);
}
void setLinePattern(uint8_t pattern) {
	writecmd("SLP");
	writebyte(pattern);
 }
void setPrintPos(uint8_t x, uint8_t y, uint8_t graph ) {
        if (graph == _TEXT_)
            writecmd("TP");
		else
            writecmd("GP");
         writebyte(x);
         writebyte(y);
}
void disableCursor(void) {
    writecmd("CS0");
}
void enableCursor(void) {
    writecmd("CS1");
}
void drawStr(uint8_t x, uint8_t y, char *s) {
		setTextPosOffset(x,y);
		print(s);
}
/*----------Functions for Graphic LCD/OLED adapters only---------*/
void drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap) {
    uint8_t i = 0;
	int j;
    if ((w & 7) != 0)
        i = 1;
    writecmd("DIM");
    writebyte(x); //x;
    writebyte(y);
    writebyte(w);
    writebyte(h);
    for ( j = 0; j < h * ((w >> 3) + i); j++) {
        writebyte(*(bitmap + j));
    }
}
void drawBitmap256(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap,uint8_t c ) {
	int j;
    writecmd("EDIM1");
    writebyte(x);
    writebyte(y);
    writebyte(w);
    writebyte(h);
    for ( j = 0; j < h * w; j++) {
        writebyte(*(bitmap + j));
    }
}
void drawBitmapc(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap,uint8_t c ) {
    uint8_t i = 0;
	int j;
	if (c!=0) c=0xff;
    if ((w & 7) != 0)
        i = 1;
    writecmd("DIM");
    writebyte(x);
    writebyte(y);
    writebyte(w);
    writebyte(h);
    for ( j = 0; j < h * ((w >> 3) + i); j++) {
        writebyte((*(bitmap + j))^c);
    }
}
const char *byte_to_binary(int x) {
    static char b[9];
    b[0] = '\0';
    int z;
    for (z = 128; z > 0; z >>= 1){
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    return b;
}
 int printxy(uint8_t x, uint8_t y, char *command) {
	setPrintPos(x, y,_TEXT_);
	writecmd("TT");
	writecmd(command);
	writebyte(0);
	return 0;
 }
 int printxy_abs(uint8_t x, uint8_t y, char *command) {
	setTextPosAbs(x, y);
	writecmd("TT");
	writecmd(command);
	writebyte(0);
	return 0;
 }
 int printxyf_abs(uint8_t x, uint8_t y,uint8_t font, char *command) {
	setFont(fonts[font]);
	setTextPosAbs(x, y);
	writecmd("TT");
	writecmd(command);
	writebyte(0);
	return 0;
 }
int printxyf(uint8_t x, uint8_t y,uint8_t font, char *command) {
	setFont(fonts[font]);
	setPrintPos(x, y,_TEXT_);
	writecmd("TT");
	writecmd(command);
	writebyte(0);
	return 0;
 }
int printnxyf(uint8_t x, uint8_t y,uint8_t font, int command) {
    char tmp[100];
    sprintf(tmp,"%d",command);
    setFont(fonts[font]);
    setPrintPos(x, y,_TEXT_);
    writecmd("TT");
    writecmd(tmp);
    writebyte(0);
    return 0;
 }
 int printnxyf_abs(uint8_t x, uint8_t y,uint8_t font, int command) {
    char tmp[100];
    sprintf(tmp,"%d",command);
    setFont(fonts[font]);
    setTextPosAbs(x, y);
    writecmd("TT");
    writecmd(tmp);
    writebyte(0);
    return 0;
 }
int LCDInit( uint8_t contrast) {
    // Open port for reading and writing
    if ((fd = open (fileName, O_RDWR)) < 0) {
        printf(" Failed to open i2c port ");
        return -1;
    }
    // Set the port options and set the address of the device
    if (ioctl(fd, I2C_SLAVE, address) < 0) {
        printf(" Unable to get bus access to talk to slave ");
        return -1;
    }
	return (0);
}
void LCDdrawbitmap(uint8_t x, uint8_t y, uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color,uint8_t *dest) {
uint8_t j,i,jj;
uint8_t o,p,k;
printf(" 000 0b ");
k=0;
for (p=0;p<(w/8);p++)
  for (o=0;o<=7;o++) {
		for ( j=0; j<h; j++) {
			for ( i=0; i<(w/8); i++ ) {
				uint8_t d = *(bitmap + i + (j)*w/8);
				for (jj = 8; jj>0; jj--)	{
					if (((jj-1)==(7-o)) && (i==p)) {
						if (d & _BV(jj-1))
						  printf("1");
						else
						  printf("0");
						k++;
						if (k%8==0)
							printf(",0b");
					 }
				}
			}
		}
		k=0;
		printf(" %.03d 0b ",(p*8)+o);
	}
}

int main (int argc, char* argv[]) {
    int i=0;
	LCDInit(0);
    printf(" %s ", argv[0]);

    // show incoming commands   
    printf(" arg%d=%s ", 1, argv[1]);
    printf(" arg%d=%s ", 2, argv[2]);

    // parse command to char
    char digoleCommand[100];
    sprintf(digoleCommand,"%s",argv[1]);
    
    // based on command issue call to Digole screen
    if (strcmp(digoleCommand, "setRot90") == 0) {
        setRot90();
    } else if (strcmp(digoleCommand, "setRot180") == 0) {
        setRot180();
    } else if (strcmp(digoleCommand, "setRot270") == 0) {
        setRot270();
    } else if (strcmp(digoleCommand, "undoRotation") == 0) {
        undoRotation();
    } else if (strcmp(digoleCommand, "setRotation") == 0) {
        setRotation((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "setContrast") == 0) {
        setContrast((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "displayStartScreen") == 0) {
        displayStartScreen((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "drawBox") == 0) {
        drawBox((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]));
    } else if (strcmp(digoleCommand, "drawFrame") == 0) {
        drawFrame((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]));
    } else if (strcmp(digoleCommand, "setColor") == 0) {
        setColor((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "drawBoxfill") == 0) {
        drawBoxfill((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]));
    } else if (strcmp(digoleCommand, "clearobd") == 0) {
        clearobd();
    } else if (strcmp(digoleCommand, "drawCircle") == 0) {
        drawCircle((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]));
    } else if (strcmp(digoleCommand, "drawDisc") == 0) {
        drawDisc((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]));
    } else if (strcmp(digoleCommand, "drawPixel") == 0) {
        drawPixel((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]));
    } else if (strcmp(digoleCommand, "drawLine") == 0) {
        drawLine((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]));
    } else if (strcmp(digoleCommand, "drawLineTo") == 0) {
        drawLineTo((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]));
    } else if (strcmp(digoleCommand, "drawHLine") == 0) {
        drawHLine((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]));
    } else if (strcmp(digoleCommand, "drawVLine") == 0) {
        drawVLine((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]));
    } else if (strcmp(digoleCommand, "nextTextLine") == 0) {
        nextTextLine();
    } else if (strcmp(digoleCommand, "setFont") == 0) {
        setFont((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "directCommand") == 0) {
        directCommand((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "directData") == 0) {
        directData((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "moveArea") == 0) {
        moveArea((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]), (uint8_t)atoi(argv[6]), (uint8_t)atoi(argv[7]));
    } else if (strcmp(digoleCommand, "uploadStartScreen") == 0) {
        //uploadStartScreen(int lon,  uint8_t *data,int xdelay);
    } else if (strcmp(digoleCommand, "uploadUserFont") == 0) {
        //uploadUserFont(int lon, uint8_t *data, uint8_t sect);
    } else if (strcmp(digoleCommand, "clear") == 0) {
        clearScreen();
    } else if (strcmp(digoleCommand, "setMode") == 0) {
        setMode((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "setTextPosBack") == 0) {
        setTextPosBack();
    } else if (strcmp(digoleCommand, "setTextPosOffset") == 0) {
        setTextPosOffset((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]));
    } else if (strcmp(digoleCommand, "setTextPosAbs") == 0) {
        setTextPosAbs((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]));
    } else if (strcmp(digoleCommand, "setLinePattern") == 0) {
        setLinePattern((uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "setPrintPos") == 0) {
        setPrintPos((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]));
    } else if (strcmp(digoleCommand, "disableCursor") == 0) {
        disableCursor();
    } else if (strcmp(digoleCommand, "enableCursor") == 0) {
        enableCursor();
    } else if (strcmp(digoleCommand, "drawStr") == 0) {
        drawStr((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), argv[4]);
    } else if (strcmp(digoleCommand, "drawBitmap") == 0) {
        //drawBitmap((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]), (uint8_t)atoi(argv[2]));
    } else if (strcmp(digoleCommand, "drawBitmapc") == 0) {
        //drawBitmapc((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[2]), const uint8_t *bitmap,uint8_t c );
    } else if (strcmp(digoleCommand, "print") == 0) {
        print(argv[2]);
    } else if (strcmp(digoleCommand, "printxy") == 0) {
        printxy((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), argv[4]);
    } else if (strcmp(digoleCommand, "printxy_abs") == 0) {
        printxy_abs((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), argv[4]);
    } else if (strcmp(digoleCommand, "printxyf_abs") == 0) {
        printxyf_abs((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]),(uint8_t)atoi(argv[4]), argv[5]) ;
    } else if (strcmp(digoleCommand, "printxyf") == 0) {
        printxyf((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]),(uint8_t)atoi(argv[4]), argv[5]);
    } else if (strcmp(digoleCommand, "printnxyf") == 0) {
        printnxyf((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]),(uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]));
    } else if (strcmp(digoleCommand, "printnxyf_abs") == 0) {
        printnxyf_abs((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]),(uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]));
    } else if (strcmp(digoleCommand, "LCDdrawbitmap") == 0) {
        //LCDdrawbitmap((uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3]), (uint8_t)atoi(argv[4]), (uint8_t)atoi(argv[5]), (uint8_t)atoi(argv[6]),(uint8_t)atoi(argv[7]),(uint8_t)argv[8]);
    } else if (strcmp(digoleCommand, "clear-day") == 0) {
        drawBitmap256(10, 10, 128, 128, &clearDay,0);
    } else if (strcmp(digoleCommand, "clear-night") == 0) {
        drawBitmap256(10, 10, 128, 128, &clearNight,0);
    } else if (strcmp(digoleCommand, "cloudy") == 0) {
        drawBitmap256(10, 10, 128, 128, &cloudy,0);
    } else if (strcmp(digoleCommand, "fog") == 0) {
        drawBitmap256(10, 10, 128, 128, &fog,0);
    } else if (strcmp(digoleCommand, "partly-cloudy-day") == 0) {
        drawBitmap256(10, 10, 128, 128, &partlyCloudyDay,0);
    } else if (strcmp(digoleCommand, "partly-cloudy-night") == 0) {
        drawBitmap256(10, 10, 128, 128, &partlyCloudyNight,0);
    } else if (strcmp(digoleCommand, "rain") == 0) {
        drawBitmap256(10, 10, 128, 128, &rain,0);
    } else if (strcmp(digoleCommand, "sleet") == 0) {
        drawBitmap256(10, 10, 128, 128, &sleet,0);
    } else if (strcmp(digoleCommand, "snow") == 0) {
        drawBitmap256(10, 10, 128, 128, &snow,0);
    } else if (strcmp(digoleCommand, "wind") == 0) {
        drawBitmap256(10, 10, 128, 128, &wind,0);
    }

    printf("\n");
	return 0;
}
