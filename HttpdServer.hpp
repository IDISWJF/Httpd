#ifndef __HTTPD_SERVER_HPP__
#define __HTTPD_SERVER_HPP__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include "ProtocolUtil.hpp"
#include "ThreadPool.hpp"
#include "Log.hpp"



class HttpdServer{
	private:
		int listen_sock;
		int port;
		ThreadPool *tp;
		event_base *base_;

	public:
	 static void listen_cb(struct evconnlistener* listen,evutil_socket_t fd,
		sockaddr* addr,int len, void* ptr)
{
	HttpdServer* serp = (HttpdServer*)ptr;
	LOG(INFO,"listen_cb");

	struct  bufferevent* bev = bufferevent_socket_new(serp->GetEventBase(), fd, 
			BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE);
	LOG(INFO,"listen_cb end "); 
	bufferevent_setcb(bev, HttpdServer::read_cb, NULL, HttpdServer::event_cb, NULL);

	bufferevent_enable(bev, EV_READ | EV_PERSIST); 
 
}

static void read_cb(struct bufferevent* bev, void* arg)
{
	LOG(INFO,"read_cd ！！！");
	HttpdServer* hs = (HttpdServer*)arg;
	ThreadPool* tp_ = hs->GetThreadPool();
	tp_ = new ThreadPool();
	tp_->initThreadPool();
	LOG(INFO,"initServer success!");

	int sock_ = bufferevent_getfd(bev);
	Task t;
	t.SetTask(sock_,Entry:: HandlerRequest);
	tp_->PushTask(t);
}

static void event_cb(struct bufferevent* bev, short events, void* arg)
{
	if(events & BEV_EVENT_EOF)
	{
		std::cout<<" connect closed " <<std::endl;
	}
	else if(events & BEV_EVENT_ERROR)
	{
		std::cout<<" other error " <<std::endl;
	}
	bufferevent_free(bev);
}
		event_base* GetEventBase() { return base_; }
		ThreadPool* GetThreadPool() { return tp; }
		HttpdServer(int port_):port(port_),listen_sock(-1)
	{}
		void run()
		{

			// new是操作符一旦申请失败会抛异常，所以不需要判断
			//serp->InitServer();
			//serp->Start();
			//创建事件处理框架
			base_ = event_base_new();
			//init server info
			struct sockaddr_in serv;

			serv.sin_family = AF_INET;
			serv.sin_port = htons(1234);
			serv.sin_addr.s_addr = htonl(INADDR_ANY);
			//创建监听套接字
			//绑定
			//监听
			//等待并接收了连接
			struct evconnlistener* listen = NULL;
			//当有新的连接过来时，listen_cb 会被调用
			//第三参数是为了给回调提供参数
			//第五个参数backlog(-1)使用默认最大数量128
			listen = evconnlistener_new_bind(base_, HttpdServer::listen_cb, this,
			         LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
					 -1, (struct sockaddr*)&(serv) , sizeof(serv));
			//开始事件循环
			event_base_dispatch(base_);    
			LOG(INFO,"dispatch !!!");
			//释放资源
			//evconnlistener_free(listen);
			event_base_free(base_);
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
