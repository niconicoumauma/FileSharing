# FileSharing
現在ダウンロード機能しかありません  
サーバ側はマルチスレッドです  
  
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
