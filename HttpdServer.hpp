#ifndef __HTTPD_SERVER_HPP__
#define __HTTPD_SERVER_HPP__

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "ThreadPool.hpp"
#include "Log.hpp"

class HttpdServer{
	private:
		int listen_sock;
		int port;
		int socket;
		ThreadPool *tp;
	public:
		HttpdServer(int port_):port(port_),socket(-1),listen_sock(-1),tp(NULL)
	{}
		void InitServer()
		{
			tp = new ThreadPool();
			tp->initThreadPool();
			LOG(INFO,"initServer success!");
		}
		void read_cb(struct bufferevent* bev, void* arg)
		{
			LOG(INFO,"Start Server begin");

			Task t;
			t.SetTask(socket,Entry:: HandlerRequest);
			tp->PushTask(t);

		}
		//事件回调
		void event_cb(struct bufferevent* bev, short events, void* arg)
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
		void getsocket(evutil_socket_t fd_)
		{
			socket = fd_;
		}

		void listen_cb(struct  evconnlistener* listen,
				evutil_socket_t fd,
				struct  sockaddr* addr,
				int len, void* ptr)
		{
			//得到传进来的event_base
			struct event_base* base = (struct event_base*)ptr;
			struct  bufferevent* bev = NULL;
			bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
			getsocket(fd);
			bufferevent_setcb(bev, HttpdServer::read_cb, NULL, NULL, HttpdServer::event_cb);
			bufferevent_enable(bev, EV_READ);   
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
