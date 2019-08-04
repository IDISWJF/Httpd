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
#include "Log.hpp"

class HttpdServer{
	private:
		int listen_sock;
		int port;
		static int fd_;
	public:
		HttpdServer(int port_):port(port_),listen_sock(-1)
	{}
	
		~HttpdServer()
		{
			if(listen_sock != -1){
				close(listen_sock);
			}
			port =-1;
		}
};
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
void read_cb(struct bufferevent* bev, void* arg)
{
	Entry::HandlerRequest(bev);
    char buf[4097] = {0};
    bufferevent_read(bev, buf, sizeof(buf));
}
//连接完成后对应的操作
void listen_cb(struct  evconnlistener* listen,
                evutil_socket_t fd,
                struct  sockaddr* addr,
                int len, void* ptr)
{
    //得到传进来的event_base
    struct event_base* base = (struct event_base*)ptr;
  
    struct  bufferevent* bev = NULL;
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	HttpdServer fd_ =fd;
    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);

    bufferevent_enable(bev, EV_READ);   
}


#endif
