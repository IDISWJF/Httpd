#ifndef __PROTOCOL_UTIL_HPP__
#define __PROTOCOL_UTIL_HPP__

#include"Log.hpp"
#include<iostream>
#include<sstream>
#include<string>
#include<string.h>
#include<unordered_map>
#include<strings.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/sendfile.h>

#define NOT_FOUND 404
#define OK 200

#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define HTTP_VERSON "http/1.0"

std::unordered_map<std::string, std::string> stuffix_map
{
		{".html","text/html"},
				{".htm", "text/html"},
				{".css", "text/css"},
				{".jc", "application/x-javascript"}
};

class ProtocolUtil//协议工具
{
		public:
				static void Makekv(std::unordered_map<std::string,std::string> &kv_, std::string &str_)
				{
						std::size_t pos = str_.find(": ");
						if(std::string::npos == pos)
						{
								return ;
						}
						std::string k_ = substr(0,pos);
						std::string v_ = substr(pos + 2);

						kv_.insert( make_pair(k_,v_) );
				}
				static std::string IntToString(int code)//整形转成字符串
				{
						std::stringstream ss;
						ss << code;
						return ss.str();
				}
				static std::string CodeToDesc(int code)
				{
						switch(code)
						{
								case 200:
										return "OK";
								case 404:
										return "NOT FOUND";
								default:
										return "UNKNOW";
						}
				};
				std::string SuffixToType(const std::string &suffix_)//后缀转类型
				{
						return stuffix_map[suffix_];
				}
};

class Request{
		public:
				std::string rq_line;//请求行：方法，URI,HTTP/版本
				std::string rq_head;//请求报头
				std::string blank;//空行
				std::string rq_text;//请求正文
		private:
				std::string method;//方法
				std::string uri;//同一资源标识符
				std::string version;//版本
				bool cgi;//method=POST,GET含有？说明是cgi

				std::string path;//要访问资源的路径
				std::string param;//参数


				int resource_size;//资源大小
				std::string resource_suffix;
				std::unordered_map<std::string,std::string> head_kv;
				int content_length;
		public:
				Request():blank("\n"), cgi(false),path(WEB_ROOT),resource_size(0),content_length(-1),resource_suffix(".html")
		{}
				int GetResourceSize()
				{
						return resource_size;
				}
				std::string &GetParam()
				{
						return param;
				}				
				std::string &GetSuffix()		
				{
						return resource_suffix;
				}						
				std::string &GetPath()
				{
						return path;
				}				
				void RequestOneLineParse()//请求行解析
				{
						std::stringstream ss(rq_line);//stringstream类同时可以支持c风格的串流输入输出操作
						ss >> method >> uri >> version;//字符串按顺序输出
				}
				void UriParse()//分析URI
				{
						if(strcasecmp(method.c_str(), "GET") == 0)
						{
								std::size_t pos_ = uri.find('?');
								if(std::string::npos != pos_)//判断是否是get方式，如果是是否携带参数
								{
										cgi = true;
										path += uri.substr(0,pos_);//从0开始截，截取pos个字符
										param = uri.substr(pos_ + 1);//越过‘？’截取后面字符串
								}
								else
								{
										path += uri;
								}
						}
						else{
								path += uri;
						}

						if(path[path.size() - 1] == '/')//如果路径最后是‘\’在后面加上后缀index.html
						{
								path += HOME_PAGE;
						}
				}
				bool RequestHeadParse()//请求报头解析
				{
						int start = 0;
						while( start < rq_head.size() )
						{
								std::size_t pos = rq_head.find("\n",start);//每个子串以"\n"结尾，每次从start开始找
								if(std::string::npos == pos)
								{
										break;
								}
								std::string sub_string_ = rq_head.substr(start,pos - start);//找到所有的子串
								if( !sub_string_.empty() )
								{//如果不为空，报头子串就是键值对<key:value>的形式存放
										ProtocolUtil::Makekv(head_kv,sub_string);
								}
								else
								{
										break;
								}
								start = pos + 1;//从"\n"下一个开始找
						}
				}
				int GetContentLength()
				{//如果是post方法，报头中描述了正文长度，防止正文中‘\n’干扰，无法完整读取造成粘包问题
						std::string cl_ = head_kv["Content-Length"];//获取报头键值对中的正文长度
						if(!cl_.empty())
						{
								std::string stringstream ss(cl_);//将string类型的正文长度转换成整形长度
								ss >> content_length;
						}
						return content_length;
				}
				bool IsMethodLegal()//判断方法是否合法
				{//只要是get、post忽略大小写
						if(strcasecmp(method.c_str(),"GET") == 0 || \
										(cgi = (strcasecmp(method.c_str(),"POST") == 0)) ) 
								//strcasecmp：忽略大小写的比较，c的函数所以将method必须转成c风格
						{
								return true;
						}
						return false;
				}
				bool IsPathLegal()//判断是否是合法路径
				{
						struct stat st;//第一个参数必须是c风格的字符串
						if( stat(path.c_str(),&st) < 0)//通过path获取文件信息，并保存在st所指向的结构体中
						{
								LOG(WARNING,"path not found");
								return false;
						}

						if( S_ISDIR(st.st_mode) )//如果是目录返回默认首页
						{
								path += "/";
								path += HOME_PAGE;
						}
						else
						{
								if( (st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH) )
										cgi = true;
						}
						resource_size = st.st_size;//读取到的资源长度
						std::size_t pos = path.rfind(".");
						if(std::string::npos != pos )
						{
								resource_suffix = path.substr(pos);
						}
						return true;			
				}
				bool IsNeedRecvText()//判断是否需要读正文
				{
						if( strcasecmp(method.c_str(),"POST") )
						{
								return true;
						}
						return false;
				}
				bool IsCgi()
				{
						return cgi;
				}
				~Request()
				{}
};
class Response{
		public:
				std::string rsp_line;//HTTP/版本，状态码，状态码描述符
				std::string rsp_head;//响应报头
				std::string blank;//空行
				std::string rsp_text;//响应正文
		public:
				int code;
				int fd;	
		public:
				Response():blank("\n"), code(OK),fd(-1)
		{}
				void MakeStatusLine()
				{
						rsp_line = HTTP_VERSON;
						rsp_line += " ";
						rsp_line += ProtocolUtil::IntToString(code);
						rsp_line += " ";
						rsp_line += ProtocolUtil::CodeToDesc(code);
						rsp_line += "\n";
				} 
				void MakeResponseHead(Request *&rq_)
				{
						rsp_head = "Content-Length: ";
						rsp_head += ProtocolUtil::IntToString(rq_->GetResourceSize() );
						rsp_head += "\n";
						rsp_head += "Content-Type: ";//type：通过后缀找到对应的类型，
						std::string suffix_ = rq_->GetSuffix();//在Content-Type对照表中(html:text/html)(css:text/css)
						rsp_head += ProtocolUtil::SuffixToType(suffix_);
						rsp_head += "\n";
				}
				void OpenResource(Request *&rq_)
				{
						std::string path_ = rq_->GetPath();
						fd = open( path_.c_str(),O_RDONLY );   
				}
				~Response()
				{
						if(fd >= 0)
						{
								close(fd);
						}
				}
};
class Connect
{
		private:
				int sock;
		public:
				Connect(int sock_):sock(sock_)
		{}
				int RecvOneLine(std::string &line_)//读一行
				{
						char c = 'x';//一次读一个字符,且必须要初始化，否则随机值可能是\n就不会循环
						while(c != '\n')
						{
								ssize_t s = recv(sock,&c,1,0);
								if(s > 0)
								{// 区分 \n,\r\n,\r 都代表一个意思，所以将三者都替换从\n
										if(c == '\r')
										{
												recv(sock,&c,1,MSG_PEEK);//MSG_PEEK标志会将套接字接收队列中的可读的数据拷贝到缓冲区，
												//但不会使套接子接收队列中的数据减少，
												//常见的是：例如调用recv或read后，导致套接字接收队列中的数据被读取后而减少，
												//而指定了MSG_PEEK标志可通过返回值获得可读数据长度，并且不会减少套接字接收缓冲区中的数据
												//所以可以供程序的其他部分继续读取。
												if(c == '\n'){//如果是\r\n,在接收一次而不push，\r\n=\n
														recv(sock,&c,1,0);
												}
												else{
														c = '\n';//如果走到这一步，说明是\r ,直接替换成\n
												}											
										}
										line_.push_back(c);
								}
								else
								{
										break;
								}
						}
						return line_.size();
				}
				void RecvRequestHead(std::string &head)//接收请求报头
				{
						std::string line_;
						head_ =" ";
						while( line_ != "\n" )//循环读取，放在rq_head直到遇见空行
						{
								line_ = "";
								RecvOneLine(line_);
								head_ += line_;
						}
				}
				void RecvRequestText(std::string &text_ ,int len_, std::string &param_)
				{
						char c_ = " ";
						int i_ = 0;
						while(i_ < len_--)
						{
								recv(sock, &c_ ,1 ,0);
								text_.push_back(c_);
						}
						param_ = text_;
				}
				void SendResponse(Response *&rsp_,Request *&rq_, bool cgi_)
				{
						if(cgi_)
						{}
						else
						{
								std::string &rsp_line_ = rsp_->rsp_line;
								std::string &rsp_head_ = rsp_->rsp_head;
								std::string &blank_ = rsp_->blank;
								int fd = &rsp_->fd;

								send(sock, rsp_line_.c_str(), rsp_line_.size(), 0);
								send(sock, rsp_head_.c_str(), rsp_head_.size(), 0);
								send(sock, blank_.c_str(), blank_.size(), 0);
								sendfile( sock, fd, NULL, rq_->GetResourceSize() );//发送文件的资源

						}
				}
				~Connect()
				{
						if(sock >= 0)
								close(sock);
				}

};
class Entry{
		public:
				static int ProcessNonCgi(Connect *&conn_, Request *&rq_, Response *&rsp_ )
				{
						int &code_ = rsp_->code_;
						rsp_->MakeStatusLine();
						rsp_->MakeResponseHead(rq_);
						rsp_->OpenResource(rq_);
						conn_->SendResponse(rsp_, rq_, false);
				}
				static int PorcessResponse(Connect *&conn_, Request *&rq_, Response *&rsp_ )//处理响应
				{
						if( rq_->IsCgi() )
						{
								//ProcessCgi();
						}
						else
						{
								ProcessNonCgi(conn_, rq_, rsp_);
						}
				}
				static void *HandlerRequest(void *arg_)//处理请求
				{
						int sock_ = *(int*)arg_;
						delete (int*)arg_;
						Connect *conn_ = new Connect(sock_);
						Request *rq_ = new Request();
						Response *rsp_ = new Response();

						int &code_ = rsp_->code;
						conn_-> RecvOneLine(rq_->rq_line);
						rq_->RequestOneLineParse();
						if( !rq_->IsMethodLegal() )
						{
								code_ = NOT_FOUND;
								goto end;
						}

						rq_->UriParse();

						if( !rq_->IsPathLegal() )
						{
								code_ = NOT_FOUND;
								goto end;
						}
						LOG(INFO,"request path os ok");

						conn_->RecvRequestHead(rq_->rq_head);
						if( rq_->RequestHeadParse() )
						{
								LOG(INFO,"parse head done");
						}
						else
						{
								code_ = NOT_FOUND;
								goto end;
						}
						if( rq_->IsNeedRecvText() )
						{
								conn_->RecvRequestText(rq_->rq_text, rq_->GetContentLength(), rq_->GetParam() );
						}
						//响应接收完毕
						PorcessResponse(conn_, rq_, rsp_ );//处理响应

end:
						if(code_ != OK)
						{
								//							HandlerError(sock_);
						}
						delete conn_;
						delete rq_;
						delete rsp_;
				}
};
#endif
