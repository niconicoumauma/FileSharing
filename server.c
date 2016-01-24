#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<dirent.h>

#define MSG_LEN 1024
#define SPLIT_SIZE 1000L

void *ThreadMain(void *threadArg);

struct ThreadArgs
{
	int soc;
};

int main(int argc, char *argv[])
{

/*▼ソケット関連*/
	int i;
	//echo server
	struct sockaddr_in me;
	int soc_waiting;
	int soc;
	int in_port = atoi(argv[1]);
	pthread_t threadID;
	struct ThreadArgs *threadArgs;
/*△ソケット関連*/

/*▼コネクション*/
	memset((char *)&me,0,sizeof(me));
	me.sin_family=AF_INET;
	me.sin_addr.s_addr=htonl(INADDR_ANY);
	me.sin_port=htons(in_port);
	if((soc_waiting = socket(AF_INET, SOCK_STREAM, 0)) < 0){/*TCP(&local):SOCK_STREAM, UDP:SOCK_DGRAM*/
		perror("socket");
		exit(1);
	}
	if(bind(soc_waiting, (struct sockaddr *)&me, sizeof(me)) == -1){
		perror("bind");
		exit(1);
	}
	listen(soc_waiting, 1);
/*△コネクション*/

/*▼スレッド生成*/
	while(1){

		/*WAIT*/
		soc = accept(soc_waiting, NULL, NULL);

		/*引数用にメモリを新しく確保*/
		if((threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs))) == NULL){
			fprintf(stderr, "malloc failed\n"), exit(1);
		}
		threadArgs->soc = soc;
		/*スレッドを生成*/
		if(pthread_create(&threadID, NULL, ThreadMain, (void *)threadArgs) != 0){
			fprintf(stderr, "pthread_create() failed\n"), exit(1);
		}
	}
/*△スレッド生成*/


/*▼プロセス生存処理*/
	while(1){
		//printf("MAIN FUNCTION IS ALIVE.\n");
		//sleep(5);
	}
/*△プロセス生存処理*/
}

//SOCの値がどの端末と通信しているのかを識別するものとなるので、threadにSOCを渡さないと通信できない
void *ThreadMain(void *threadArgs)
{
	int soc;
	int n;
	/*▼ファイル操作関連*/
	FILE *fpr;
	char *dir_ini = "./media/";	//fopenで使用
	char dir[MSG_LEN];
	char fname_r[MSG_LEN];
	char msg[MSG_LEN];
	char dir_info[MSG_LEN];
	int file_size;
	int file_split_num;
	int file_split_mod;
	long i;
	/*△ファイル操作関連*/

	/*▼ディレクトリ*/
	memset(dir_info, MSG_LEN, '\0');
	printf("%s\n", dir_info);
	const char *dirname = "./media/";
	struct dirent **namelist;
	int r = scandir(dirname, &namelist, NULL, NULL);
	if(r == -1){
		err(EXIT_FAILURE, "%s", dirname);
	}
	for(i=0; i<r; i++){
		strcat(dir_info, namelist[i]->d_name);
		if(i!=(r-1)){
			strcat(dir_info, "\n");
		}
	}
	printf("%s\n", dir_info);
	free(namelist);
	/*△ディレクトリ*/

	/*戻り値にスレッドのリソース割り当てを解除*/
	pthread_detach(pthread_self());

	/*引数からの値を取り出す*/
	soc = ((struct ThreadArgs *) threadArgs)->soc;
	free(threadArgs);
	printf("SOC: %d, thread_id=%lu\n", soc, pthread_self());

//=====================================-

	/*▼モード選択rcv(download, upload)*/
	memset(msg, MSG_LEN, '\0');
	n = read(soc, msg, MSG_LEN);

	if(msg[0]=='1' && msg[1]=='\0'){
	/*1. DOWNLOAD*/
		/*▲ファイルリストsnd*/
		write(soc, dir_info, MSG_LEN);
		/*▼ファイル名rcv*/
		read(soc, fname_r, MSG_LEN);
		printf("ファイル名受信 : %s\n", fname_r);
		/*ディレクトリ指定*/
		strcpy(dir, dir_ini);
		strcat(dir, fname_r);
		printf("(PATH: %s)\n", dir);
		/*ファイルオープン*/
		fpr = fopen( dir, "rb" );
		if( fpr == NULL ){
			printf( "読込用 %sファイルが開けません\n", fname_r );
		}else{
			/*ファイルサイズ取得, 分割数と余り算出*/
			fseek(fpr, 0, SEEK_END);
			file_size = ftell(fpr);
			fseek(fpr, 0, SEEK_SET);
			file_split_num = file_size / SPLIT_SIZE;
			file_split_mod = file_size % SPLIT_SIZE;
			/*▲ファイル情報（分割サイズ、分割数、余り）snd*/
			memset(msg, MSG_LEN, '\0');
			sprintf(msg, "%ld %d %d", SPLIT_SIZE, file_split_num, file_split_mod);
			write(soc, msg, MSG_LEN);
			/*ファイルをバッファに格納しサイズを取得*/
			/*▲バッファsnd*/
			for(i=0; i<file_split_num; i++){
				/*バッファに書き込み*/
				unsigned char buf[SPLIT_SIZE];
				fread(buf, sizeof(unsigned char), SPLIT_SIZE, fpr);
				//ファイルポインタ移動
				fseek(fpr, SPLIT_SIZE*(i+1L), SEEK_SET);
				write(soc, buf, SPLIT_SIZE);
			}
			unsigned char buf[SPLIT_SIZE];
			fread(buf, sizeof(unsigned char), file_split_mod, fpr);
			write(soc, buf, SPLIT_SIZE);

			printf("SENT TO SOC%d\n",soc);
			fclose(fpr);
			
		}
	}

//=====================================-

	//while(1){
	//}
	//return (NULL);
}
