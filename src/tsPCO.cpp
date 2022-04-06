#include "tsPCO.h"

const char tb[3][3]={"ns","us","ms"};
const char tmode[4][20]={"Auto","SW-Trig","Ext-Exp. Start","Ext-Exp. Ctrl"};


int shift;
CPCODisp* Cdispwin;

CPco_Log mylog("pco_camera_grab.log");

CPco_grab_clhs *threadGrab;
CPco_grab_clhs *grabber;
CCambuf Cbuf[BUFNUM];

DWORD grab_single(CPco_grab_clhs* grabber,void* picbuf)
{
 int err;
 int picnum;
 unsigned int w,h,l;
 int min,max,v;
 WORD* adr;


 err=grabber->Acquire_Image(picbuf);
 if(err!=PCO_NOERROR)
  printf("\ngrab_single Acquire_Image error 0x%x\n",err);
 else
 {
  grabber->Get_actual_size(&w,&h,NULL);

  picnum=image_nr_from_timestamp(picbuf,shift);
  printf("grab_single done successful, timestamp image_nr: %d\n",picnum);

  max=0;
  min=0xFFFF;
  adr=(WORD*)picbuf;
  l=w*20; //skip first lines with timestamp
  for(;l<w*h;l++)
  {
   v=*(adr+l);
   if(v<min)
    min=v;
   if(v>max)
    max=v;
  }
  printf("grab_single pixels min_value: %d max_value %d\n",min,max);
 }

 return err;
}

DWORD get_image(CPco_grab_clhs* grabber,char* filename,WORD Segment,DWORD ImageNr)
{
 int err;
 unsigned int w,h,l;
 int ima_num; 
 WORD *picbuf;

 grabber->Get_actual_size(&w,&h,NULL);
 picbuf=(WORD*)malloc(w*h*sizeof(WORD));
 if(picbuf==NULL)
 {
  printf("\nget_image cannot allocate buffer\n");
  return PCO_ERROR_NOMEMORY | PCO_ERROR_APPLICATION;
 }

 err=grabber->Get_Image(Segment,ImageNr,picbuf);
 if(err!=PCO_NOERROR)
  printf("\nget_image Acquire_Image error 0x%x\n",err);

 if(err==PCO_NOERROR)
 {
  ima_num=image_nr_from_timestamp(picbuf,0);
  printf("\nget_image done successful, timestamp image_nr: %d\n",ima_num);
 
  if(filename!=NULL)
  {
   char *txt;
   int min,max,v;

   max=0;
   min=0xFFFF;
   l=w*20; //skip first lines with timestamp
   for(;l<w*h;l++)
   {
    v=*(picbuf+l);
    if(v<min)
     min=v;
    if(v>max)
    max=v;
   } 
   printf("get_image pixels min_value: %d max_value %d\n",min,max);

   do
   {
    txt=strchr(filename,'.');
   } 
   while((txt)&&(strlen(txt)>4));

   if(txt==NULL)
   {
    txt=filename;
    strcat(txt,".b16");
   }

   if(strstr(txt,"b16"))
   {
    store_b16(filename,w,h,0,picbuf);
    printf("b16 image saved to %s\n",filename);
   }
   else if(strstr(txt,"tif"))
   {
    store_tif(filename,w,h,0,picbuf);
    printf("tif image saved to %s\n",filename);
   }
  }
 }

 free(picbuf);

 return err;
}

DWORD grab_count_single(CPco_grab_clhs* grabber,int ima_count,CPCODisp* Cdispwin,CCambuf* Cbuf)
{
 int err,imanum;
 unsigned int width,height;
 int firstnum,lastnum,currentnum;
 int numoff;
 double time,freq;

 time=freq=1;

 grabber->Get_actual_size(&width,&height,NULL);

 err=PCO_NOERROR;

 numoff=0;
 firstnum=1;

 imanum=0;
 do
 {
  err=grabber->Acquire_Image(Cbuf->Get_actadr());
  if(err!=PCO_NOERROR)
  {
   printf("\ngrab_loop Acquire_Image error 0x%x\n",err);
   break;
  }
  
  imanum++;
  if(imanum==1)
   mylog.start_time_mess();

  currentnum=image_nr_from_timestamp(Cbuf->Get_actadr(),0);
  if(imanum==1)
   firstnum=currentnum;
  Cdispwin->convert();

  printf("imanum %06d currentnum %06d camera_count %06d\r",imanum,currentnum,currentnum-firstnum+1);
  if(ima_count<=20)
   printf("\n");
  else
   printf("\r");
  fflush(stdout);

//for testing only lost images can also be seen at summary output
  if((currentnum-firstnum-numoff+1)!=imanum)
  {
   numoff++;
//   printf("\ngrab_loop Error number: camera %d grabber %d offset now %d\n",currentnum-firstnum+1,imanum,numoff);
  }


 }while(imanum<ima_count);

 time=mylog.stop_time_mess();

 printf("\n");

 if(err==PCO_NOERROR)
 {
  printf("Grab %d Images done (imanum %d) missed %d\n",ima_count,imanum,numoff);
  lastnum=image_nr_from_timestamp(Cbuf->Get_actadr(),0);
  freq=(imanum-1)*1000;
  freq/=time;
  printf("%05d images grabbed time %dms freq: %.2fHz %.2fMB/sec \n",imanum,(int)time,freq,(freq*width*height*2)/(1024*1024));

  freq=(lastnum-firstnum)*1000;
  freq/=time;
  printf("Imagenumbers: first %d last %d count %d freq: %.2fHz\n",firstnum,lastnum,lastnum-firstnum+1,freq);

  Cdispwin->convert();
 }

 fflush(stdout);

 return PCO_NOERROR;
}


DWORD grab_count_wait(CPco_grab_clhs* grabber,int count)
{
 int err,i,timeout;
 int picnum,buf_nr,first_picnum,lost;
 unsigned int w,h,bp;
 WORD *picbuf[BUFNUM];
 double tim,freq;

 picnum=1;
 err=grabber->Get_actual_size(&w,&h,&bp);
 if(err!=PCO_NOERROR)
 {
  printf("\ngrab_count_wait Get_actual_size error 0x%x\n",err);
  return err;
 }
 printf("\n");
 lost=first_picnum=0;

 grabber->Get_Grabber_Timeout(&timeout);

 memset(picbuf,0,sizeof(WORD*)*BUFNUM); 
 for(i=0;i< BUFNUM;i++)
 {
  picbuf[i]=(WORD*)malloc(w*h*sizeof(WORD));
  if(picbuf[i]==NULL)
  {
   printf("\ngrab_count_wait cannot allocate buffer %d\n",i);
   err=PCO_ERROR_NOMEMORY | PCO_ERROR_APPLICATION;
   break;
  }
 }
 if(err!=PCO_NOERROR)
 {
  for(i=0;i< BUFNUM;i++)
  {
   if(picbuf[i]!=NULL)
    free(picbuf[i]);
  }
  return err;
 }

 err=grabber->Start_Acquire(); 
 if(err!=PCO_NOERROR)
 {
  printf("\ngrab_count_wait Start_Acquire error 0x%x\n",err);
  return err;
 }

 for(i=0;i<count;i++)
 {
  buf_nr=i%BUFNUM;
  err=grabber->Wait_For_Next_Image(picbuf[buf_nr],timeout);
  if(err!=PCO_NOERROR)
  {
   printf("\nWait_For_Next_Image error 0x%x\n",err);
   break;
  }
  else
  {
   picnum=image_nr_from_timestamp(picbuf[buf_nr],0);
   printf("%05d. Image to %d  ts_nr: %05d",i+1,buf_nr,picnum);
   if(i==0)
   {
    first_picnum=picnum;
    mylog.start_time_mess();
   }
   else if((first_picnum+i)!=picnum)
   {
    printf(" %05d != %05d\n",first_picnum+i,picnum);
    first_picnum=picnum-i;
    lost++;
   }

   if((count<=10)||(i<3))
    printf("\n");
   else
    printf("\r");
   fflush(stdout);
  }
 }
 i--;
 tim=mylog.stop_time_mess();
 freq=i*1000;
 freq/=tim;
 printf("\n %05d images grabbed in %dms lost images %d\n",i+1,(int)tim,lost);
 printf(" freq: %.2fHz time/pic: %.3fms  %.2fMB/sec",freq,tim/i,(freq*w*h*2)/(1024*1024));

 err=grabber->Stop_Acquire(); 
 if(err!=PCO_NOERROR)
 {
  printf("\ngrab_count_wait Stop_Acquire error 0x%x\n",err);
 }

 for(i=0;i< BUFNUM;i++)
 {
  if(picbuf[i]!=NULL)
   free(picbuf[i]);
 }

 return err;
}


int image_nr_from_timestamp(void *buf,int shift)
{
  unsigned short *b;
  int y;
  int image_nr=0;
  b=(unsigned short *)(buf);

  y=100*100*100;
  for(;y>0;y/=100)
  {
   *b>>=shift;
   image_nr+= (((*b&0x00F0)>>4)*10 + (*b&0x000F))*y;
   b++;
  }
  return image_nr;
}


void get_number(char *number,int len)
{
   int ret_val;
   int x=0;

   while(((ret_val=getchar())!=10)&&(x<len-1))
   {
    if(isdigit(ret_val))
     number[x++]=ret_val;
   }
   number[x]=0;
}

void get_text(char *text,int len)
{
   int ret_val;
   int x=0;

   while(((ret_val=getchar())!=10)&&(x<len-1))
   {
    if(isprint(ret_val))
     text[x++]=ret_val;
   }
   text[x]=0;
}

void get_hexnumber(int *num,int len)
{
  int ret_val;
  int c=0;
  int cmd=0;
  while(((ret_val=getchar())!=10)&&(len > 0))
  {
   if(isxdigit(ret_val))
   {
    if(ret_val<0x3A)
     cmd=(ret_val-0x30)+cmd*0x10;
    else if(ret_val<0x47)
     cmd=(ret_val-0x41+0x0A)+cmd*0x10;
    else
     cmd=(ret_val-0x61+0x0A)+cmd*0x10;
    len--;
    c++;
   }
  }
  if(c>0)
   *num=cmd;
  else
   *num=-1;
}

// void pcoReturnErrorMessage(DWORD errorValue){

// }

std::string printErrorMessage(DWORD errorValue)
{
  char *cstr = new char[1000];
  std::string errorOut;
  PCO_GetErrorText(errorValue, cstr, 1000);
  
  errorOut = cstr;
  std::cerr << errorOut << std::endl;

    return errorOut;
}
