#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>
#define X_SIZE 4000 
#define Y_SIZE 3000 

/* here are our X variables */
Display *display;
int screen,a;
int XS=X_SIZE*4;
int MS=X_SIZE*4*Y_SIZE;
int JS=X_SIZE*3*Y_SIZE;
Window win;
GC gc;
XImage *x_image;
unsigned char *x_buffer;
int SQUARES ;

struct squares {
	int considered;
	int r;
	int g;
	int b;
	int x1;
	int x2;
	int x3;
	int x4;
	int y1;
	int y2;
	int y3;
	int y4;
	int yp;
	int xp;
	int angle;
	int w;
	int h;
	float l;
	float theta;
};


/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();
void place(struct squares *);
void allocate(struct squares *);
int n_score(struct squares *,struct squares *);
int testline (int,int,int,int,int,int,int,int);
int recalc(struct squares *);
void recopy(struct squares *, struct squares *, int);

int square();
void plot( int, int, int,int,int);
void line (int,int,int,int,int,int,int);
void fillup (struct squares *);
void clear();

/* Jpegs */
int read_JPEG_file (char *, unsigned char *, int *);
int jayit(unsigned char *,int, int, char *);

void usage ()
{
	printf("usage: rect seed [1-100000] squares [1-10000] \n");
	exit (1);
}

int main(int argc,char *argv[])
{
	int seed;
	char jayfile[100],*jay;
	if (argc<3){ usage();}
	seed=atoi(argv[1]);
	SQUARES=atoi(argv[2]);
	if (seed<1 || SQUARES<1 ){ usage();}
	srand(seed);

	int option,ss,rss,st,tscore,trys,frame;
	init_x();
	char dummy[10];

	struct squares *sq;
	struct squares *tr;
	int *values;
	frame=0;

	sq=(struct squares *)malloc(sizeof (struct squares) * SQUARES);
	tr=(struct squares *)malloc(sizeof (struct squares));
	values=(int *)malloc(sizeof (int)*SQUARES);
	jay=(char *)malloc(sizeof (char)*JS);

	// First allocate the images
	//for (SQUARES=50;SQUARES<300;SQUARES+=10)
	//{	
	clear();
	allocate(sq);
	for (ss=0;ss<SQUARES;ss++)
	{
		//fillup(sq+ss);
	printf("usage: rect seed [1-100000] squares [1-10000] \n");
		place(sq+ss);
	}

	tscore=0;
	for (st=0;st<SQUARES;st++)
	{
		int rts;
		rts=0;
		for (ss=0;ss<SQUARES;ss++)
		{
			if (ss==st){ continue;}
			rss=n_score(sq+st,sq+ss);
			rts+=rss;
			tscore+=rss;
		}
		values[st]=rts;
		printf ("%d rts %d tscore %d\n",st,rts,tscore/2);
	}
	//a=180;
	a=rand()%X_SIZE;
	for (trys=0;trys<190000000;trys++)
	{
	if (tscore==0){ continue;}
	//XPutImage(display, win, gc, x_image, 0, 0, 0, 0, X_SIZE, Y_SIZE);
	//scanf("%c",dummy);

	int worst,worstone;
	worst=0;
	for (st=0;st<SQUARES;st++)
	{
		if (values[st]>worst ){
			if ((sq+st)->considered==0)
			{
				worst=values[st];
				worstone=st;
			}
		}
	}
	// nothing considered
	if ( worst==0)
	{
		a--;if(a<1){a=180;}
		printf("reset \n");
        	for (st=0;st<SQUARES;st++)
        	{
                	(sq+st)->considered=0;
                	if (values[st]>worst ){
                                worst=values[st];
                                worstone=st;
                        }
                }
        }

	a=rand()%X_SIZE/2;
	//a++;if ( a=){a=X_SIZE;}
        int options[10];
	int bestscore;
        // test the options
	
        for ( option=0;option<10;option++)
        {
	        recopy(sq+worstone,tr,option);
		if (recalc(tr)==1) {
            	    int rts;
                	rts=0;
                	for (ss=0;ss<SQUARES;ss++)
                	{
                        	if (ss==worstone){ continue;}
                        	rss=n_score(tr,sq+ss);
                        	rts+=rss;
	        		//recopy(tr,sq+worstone,10);
                	}
                	printf ("option %d worst %d rts %d \n",option,worstone,rts);
			options[option]=rts;
		}else {
                	printf ("option %d out of bounds\n",option);
			options[option]=-1;
		}

	}

	int goodones[10];
	int best,good,choice;
	best=values[worstone];
        for ( option=0;option<10;option++)
	{
		if (options[option]==-1 ){ continue;}
		if (options[option]< best){ best=options[option];}
	}
	good=0;
	if (best<values[worstone]) { 
        	for ( option=0;option<10;option++)
		{
			if (options[option]==best){ printf ("best %d\n",option);
			goodones[good]=option;
			good++;
			}
		}
	}else{
		printf ("Cant improve \n");
	}
	//if (good==0){ a--; if (a<1){a=180;(sq+worstone)->considered=1;}}
	if (good==0){ (sq+worstone)->considered=1;}
	else if (good==1){ choice=goodones[0];}else{ choice=goodones[rand()%good];}
	
	printf ("Choice %d  a %d \n",choice,a);
	if (good>0)
	{
		tscore=tscore+best-values[worstone];
		values[worstone]=best;
	        recopy(sq+worstone,tr,choice);
	        recopy(tr,sq+worstone,10);
		recalc(sq+worstone);
		// display
		clear();
        	for (ss=0;ss<SQUARES;ss++)
        	{
			//fillup(sq+ss);
                	place(sq+ss);
        	}
		XPutImage(display, win, gc, x_image, 0, 0, 0, 0, X_SIZE, Y_SIZE);
		int p=0;
		for (ss=0;ss<MS;ss+=4)
		{
			jay[p++]=x_buffer[ss+2];
			jay[p++]=x_buffer[ss+1];
			jay[p++]=x_buffer[ss];
		}
		sprintf(jayfile,"./jpegs/image%04d.jpg",frame);
		jayit(jay,X_SIZE,Y_SIZE,jayfile);
		
		frame++;

	}
	//scanf("%c",dummy);
	}
	//}
	scanf("%c",dummy);
}

void fillup (struct squares *sq)
{
	//use the rasta to find the squares and then line to do the overlay compute
	char *rasta;
	rasta=(char *)malloc(sizeof (char)*X_SIZE*Y_SIZE);


	float along,xt,yt,xb,yb,dx,dy,mm,dex,dwhy;
	float l,k;
	int xn,yn,x,y;
	dx=sq->x2-sq->x1;
	dy=sq->y2-sq->y1;
	l=sqrt((dx*dx)+(dy*dy));
	dex=sq->x2-sq->x3;
	dwhy=sq->y2-sq->y3;
	k=sqrt((dex*dex)+(dwhy*dwhy));
        for (x=0;x<X_SIZE*Y_SIZE;x++)
        {
        	rasta[x]=0;
        }
	for (along=0;along<=l;along+=0.6)
	{
		xt=(sq->x1)+((along*dx)/l);
		yt=(sq->y1)+((along*dy)/l);
		xb=(sq->x4)+((along*dx)/l);
		yb=(sq->y4)+((along*dy)/l);
        	for (mm=0;mm<=k;mm+=0.6)
        	{
                	xn=xb+(dex*mm/k);
                	yn=yb+(dwhy*mm/k);
                	rasta[xn+(yn*X_SIZE)]=1;
		}
        }
	for (x=0;x<X_SIZE;x++)
	{
		for (y=0;y<Y_SIZE;y++)
		{
			if (rasta[x+(y*X_SIZE)]==1){
			plot(x,y,sq->r,sq->g,sq->b) ;}
		}

	}
	free(rasta);
}

int n_score(struct squares *can,struct squares *agn)
{
	int total,dr,dg,db;
	total=0;
	total+=testline (can->x1,can->y1,can->x2,can->y2,agn->x1,agn->y1,agn->x2,agn->y2);
	total+=testline (can->x2,can->y2,can->x3,can->y3,agn->x1,agn->y1,agn->x2,agn->y2);
	total+=testline (can->x3,can->y3,can->x4,can->y4,agn->x1,agn->y1,agn->x2,agn->y2);
	total+=testline (can->x4,can->y4,can->x1,can->y1,agn->x1,agn->y1,agn->x2,agn->y2);

	total+=testline (can->x1,can->y1,can->x2,can->y2,agn->x2,agn->y2,agn->x3,agn->y3);
	total+=testline (can->x2,can->y2,can->x3,can->y3,agn->x2,agn->y2,agn->x3,agn->y3);
	total+=testline (can->x3,can->y3,can->x4,can->y4,agn->x2,agn->y2,agn->x3,agn->y3);
	total+=testline (can->x4,can->y4,can->x1,can->y1,agn->x2,agn->y2,agn->x3,agn->y3);

	total+=testline (can->x1,can->y1,can->x2,can->y2,agn->x3,agn->y3,agn->x4,agn->y4);
	total+=testline (can->x2,can->y2,can->x3,can->y3,agn->x3,agn->y3,agn->x4,agn->y4);
	total+=testline (can->x3,can->y3,can->x4,can->y4,agn->x3,agn->y3,agn->x4,agn->y4);
	total+=testline (can->x4,can->y4,can->x1,can->y1,agn->x3,agn->y3,agn->x4,agn->y4);

	total+=testline (can->x1,can->y1,can->x2,can->y2,agn->x4,agn->y4,agn->x1,agn->y1);
	total+=testline (can->x2,can->y2,can->x3,can->y3,agn->x4,agn->y4,agn->x1,agn->y1);
	total+=testline (can->x3,can->y3,can->x4,can->y4,agn->x4,agn->y4,agn->x1,agn->y1);
	total+=testline (can->x4,can->y4,can->x1,can->y1,agn->x4,agn->y4,agn->x1,agn->y1);

	dr=can->r-agn->r;
	dg=can->g-agn->g;
	db=can->b-agn->b;

	total++;

	total=total*(sqrt((dr*dr)+(dg*dg)+(db*db)));

	return total;

	

}

int testline(int x1, int y1, int x2, int y2, int tx1, int ty1, int tx2, int ty2)
{
	char dummy[10];
	//printf ("trying %d %d %d %d  against %d %d %d %d \n",x1,x2,y1,y2,tx1,tx2,ty1,ty2);
	//line(x1,y1,x2,y2,255,0,0);
	//line(tx1,ty1,tx2,ty2,0,255,0);
	int dx,dxa,dy,dya;
	int X;

	dx=x2-x1;dxa=tx2-tx1;
	dy=y2-y1;dya=ty2-ty1;

	// do the x planes match?
	//scanf("%c",dummy);

	if (dx>=0 && dxa>=0) {
		if (x1>tx2 || x2<tx1 ) {
			return 0;
		}
	} else if (dx<=0 && dxa>=0){
		if (x2>tx2 || x1<tx1 )
		{
			return 0;
		}
	} else if (dx>=0 && dxa<=0){
		if (x1>tx1 || x2<tx2 ) {
			return 0;
		}
	} else if (dx<=0 && dxa<=0){
		if (x2>tx1 || x1<tx2 ) {
			return 0; 
		}
	} else {
		printf ("uncaght x exception \n");
	}

        if (dy>=0 && dya>=0) {
                if (y1>ty2 || y2<ty1 ) {
                        return 0;
                }
        } else if (dy<=0 && dya>=0){
                if (y2>ty2 || y1<ty1 )
                {
                        return 0;
                }
        } else if (dy>=0 && dya<=0){
                if (y1>ty1 || y2<ty2 ) {
                        return 0;
                }
	} else if (dy<=0 && dya<=0){
                if (y2>ty1 || y1<ty2 ) {
                        return 0;
		}
        } else {
		printf ("uncaght y exception \n");
        } 


	int b;
	b=(dx*dya)-(dxa*dy);
	if (b != 0)
	{	
		X=((dx*dxa*y1)-(dxa*dy*x1)-(dx*dxa*ty1)+(dx*dya*tx1))/b;
	}else{
		if ( ((y2-ty1)*dx) == ((x2-tx1)*dy) )
		{
               		printf ("intersecting parallel\n");
			return 40;
		} else {
               		printf ("parallel\n");
			return 0;
		}
	}

	//printf ("?? Intersect at %f x1 %d x2 %d\n",X,x1,x2);
	if (dx>=0 && dxa>=0) {
		if ( X>=x1 && X<=x2 && X>=tx1 && X<=tx2 )
		{
			return 1;
		}
	} else if (dx>=0 && dxa<=0) {
		if ( X>=x1 && X<=x2 && X>=tx2 && X<=tx1 )
		{
			return 1;
		}
	}else if (dx<=0 && dxa>=0) {
		if ( X>=x2 && X<=x1 && X>=tx1 && X<=tx2 )
		{
			return 1;
		}
	}else {
		if ( X>=x2 && X<=x1 && X>=tx2 && X<=tx1 )
		{
			return 1;
		}
	}
	return 0;
}

void recopy(struct squares *in, struct squares *out, int tweak)
{
        out->angle=in->angle;
        out->considered=in->considered;
        out->r=in->r;
        out->g=in->g;
        out->b=in->b;
        out->xp=in->xp;
        out->yp=in->yp;
        out->x1=in->x1;
        out->x2=in->x2;
        out->x3=in->x3;
        out->x4=in->x4;
        out->y1=in->y1;
        out->y2=in->y2;
        out->y3=in->y3;
        out->y4=in->y4;
        out->yp=in->yp;
        out->xp=in->xp;
        out->w=in->w;
        out->h=in->h;
        out->l=in->l;
        out->theta=in->theta;

      switch(tweak) {
      case 0 :
        //printf("down one\n" );
        out->yp+=a;
	break;
     case 1 :
        //printf("up one\n" );
        out->yp-=a;
        break;
     case 2 :
        //printf("right one\n" );
        out->xp+=a;
        break;
     case 3 :
        //printf("left one\n" );
        out->xp-=a;
        break;
     case 4 :
        //printf("twist clock\n" );
        out->angle+=a;
        break;
     case 5 :
        //printf("twist anti-clock\n" );
        out->angle-=a;
        break;
     case 6 :
        //printf("diagonal down left\n" );
        out->xp-=a;
        out->yp-=a;
	break;
     case 7 :
        //printf("diagonal up right\n" );
        out->xp+=a;
        out->yp+=a;
	break;
     case 8 :
        //printf("diagonal down right\n" );
        out->xp-=a;
        out->yp+=a;
	break;
     case 9 :
        //printf("diagonal down right\n" );
        out->xp+=a;
        out->yp-=a;
	break;
     case 10 :
        //printf("locking in\n" );
        break;
     default :
         printf("Invalid control\n" );
	 break; }
}	

void line (int x1,int y1,int x2,int y2, int r, int g, int b)
{
	int x,y,w,h,len,xp,yp,xn,yn;
	w=x2-x1;
	h=y2-y1;
	len=4*sqrt((w*w)+(h*h));
	for (x=0;x<=len;x++)
	{
		xn=x1+(w*x/len);
		yn=y1+(h*x/len);
		plot(xn,yn,r,g,b);
	}
}	

void clear()
{
	int x;
	for (x=0;x<X_SIZE*Y_SIZE*4;x++)
	{
		x_buffer[x]=255;
	}
}

void plot(int x,int y,int r,int g,int b)
{
	int p;
	p=(4*x)+(y*XS);

	x_buffer[p]=(x_buffer[p]*b)/255;
	x_buffer[p+1]=(x_buffer[p+1]*g)/255;
	x_buffer[p+2]=(x_buffer[p+2]*r)/255;

}	

int recalc(struct squares *sq)
{
	int w,h,angle,x1,x2,x3,x4,y1,y2,y3,y4,xp,yp;
	float l,theta;

	w=sq->w;
	h=sq->h;
      	angle=sq->angle;
	l=sq->l;
	theta=sq->theta;
	xp=sq->xp;
	yp=sq->yp;


        x1=xp+(l*((float)w*cos(theta+((2*M_PI*angle)/360))));
        x2=xp+(l*((float)w*cos(((2*M_PI*angle)/360))));
        x3=xp-(l*((float)w*cos(theta+((2*M_PI*angle)/360))));
        x4=xp-(l*((float)w*cos(((2*M_PI*angle)/360))));

        y1=yp+(l*((float)w*sin(theta+((2*M_PI*angle)/360))));
        y2=yp+(l*((float)w*sin(((2*M_PI*angle)/360))));
        y3=yp-(l*((float)w*sin(theta+((2*M_PI*angle)/360))));
        y4=yp-(l*((float)w*sin(((2*M_PI*angle)/360))));


        if (!(x1<X_SIZE && x2<X_SIZE && x3<X_SIZE && x4<X_SIZE && x1>=0 && x2 >=0 && x3 >=0 && x4>=0 &&
        y1<Y_SIZE && y2<Y_SIZE && y3<Y_SIZE && y4<Y_SIZE && y1>=0 && y2 >=0 && y3 >=0 && y4>=0))
        {
		return 0;
        }
	sq->x1=x1;
	sq->x2=x2;
	sq->x3=x3;
	sq->x4=x4;
	sq->y1=y1;
	sq->y2=y2;
	sq->y3=y3;
	sq->y4=y4;
	return 1;
}

void allocate(struct squares *sq)
{
        int loop,xp,yp,x1,y1,x2,y2,x3,y3,x4,y4,w,h,angle,r,g,b,poss,cposs;
    	float l,theta;
        for (loop=0;loop<SQUARES;loop++)
        {
                poss=0;
		cposs=0;
                while (poss==0 || cposs==0)
                {
			poss=0;cposs=0;
			r=56+rand()%200;
			g=56+rand()%200;
			b=56+rand()%200;

			if ((((r-g)*(r-g))+((b-r)*(b-r))+((g-b)*(g-b)))>40000){cposs=1;}


                	w=(2+rand()%((X_SIZE-2)/4));
                	h=(2+rand()%((Y_SIZE-2)/4));
			//w=2;h=2;
                	angle=rand()%179;
                	xp=rand()%X_SIZE;
                	yp=rand()%Y_SIZE;

                	l=sqrt((h*h)+(w*w));
                	theta=2*atan((float)w/(float)h);

                	x1=xp+(l*((float)w*cos(theta+((2*M_PI*angle)/360))));
                	x2=xp+(l*((float)w*cos(((2*M_PI*angle)/360))));
                	x3=xp-(l*((float)w*cos(theta+((2*M_PI*angle)/360))));
                	x4=xp-(l*((float)w*cos(((2*M_PI*angle)/360))));

                	y1=yp+(l*((float)w*sin(theta+((2*M_PI*angle)/360))));
                	y2=yp+(l*((float)w*sin(((2*M_PI*angle)/360))));
                	y3=yp-(l*((float)w*sin(theta+((2*M_PI*angle)/360))));
                	y4=yp-(l*((float)w*sin(((2*M_PI*angle)/360))));



                	if (x1<X_SIZE && x2<X_SIZE && x3<X_SIZE && x4<X_SIZE &&  x4<X_SIZE &&  x4<X_SIZE &&  x4<X_SIZE && x1>=0 && x2 >=0 && x3 >=0 && x4>=0 &&
                	y1<Y_SIZE && y2<Y_SIZE && y3<Y_SIZE && y4<Y_SIZE &&  y4<Y_SIZE &&  y4<Y_SIZE &&  y4<Y_SIZE && y1>=0 && y2 >=0 && y3 >=0 && y4>=0)
                	{
                        	poss=1;
                	}
		}
                sq[loop].r=r;
                sq[loop].g=g;
                sq[loop].b=b;
		sq[loop].x1=x1;
		sq[loop].x2=x2;
		sq[loop].x3=x3;
		sq[loop].x4=x4;
		sq[loop].y1=y1;
		sq[loop].y2=y2;
		sq[loop].y3=y3;
		sq[loop].y4=y4;
		sq[loop].yp=yp;
		sq[loop].xp=xp;
		sq[loop].w=w;
		sq[loop].h=h;
		sq[loop].angle=angle;
		sq[loop].considered=0;
		sq[loop].l=l;
		sq[loop].theta=theta;
	}
}

void place(struct squares *sq)
{
	line(sq->x1,sq->y1,sq->x2,sq->y2,sq->r,sq->g,sq->b);
	line(sq->x2,sq->y2,sq->x3,sq->y3,sq->r,sq->g,sq->b);
	line(sq->x3,sq->y3,sq->x4,sq->y4,sq->r,sq->g,sq->b);
	line(sq->x4,sq->y4,sq->x1,sq->y1,sq->r,sq->g,sq->b);
}	

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

GLOBAL(int)
read_JPEG_file (char * filename, unsigned char * dots, int * params)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.txt for more info.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */

  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    memcpy (dots+(row_stride*cinfo.output_scanline),buffer[0],row_stride);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    /* put_scanline_someplace(buffer[0], row_stride); */

  }
  /* Step 7: Finish decompression */
  params[0]=cinfo.output_width;
  params[1]=cinfo.output_height;
  params[2]=cinfo.output_components;

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  /* And we're done! */
  return 1;
}

int jayit(unsigned char *screen,int image_width, int image_height, char *name)
{

int row_stride,ex,why,cmp,div,set;
unsigned char *image,**row_pointer,*cr,*cg,*cb;
row_pointer=(unsigned char **)malloc(1);

struct jpeg_compress_struct cinfo;
struct jpeg_error_mgr jerr;
FILE * outfile;		/* target file */
cinfo.err = jpeg_std_error(&jerr);
jpeg_create_compress(&cinfo);
if ((outfile = fopen(name, "wb")) == NULL) { 
	fprintf(stderr, "can't open file\n");
	exit(1);
}
jpeg_stdio_dest(&cinfo, outfile);
cinfo.image_width = image_width; 	/* image width and height, in pixels */
cinfo.image_height = image_height;
cinfo.input_components = 3;		/* # of color components per pixel */
cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
jpeg_set_defaults(&cinfo);
jpeg_set_quality(&cinfo,100,TRUE); /* limit to baseline-JPEG values */
jpeg_start_compress(&cinfo, TRUE);

  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & screen[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
jpeg_finish_compress(&cinfo);
fclose(outfile);
jpeg_destroy_compress(&cinfo);
}

void init_x()
{
/* get the colors black and white (see section for details) */
        unsigned long black,white;

        x_buffer=(unsigned char *)malloc(sizeof(unsigned char)*4*X_SIZE*Y_SIZE);
        display=XOpenDisplay((char *)0);
        screen=DefaultScreen(display);
        black=BlackPixel(display,screen),
        white=WhitePixel(display,screen);
        win=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0,
                X_SIZE, Y_SIZE, 5, white,black);
        XSetStandardProperties(display,win,"image","images",None,NULL,0,NULL);
        gc=XCreateGC(display, win, 0,0);
        XSetBackground(display,gc,black); XSetForeground(display,gc,white);
        XClearWindow(display, win);
        XMapRaised(display, win);
        //XMoveWindow(dis, win,window_x,100);
        Visual *visual=DefaultVisual(display, 0);
        x_image=XCreateImage(display, visual, DefaultDepth(display,DefaultScreen(display)), ZPixmap, 0, x_buffer, X_SIZE, Y_SIZE, 32, 0);
};

void close_x() {
        XFreeGC(display, gc);
        XDestroyWindow(display,win);
        XCloseDisplay(display);
        exit(1);
};

void redraw() {
        XClearWindow(display, win);
};


