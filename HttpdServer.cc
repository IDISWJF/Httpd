#include "HttpdServer.hpp"


static void Usage(std::string proc_)
{
	std::cout<<"Usage"<<proc_<<" port "<<std::endl;
}
int main(int argc,char *argv[])
{
	if(argc != 2)
	{
		Usage(argv[0]);//0就是输入可执行程序的名称
		exit(1);
	}	
	signal(SIGPIPE,SIG_IGN);
	HttpdServer *serp = new HttpdServer(atoi(argv[1]));//将字符型装换成整形

	//创建事件处理框架
	struct event_base* base = event_base_new();
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
	listen = evconnlistener_new_bind(base, serp->listen_cb, base, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, 
			                        (struct sockaddr*)&(serv) , sizeof(serv));
	if(!listen)
	{
		std::cout<<"listen error"<<std::endl;
	}
	//开始事件循环
	event_base_dispatch(base);    
	//释放资源
	evconnlistener_free(listen);
	event_base_free(base);


	delete serp;
	return 0;
}
