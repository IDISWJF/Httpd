#ifndef __LOG_HPP__
#define __LOG_HPP__

#include<string>
#include<iostream>

#define INFO 0 
#define DEBUG 1
#define WARNING 2
#define ERROR 2

std::string GetLogLevel(int level_)
{
		switch(level_)
		{
				case 0:
						return "INFO";
				case 1:
						return "DEBUG";
				case 2:
						return "WARNING";
				case 3:
						return "ERROR";
				default:
						return "UNKNOW";
		}

}//打印日志
void LOG(int level_, std::string message_,std::string file_, int line_)
{
		std::cout << " [ " <<GetLogLevel(level_)<<" ] "<<" [ "<<\
              file_ <<" : "<<line_ <<" ] "<<message_ <<std::endl;
}
#define LOG(level_, message_) LOG(level_, message_, __FILE__, __LINE__)
#endif
