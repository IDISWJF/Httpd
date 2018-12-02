#ifndef __PROTOCOL_UTTL_HPP__
#define __PROTOCOL_UTTL_HPP__
#include<iostream>
#include<string>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include"Log.hpp"
class Request{
		public:
				std::string rq_line;
				std::string rq_head;
				std::string blank;
				std::string rq_text;
		public:
				Request():blank("\n")
		{}
				~Request()
				{}
};
class Response{
		private:
				std::string rsp_line;
				std::string rsp_head;
				std::string blank;
				std::string rsp_text;
		public:
				Response():blank("\n")
		{}
				~Response()
				{}
};
class Connect{
		private:
				int sock;
		public:
				Connect(int sock_):sock(sock_)
		{}
				int RecvOneLine(std::string &line_)
				{
						char c = 'x';//一次读一个字符,且必须要初始化，否则随机值可能是\n就不会循环
						while(c != '\n')
						{
								ssize_t s = recv(sock,&c,1,0);
								if(s > 0)
								{//\n,\r\n,\r区分
										if(c == '\r'){
												recv(sock,&c,1,MSG_PEEK);//MSG_PEEK标志会将套接字接收队列中的可读的数据拷贝到缓冲区，
												//但不会使套接子接收队列中的数据减少，
												//常见的是：例如调用recv或read后，导致套接字接收队列中的数据被读取后而减少，
												//而指定了MSG_PEEK标志可通过返回值获得可读数据长度，并且不会减少套接字接收缓冲区中的数据
												//所以可以供程序的其他部分继续读取。
												if(c == '\n'){
														recv(sock,&c,1,0);
												}
												else{
														c = '\n';
												}											
										}
										line_.push_back(c);

								}
								else{
										break;
								}
						}
						return line_.size();
				}
		~Connect()
				{
						if(sock >= 0)
								close(sock);
				}

};
class Entry{
		public:
				static void *HandlerRequest(void *arg_)
				{
						int sock_ = *(int*)arg_;
						delete (int*)arg_;
						Connect *conn_ = new Connect(sock_);
						Request *rq_ = new Request();
						Response *rsp_ = new Response();
						
                        conn_-> RecvOneLine(rq_->rq_line);
						std::cout<<rq_->rq_line<<std::endl;
						
                        delete conn_;
						delete rq_;
						delete rsp_;
				}
};
#endif
