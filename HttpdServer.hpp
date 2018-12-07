#ifndef __HTTPD_SERVER_HPP__
#define __HTTPD_SERVER_HPP__

#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "Log.hpp"

class HttpdServer{
		private:
				int listen_sock;
				int port;
		public:
				HttpdServer(int port_):port(port_),listen_sock(-1)
		{}
				void InitServer()
				{
						listen_sock = socket(AF_INET,SOCK_STREAM,0);//创建监听套接字
						if(listen_sock < 0){
								LOG(ERROR,"create socket error");
								exit(2);//合理规划退出码
						}
						int opt_ = 1;//设置端口复用，快速重启服务器
						setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt_, sizeof(opt_) );//即便进入TIME_WAIT也能立即重启

						struct sockaddr_in local_;
						local_.sin_family = AF_INET;
						local_.sin_port = htons(port);
						local_.sin_addr.s_addr = INADDR_ANY;//无符号长整型的宏

						if(bind(listen_sock,(struct sockaddr*)&local_,sizeof(local_))< 0 ){
								LOG(ERROR,"bind socket error");
								exit(3);
						}
						if(listen(listen_sock,5)<0){
								LOG(ERROR,"listen socket error");
								exit(4);
						}
						LOG(INFO,"initServer success!");
				}
				void Start()
				{
						LOG(INFO,"Start Server begin");
						for(;;){
								struct sockaddr_in peer_;
								socklen_t len = sizeof(peer_);
								int sock_ = accept(listen_sock,(struct sockaddr*)&peer_,&len);
								if(sock_ < 0)
								{
										LOG(WARNING,"accpet error");
										continue;
								}
								//链接成功，创建线程，交给线程去执行
								LOG(INFO,"Get New Client ,Create THread Handler Request..");								
								pthread_t tid_;
								int *sockp_ = new int;
								*sockp_ = sock_;
								pthread_create(&tid_,NULL,Entry::HandlerRequest,&sock_);
						}

				}

				~HttpdServer()
				{
						if(listen_sock != -1){
								close(listen_sock);
						}
						port =-1;
				}
};
#endif
