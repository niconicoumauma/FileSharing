#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

#define MSG_LEN 1024

int main(int argc, char *argv[])
{

/*▼ファイル操作関連*/
	FILE *fpw;
	char *dir_ini = "./media/";	//fopenで使用
	char *dir_name = "media";	//ディレクトリ作成用
	char dir[MSG_LEN];
	char fname_w[MSG_LEN];
	
	
/*△ファイル操作関連*/

	struct hostent *server_ent;
	struct sockaddr_in server;
	int soc;
	char hostname[1000];
	int in_port;

	in_port = atoi(argv[2]);
	strcpy(hostname, argv[1]);

	if((server_ent = gethostbyname(hostname)) == NULL){
		perror("gethostbyname");
		exit(1);
	}

	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(in_port);
	memcpy((char *)&server.sin_addr, server_ent->h_addr, server_ent->h_length);

	if((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}

	if(connect(soc, (struct sockaddr *)&server, sizeof(server)) == -1){
		perror("connect");
		exit(1);
	}

	//write(1, "CONNECTED\n", 10);
	//printf("=======================================\n");

	int n;
	char msg[MSG_LEN];//text送信
	int file_size;
	int file_split_num;
	int file_split_mod;
	long i, SPLIT_SIZE;
	int rcvd_size = 0;

//=====================================-

	/*モード選択(download, upload)*/
	printf("==============SELECT MODE==============\n");
	printf("1. DOWNLOAD\n");
	printf("2. UPLOAD\n");
	printf("> ");
	memset(msg, MSG_LEN, '\0');
	scanf("%s", msg);
	//ここで選択sndするとファイル名指定中に送信される

	if(msg[0]=='1'){
	/*1. DOWNLOAD*/

		/*▲選択snd*/
		write(soc, msg, MSG_LEN);
		/*▼ファイルリストrcv*/
		memset(msg, MSG_LEN, '\0');
		read(soc, msg, MSG_LEN);
		printf("======FILE LIST BEGIN======\n%s\n=======FILE LIST END=======\n", msg);
		/*▲ファイル名snd*/
		/*ファイル名指定*/
		memset(dir, MSG_LEN, '\0');
		strcpy(dir, dir_ini);
		printf("============ENTER FILE NAME============\n> ");
		memset(fname_w, MSG_LEN, '\0');
		scanf("%s", fname_w);
		strcat(dir, fname_w);
		write(soc, fname_w, MSG_LEN);
		/*▼ファイル情報rcv*/
		memset(msg, MSG_LEN, '\0');
		read(soc, msg, MSG_LEN);
		sscanf(msg, "%ld %d %d", &SPLIT_SIZE, &file_split_num, &file_split_mod);
		file_size = SPLIT_SIZE * file_split_num + file_split_mod;
		/*ファイルオープン*/
		fpw = fopen(dir, "wb" );//"a+b"
		if( fpw == NULL ){
			//ディレクトリが存在しない場合
			mkdir(dir_name, 00400 | 00200 | 00100);
			fpw = fopen(dir, "wb" );
			if( fpw == NULL ){
				printf( "ERROR : ACCESS DENIED\n");
				exit(1);
			}
		}

		printf("受信中: %3d/%3d", 0, 100);
		/*▼バッファrcv*/
		/*ファイル書き込み*/
		for(i=0; i<file_split_num; i++){
			/*バッファに書き込み*/
			unsigned char buf[SPLIT_SIZE];
			rcvd_size += read(soc, buf, SPLIT_SIZE);
			/*バッファを書き出し*/
			fwrite(buf, sizeof(unsigned char), SPLIT_SIZE, fpw);
			fseek(fpw, SPLIT_SIZE*(i+1L), SEEK_SET);
			printf("\r受信中: %3d/%3d", (int)(((float)rcvd_size/(float)file_size)*100), 100);
		}
		unsigned char buf[SPLIT_SIZE];
		rcvd_size += read(soc, buf, SPLIT_SIZE);
		fwrite(buf, sizeof(unsigned char), file_split_mod, fpw);

		printf("\r受信中: %3d/%3d\n", (int)(((float)rcvd_size/(float)file_size)*100), 100);

		fclose(fpw);
	}else if(msg[0]=='2'){
	/*1. UPLOAD*/
	}else{
	}



//=====================================-

	close(soc);



}
