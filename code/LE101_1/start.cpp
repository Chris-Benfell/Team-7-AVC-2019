
/* Trying remote compilig on RPI */

#include <stdio.h>
#include <unistd.h>
#include "camera.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <bcm2835.h>

#define CAMERA_WIDTH 320 //Control Resolution from Camera
#define CAMERA_HEIGHT 240 //Control Resolution from Camera
// change in bpp for new version
// of RPI firmware: 
#define FB_MAX_SIZE 9000000

//int hardware_exchange();

// camera stuff
CCamera *cam;  // camera instance
//int  Y_row[CAMERA_WIDTH];
unsigned char pixels_buf[CAMERA_WIDTH*CAMERA_HEIGHT*4];

int disp_level = 0;

// camera image converted to fit display depth - somewhat big for 1960x1050 screen
char cam_disp[FB_MAX_SIZE];
// original image on the screen
char original[FB_MAX_SIZE];
int fbfd = 0;
struct fb_var_screeninfo orig_vinfo;
char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize;
int res;

// hardware
unsigned char servo[5];
unsigned char adc[5];
unsigned char gpi[5];
unsigned char gpo[5];


// hardware
//int spi_h; // handle of SPI
int sock;  // socket handle

/********************EXCHANGE*********/
int hardware_exchange()
{
	int il;
	
	 for ( il = 0 ; il < 5 ; il++){             
        adc[il] = bcm2835_spi_transfer(servo[il]);
        if (disp_level>0){
            printf("   Servo[%d]: %d. adc[%d]: %d\n",
                   il,servo[il], il,adc[il]);
		}
	  }
	  
      for ( il = 0 ; il < 5 ; il++){             
        gpi[il] = bcm2835_spi_transfer(gpo[il]);
        if ( disp_level>0){
          printf("   gpo[%d]: %d. gpi[%d]: 0x%02X.\n", il,gpo[il], il,gpi[il]);
         } 
      }
      bcm2835_delay(10);
    return 0;
}



void stoph()
{
	printf("stop called\n");
    bcm2835_spi_end();
    bcm2835_close();
}

int init(int deb)
{
	// stop on Ctrl_C
    //signal(2,stop);
    disp_level = deb;

	if (!bcm2835_init()){
		printf("Init failed\n");
		return 1;
	}
	
	if (!bcm2835_spi_begin())
    {
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
      return 1;
    }
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4096); // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0); 
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default


	cam = StartCamera(CAMERA_WIDTH, CAMERA_HEIGHT,30,1,true);
    if (cam == NULL)
    {
        printf(" Camera initialization failure\n");
        return -2;
    }
    servo[0] = 48;
    servo[1] = 48;
    servo[2] = 48;
    servo[3] = 48;
    servo[4] = 48;
    hardware_exchange();
    sleep(1);
    return 0;
}





/*******CAMERA FUNCTIONS****************/

// takes camera picture and stores it into mybuff
int take_picture()
{
   if (disp_level>0)
   {
      printf("Taking camera picture\n");
   }
   cam->ReadFrame(0,&(pixels_buf[0]),sizeof(pixels_buf));
   usleep(10000);  //just in case
   if (disp_level>0)
   {
      printf("Picture taken\n");
   }

   return 0;
}

int save_picture(char fn[5])
{
          //save image into ppm file
        FILE *fp;
        char fname[9];
        sprintf(fname,"%s.ppm",fn);
        if (disp_level>0)
        {
           printf("Saving picture. %s\n",fname);
        }
        fp = fopen(fname,"wb");
        if ( !fp)
        {
           printf("Unable to open the file\n");
           return -1;
        }
        // write file header
        fprintf(fp,"P6\n %d %d %d\n",CAMERA_WIDTH , CAMERA_HEIGHT,255);
        int ind = 0;
        int row = 0;
        int col = 0;
        char red;
        char green;
        char blue;
        for ( row = 0 ; row < CAMERA_HEIGHT; row++)
        {
           for ( col = 0 ; col < CAMERA_WIDTH; col++)
           {
		     red =  pixels_buf[ind];
		     green =  pixels_buf[ind+1];
		     blue =  pixels_buf[ind+2];
		     fprintf(fp,"%c%c%c",red,green,blue);
		     ind = ind + 4;
           }
        }
        fflush(fp);
        fclose(fp);
        //system("timeout 1s gpicview im1.ppm");
        return 0;

}


// returns color component (color==0 -red,color==1-green,color==2-blue
// color == 3 - luminocity
// for pixel located at
unsigned char get_pixel( int row,int col, int color)
{
    // calculate address in 1D array of pixels
    int address = CAMERA_WIDTH*row*4 + col*4;
    if ((row < 0 ) || (row > CAMERA_HEIGHT) )
    {
        printf("row is out of range\n");
        return -1;
    }
    if ( (col< 0) || (col > CAMERA_WIDTH))
    {
        printf("column is out of range\n");
        return -1;
    }


    if (color==0)
    {
        return (pixels_buf[address]);
    }
    if (color==1)
    {
        return (pixels_buf[address + 1]);
    }
    if (color==2)
    {
        return (pixels_buf[address + 2]);
    }
    if (color==3)
    {
		int yi = (pixels_buf[address] + pixels_buf[address+1] +pixels_buf[address+2]);
        unsigned char y = yi/3;
        return y;
    }
    printf("Color encoding wrong: 0-red, 1-green,2-blue,3 - luminosity\n");
    return -2; //error
}

int set_pixel(int row, int col, char red, char green,char blue)
{
    int address = CAMERA_WIDTH*row*4 + col*4;
    if ((address < 0) || (address>CAMERA_WIDTH*CAMERA_HEIGHT*4))
    {
        printf("SetPixel(): wrong x,y coordinates\n");
        return -1;
    }
    pixels_buf[address] = red;
    pixels_buf[address+1] = green;
    pixels_buf[address+2] = blue;
    return 0;
}

// takes camera picture and converts it
// into format for frame buffer (2 Bytes per pixel)
// fill
void convert_camera_to_screen()
{
  int x;
  int y;
  int x_offset =10;
  int y_offset =10;
  // take snapshot of original screen
  memcpy(&(cam_disp[0]),fbp,screensize);
  for ( x = 0 ; x < CAMERA_WIDTH ; x++)
   {
      for ( y = 0 ; y < CAMERA_HEIGHT; y++)
       {
		  int r = get_pixel(y,x,0);
		  int g = get_pixel(y,x,1);
		  int b = get_pixel(y,x,2);
		  if (vinfo.bits_per_pixel == 16) {
                //put_pixel_RGB565(x+x_offset, y+y_offset, r, g, b);
                // calculate the pixel's byte offset inside the buffer
                // note: x * 2 as every pixel is 2 consecutive bytes
                unsigned int pix_offset = (x+x_offset) * 2 +( y+y_offset) * finfo.line_length;
                // now this is about the same as 'fbp[pix_offset] = value'
                // but a bit more complicated for RGB565
                //unsigned short c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);
                unsigned short c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
                // write 'two bytes at once'
                *((unsigned short*)(cam_disp + pix_offset)) = c;
            }
            else {
                // calculate the pixel's byte offset inside the buffer
                // note: x * 3 as every pixel is 3 consecutive bytes
                // original
               // unsigned int pix_offset = (x+x_offset) * 3 + (y+y_offset) * finfo.line_length;
                unsigned int pix_offset = (x+x_offset) * 4 + (y+y_offset) * finfo.line_length;
                // now this is about the same as 'fbp[pix_offset] = value'
                // modified for 32 bits
                *((char*)(cam_disp + pix_offset)) = (unsigned char)b;
                *((char*)(cam_disp + pix_offset + 1)) = (unsigned char)g;
                *((char*)(cam_disp + pix_offset + 2)) = (unsigned char)r;
                *((char*)(cam_disp + pix_offset + 3)) = 255;
                
                // original code
                //*((char*)(cam_disp + pix_offset)) = b;
                //*((char*)(cam_disp + pix_offset + 1)) = g;
                //*((char*)(cam_disp + pix_offset + 2)) = r;
                
                //put_pixel_RGB24(x+x_offset, y+y_offset, r, g, b);
            }
       }
    }

}


int open_screen_stream()
{
    //char *fbp = 0;
   //struct fb_var_screeninfo vinfo;
   //struct fb_fix_screeninfo finfo;

   
    fbfd = 0;
    //struct fb_var_screeninfo orig_vinfo;
    screensize = 0;
    res=-1;
    // camera image converted to fit display depth - somewhat big for 1960x1050 screen
    //char cam_disp[3528000];
    // original image on the screen
    //char original[3528000];

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
      printf("Error: cannot open framebuffer device: %s\n",strerror(errno));
      return(1);
    }
    if (disp_level>0)
    {
      printf("The framebuffer device was opened successfully.fbfd=%d\n",fbfd);
    }

    // Get variable screen information
    res = ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
    if (res == -1) {
      printf("Error reading variable information: %s\n",strerror(errno));
    }
    if (disp_level>0)
    {
      printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres,
                                        vinfo.bits_per_pixel );
    }
    // Store for reset (copy vinfo to vinfo_orig)
    memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
      printf("Error reading fixed information.\n");
    }

    // map fb to user mem
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    if (disp_level>0)
    {
       printf("screensize=%ld\n",screensize);
    }
    if ( screensize>FB_MAX_SIZE)
    {
        printf("Not enough memory to store the screen\n");
        return -1;
    }
    fbp = (char*)mmap(0,
              screensize,
              PROT_READ | PROT_WRITE,
              MAP_SHARED,
              fbfd,
              0);

    if ((int)fbp == -1) {
        printf("Failed to mmap display.\n");
        return -1;
    }

    return 0;
}

int close_screen_stream()
{
   munmap(fbp, screensize);
   if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
        printf("Error re-setting variable information.\n");
        return -1;
   }
   close(fbfd);
   return 0;
}

int update_screen()
{
   if ((int)fbp == -1) {
        printf("UpdateScreen() - no screen buffer handle\n");
        return -1;
   }
   //  memcpy(&(cam_disp[0]),fbp,screensize);
   // modify cam_disp with camera image
   convert_camera_to_screen(); //
    // copy original sceen fb to "original" buffer
   //memcpy(&(original[0]),fbp,screensize);
   // original screen -> cam_disp
   //memcpy(&(cam_disp[0]),fbp,screensize);
   // put cam_disp to screen
   memcpy(fbp,&(cam_disp[0]),screensize);
   usleep(3000);
   return 0;
}



int display_picture(int delay_sec,int delay_usec)
{
   open_screen_stream();
   if (disp_level>0)
   {
      printf("open_screen_stream() OK\n");
   }

   // copy original sceen fb to "original" buffer
   memcpy(&(original[0]),fbp,screensize);
   // original screen -> cam_disp
  // memcpy(&(cam_disp[0]),fbp,screensize);
   // update camera portion of the screen
   update_screen();
   if (disp_level>0)
   {
      printf("update_screen() OK\n");
   }

   sleep(delay_sec);
   usleep(delay_usec);
   // put original image back
   memcpy(fbp,&(original[0]),screensize);
   close_screen_stream();
   // cleanup

   return 0;

}



/*********************MOTOR*************************/
// sets motor speed and direction
int set_motors(unsigned char nm,unsigned char sp)
{
	if (( nm < 1) || (nm>5)){
		printf("Motor number wrong. %d\n",nm);
		return -1;
	}
	if ((sp<30)||(sp>65)){
		printf("motor speed out of range\n");
		return -1;
	}
	servo[nm-1] = sp;
    return 0;
}

// sets motor speed and direction
int set_digital(unsigned char no,unsigned char val)
{
	if (( no < 0) || (no>4)){
		printf("set digital: output number wrong. %d\n",no);
		return -1;
	}
	if (!((val==1)||(val==0))){
		printf("set digital: output value should be 0 or 1\n");
		return -1;
	}
	gpo[no] = val;
    return 0;
}

int read_digital(unsigned char no)
{
	if (( no < 0) || (no>4)){
		printf("Read digital: output number wrong. %d\n",no);
		return -1;
	}
    return gpi[no];
}


int read_analog(int in_ch_adc){
	if (( in_ch_adc < 0) || (in_ch_adc>4)){
		printf("read_analog: ADC channel number wrong. %d\n",in_ch_adc);
		return -1;
	}
    return adc[in_ch_adc];
}

// sleeps for number of milliseconds
int sleep1(int msec)
{
    bcm2835_delay(msec);
    return 0;
}




/*********************************/
/****************NETWORK**********/
/*********************************/

int connect_to_server( char server_addr[15],int port)
{
    int res;
    struct sockaddr_in server;

    if (disp_level>0)
    {
        printf("Connecting to %s \n",server_addr);
    }
    //char server_addr[15] = {'1','2','7','.','0','0','1','.','0','0','1','.','0','0','1'};
    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == -1)
    {
        printf("Could not create socket\n");
        res = -1;
        return res;
    }

    server.sin_addr.s_addr = inet_addr(server_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock,(struct sockaddr *)&server,sizeof(server))<0)
    {
        printf("Connection to %s failed\n",server_addr);
        return -2;
    }

    if (disp_level>0)
    {
        printf("Connection to %s established\n",server_addr);
    }
    return 0;
}

int send_to_server(char message[24])
{
    if (sock>0)
    {
        if (send(sock , message , strlen(message) , 0) < 0)
        {
            printf("Sending %s failed\n",message);
            return -1;
        }
    } else {
        printf("No connection\n");
    }
    return 0;
}

int receive_from_server(char message[24])
{
    if (sock>0){
        if (recv(sock,message,2000,0) <0) {
            printf("Receive failed\n");
        }
    } else {
        printf("No connection\n");
    }
    return 0;
}

