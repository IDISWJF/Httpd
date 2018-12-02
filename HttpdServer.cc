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
		HttpdServer *serp = new HttpdServer(atoi(argv[1]));//将字符型装换成整形
		// new是操作符一旦申请失败会抛异常，所以不需要判断
		serp->InitServer();
		serp->Start();

		delete serp;
		return 0;
}
