# FileSharing

This is the software that has been made in the C language.  
課題で提出するため1月中はクライアントソフトのみの公開となります。  
  
動作確認済みのOS  
Ubuntu14, CentOS6  
※Windowsでは動きません  
  
USAGE  
SERVER  
$ gcc server.c -o server -pthread
$ ./server [port number]
CLIENT
$ gcc client.c -o client
$ ./client [ip address] [port number]
