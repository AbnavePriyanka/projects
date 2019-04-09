using namespace std;

#include<stdio.h>
#include<stdlib.h>//maloc
#include<fcntl.h>//sys call
#include<string.h>
#include<iostream>//cin cout
#include<sys/types.h>
#include<sys/stat.h>//structure
#include<unistd.h>
#include<memory.h>
//#include<io.h>

//macro
#define READ 1
#define WRITE 2
#define CURRENT 1//setoofset
#define START 0//setoffset
#define END 2
#define TRUE 1
#define FALSE 0

typedef int BOOL;

class filewrap
{
	public:
		char fname[80];
		int fd;
		int fmode;

		filewrap(const char *,int);//parameterized constructor with default value
		filewrap(filewrap&);	   //copy constructor
		~filewrap();		   // destructor
		long int getoffset();
		void setoffset(int,int);
		BOOL writefile(const char *);
		BOOL writefile(const char *,int);//data offset
		char * readfile(int);
		char * readfile(int,int);
		friend BOOL operator ==(filewrap &,filewrap &);
		friend void operator +(filewrap &,filewrap &);
		void GetInfo();
};

filewrap::filewrap(const char* name,int mode=READ+WRITE)//constructor
{
	strcpy(fname,name);//copy
	fmode=mode;//mode
	if(fmode==READ)
	{
		fd=open(fname,O_RDONLY);
	}
	else if(fmode==WRITE)
	{
		fd=open(fname,O_WRONLY);
	}
	else
	{
		fd=open(fname,O_RDWR);
	}
	if(fd==-1)//file not open
	{
		fd=creat(fname,0777);//0777 permission read write execute
		if(fd==-1)
		{
			cout<<"ERROR:Unable to create file\n";
			return;
		}
		fmode = READ + WRITE;
	}
}



filewrap::filewrap(filewrap &ref)//copy constructor
{
	char buffer[512];
	int readcnt=0,writecnt=0;
	long int offset=0;

	sprintf(fname,"%s_new",ref.fname);
	fmode=ref.fmode;
	fd=creat(fname,0777);
	
	if(fd==-1)
	{
		cout<<"ERROR: unable to create file\n";
		return;
	}

	offset=lseek(ref.fd,0,1);//offset position where to strt

	lseek(ref.fd,0,0);//get empty file

	while((readcnt=read(ref.fd,buffer,sizeof(buffer)))!=0)
	{
		writecnt=write(fd,buffer,readcnt);//where what howmany
		if(writecnt==-1)
		{
			cout<<"Error:Problem in writting\n";
			break;
		}
	}
	lseek(ref.fd,offset,0);
	lseek(fd,0,0);//new file empty 
}


filewrap::~filewrap()
{
	if(fd!=-1)
	//{
		close(fd);
	//}
}


long int filewrap::getoffset()
{
	long int offset=0;
	offset=lseek(fd,0,1);
	return offset;
}


void filewrap::setoffset(int offset,int from)
{
	lseek(fd,offset,from);
}



BOOL filewrap::writefile(const char *data)
{
	int ret=0;
	if(fmode<2)
	{	
		cout<<"ERROR: mode mismatch";
		return FALSE;
	}
	ret=write(fd,data,strlen(data));
	if(ret==-1)
	{
		return FALSE;
	}	
	else
	{
		return TRUE;
	}
}


char * filewrap::readfile(int size)
{
	char *buffer=NULL;//ptr
	buffer=(char *)malloc(size+1);//alocate size
	
	read(fd,buffer,size);
	
	return buffer;
}


char * filewrap::readfile(int size,int from)//2nd offset fro where to read
{
	char *buffer=NULL;
	buffer=(char *)malloc(size+1);
	
	lseek(fd,from,1);
	read(fd,buffer,size);
	
	return buffer;
}


BOOL filewrap::writefile(const char *data,int size)//hadle for overwr8 data//overloding
{
	int ret=0;
	if(fmode<2||data==NULL||size<=0)
	{
		cout<<"ERROR:mde mismatch";
		return FALSE;
	}
	if(strlen(data)<size)
	{
		ret=write(fd,data,strlen(data));
	}
	else
	{
		ret=write(fd,data,size);
	}
	if(ret==-1)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
	

BOOL operator ==(filewrap &obj1,filewrap &obj2)
{
	char buffer1[512],buffer2[512];
	BOOL flag=TRUE;
	int ret1,ret2;
	long int offset1=0,offset2=0;

	offset1=lseek(obj1.fd,0,1);
	offset2=lseek(obj2.fd,0,1);

	lseek(obj1.fd,0,0);
	lseek(obj2.fd,0,0);

	while(((ret1=read(obj1.fd,buffer1,sizeof(buffer1)))!=0)&&
			((ret2=read(obj2.fd,buffer2,sizeof(buffer2)))!=0))
	{
		if(ret1!=ret2)
		{
			flag=FALSE;
			break;
		}
		else
		{
			if(memcmp(buffer1,buffer2,ret1)!=0)//compare memory area
			{ 		
				flag=FALSE;
				break;
			}
		}
		memset(buffer1,0,sizeof(buffer1));
		memset(buffer2,0,sizeof(buffer2));
	}
	lseek(obj1.fd,offset1,0);
	lseek(obj2.fd,offset2,0);

	if((ret1==0)&&((ret2=read(obj2.fd,buffer2,1)!=0)))//used to check data size
        {
		return FALSE;
		return flag;
	}
}


void operator +(filewrap &obj1,filewrap &obj2)//concate the file
{
	char buffer[512];
	int ret=0;
	if(obj1.fmode<2)
	{
		return;
	}
	lseek(obj1.fd,0,2);//end of file	
	while((ret=read(obj2.fd,buffer,sizeof(buffer)))!=0)
	{
		write(obj1.fd,buffer,ret);
		memset(buffer,0,sizeof(buffer));
	}
}



void filewrap::GetInfo()//this fun only for linux
{
	int ret;
	struct stat obj;
	fstat(fd,&obj);
	cout<<"filesize:"<<(int)obj.st_size<<endl;
	cout<<"NO.of links:"<<(int)obj.st_nlink<<endl;
	cout<<"Inode No:"<<(int)obj.st_ino<<endl;
	cout<<"No of Block:"<<(int)obj.st_blocks<<endl;
	printf((obj.st_mode & S_IRUSR)?"r":"-");//read permission is 4 user
	printf((obj.st_mode & S_IWUSR)?"w":"-");//wrte
	printf((obj.st_mode & S_IXUSR)?"x":"-");//execute
}



int main()
{
	char *ptr=NULL;
	int no;
	filewrap obj1("Hello",READ+WRITE);
	filewrap obj2("new Hello",READ+WRITE);
	filewrap obj3(obj1);

	if(obj1==obj3)
	{
		cout<<"equal";
	}
	else
	{
		cout<<"not equal";
	}
	obj2.writefile("HelloWorld");
	ptr=obj2.readfile(12);
	obj1.GetInfo();
	obj1+obj2;
	cout<<obj1.getoffset();
	return no;
}	
