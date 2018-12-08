#include<iostream>
#include<stdlib.h>
#include<string>
#include<unistd.h>
#include<cstdio>//c++中引用c头文件的方式
int main()
{
		if(getenv("Content-Length"))
		{
				int size_  = atoi(getenv("Content-Length"));
				std::string param_;
				char c_;
				while(size_)
				{
						read(0,&c_,1);
						param_.push_back(c_);
						size_--;
				}
				int a,b;
				sscanf(param_.c_str(),"a=%d&b=%d",&a,&b);//将参数以指定的形式输出
				std::cout<<a<<" "<<b<<std::endl;
		}
		return 0;
}
