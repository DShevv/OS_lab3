#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<alloca.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<libgen.h>
#include<limits.h>
#include<errno.h>
#include<unistd.h>
#include<dirent.h>


char *AppName;
sig_atomic_t chldcnt = 0, mxchldcnt = 0;
#define BUF_SIZE 1024*1024

/*unsigned char countpls(unsigned char n){
	unsigned char res = 0;
	while(n){
		res += n&1;
		n >>= 1;
	}
	return res;

}
int checkbit(const int value, const int position){
	int res;
	if ((value & (1 << position)) == 0){
		res = 0;
	} else {
		res = 1;	
	}
	
	return res;
}
*/
const char *byte_to_bin(int x){
	static char b[9];
	b[0]='\0';
	for(int z = 128; z>0;z>>=1)
		strcat(b,((x&z)==z)?"1":"0");
	return b;
}


int WC(char *path){
	int f = open(path, O_RDONLY);
	if(f == -1){
		fprintf(stderr, "%s: %s. File: %s\n", AppName, strerror(errno), path);
		exit(-1);
	}
	char flag=-1;
	unsigned w=0, b=0;
	unsigned char buf[BUF_SIZE], help[16], bb;
	char raz[BUF_SIZE];
	int cnt = 0;
	int arr1[9] = 
{
/*  0  1  2  3  4  5  6  7  8  */
    0, 0, 0, 0, 0, 0, 0, 0, 0
};
	int arr0[9] = 
{
/*  0  1  2  3  4  5  6  7  8  */
    0, 0, 0, 0, 0, 0, 0, 0, 0
};
	while((cnt = read(f, &buf, sizeof(BUF_SIZE))) != 0){
		if(cnt == -1){
			fprintf(stderr, "%s: %s. File: %s\n", AppName, strerror(errno), path);
			exit(-1);
		}
		int con = 1;
		for(int i = 0;i<cnt;i++){
			char hlpstr[16];
			strcpy(hlpstr,byte_to_bin((int)buf[i]));
			if (i == 1)
				strcpy(help,hlpstr);
			int count = 1;
			for(int j = 0; j < 8; j++){
				
				if (j!=7){
					if(hlpstr[j] == hlpstr[j+1]){
						count++;
					}
					else {
						if (hlpstr[j]=='1')
						arr1[count]++;
						if (hlpstr[j]=='0')						
						arr0[count]++;
						count = 1;					
					}
				}
				else
					if (hlpstr[j]!=hlpstr[j-1]){
						if (hlpstr[j]=='1')
						arr1[count]++;
						if (hlpstr[j]=='0')						
						arr0[count]++;
						
						count = 1;		
					}
					else {
						count++;
						if (hlpstr[j]=='1')
						arr1[count]++;
						if (hlpstr[j]=='0')						
						arr0[count]++;
						count = 1;
						
					}
			}
			b++;	
		}
		
	}
	

	printf("%d %s %u\n 0 - 1: %d 2: %d 3: %d 4: %d 5: %d 6: %d 7: %d 8: %d \n 1 - 1: %d 2: %d 3: %d 4: %d 5: %d 6: %d 7: %d 8: %d \n",(int)getpid(), path, b, arr0[1], arr0[2], arr0[3], arr0[4], arr0[5], arr0[6], arr0[7], arr0[8], arr1[1], arr1[2], arr1[3], arr1[4], arr1[5], arr1[6], arr1[7], arr1[8] );//// РІС‹РІРѕРґ	
	

	if(close(f) == -1){
		fprintf(stderr, "%s: %s. File: %s\n", AppName, strerror(errno), path);
		exit(-1);
	}
	exit(0);
}

void WORK(char *curPath){
	DIR *curDir;
	if((curDir = opendir(curPath)) == NULL) {
		fprintf(stderr,"%s: %s. File: %s\n", AppName, strerror(errno), curPath);
		errno = 0;
		return;
	}
	struct dirent *dent;
	char *file = alloca(strlen(curPath) + NAME_MAX + 2);
	if(file==NULL){
		fprintf(stderr,"%s: %s.", AppName, strerror(errno));
		return;
	}
	errno = 0;
	while(dent = readdir(curDir)){
		if(strcmp(".", dent->d_name) && strcmp("..", dent->d_name)){
			strcpy(file, curPath);
			if(strcmp(file,"/")!=0)
				strcat(file, "/");
 			strcat(file, dent->d_name);
			struct stat buf;
			if (lstat(file,&buf) == -1) {
						fprintf(stderr,"%s: %s. File: %s\n", AppName, strerror(errno), curPath);
						return;
					}
			if(S_ISDIR(buf.st_mode)){
				WORK(file);
			}
			int c;
			if(S_ISREG(buf.st_mode)){
	 			if(chldcnt==mxchldcnt) {
	 				wait(&c);
	 				chldcnt--;
	 			}
				switch(fork()){
					case (pid_t)-1:
						fprintf(stderr,"%s: %s.\n", AppName, strerror(errno));
						errno = 0;
						break;
					case (pid_t)0:
						WC(file);
						break;
					default:
						chldcnt++;
						break;
				}
			}
		}
	}
	if(errno!=0){
		fprintf(stderr,"%s: %s. File: %s\n", AppName, strerror(errno), curPath);
		errno = 0;
		return;
	}

	if(closedir(curDir)==-1){
		fprintf(stderr,"%s: %s. File: %s\n", AppName, strerror(errno), curPath);
		errno = 0;
		return;
	}
	return;
}

int main(int argc, char**argv){
	AppName = alloca(strlen(basename(argv[0])));
	strcpy(AppName, basename(argv[0]));

	if(argc<2){
		fprintf(stderr, "%s: Too few arguments\n", AppName);
		return -1;
	}

	mxchldcnt = atoi(argv[2]);
	if(mxchldcnt<=0){
		fprintf(stderr, "%s: Second argument is wrong\n", AppName);
		return -1;
	}

	char *res;
	if((res = realpath(argv[1], NULL)) == NULL) {
		fprintf(stderr, "%s: %s. File: %s.\n", AppName, strerror(errno), argv[1]);
		return -1;
	}


	WORK(res);

	while (wait(NULL) != -1) { }

	return 0;
}