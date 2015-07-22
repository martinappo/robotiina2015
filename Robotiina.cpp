// Robotiina.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "Robot.h"
#include "RemoteControl.h"


int main(int argc, char *argv[])
{
    boost::asio::io_service io;
    Robot robotiina(io);
//    RemoteControl sr(io, &robotiina);

	try
	{
//           sr.Start();

		robotiina.Launch(argc, argv);
    }
	catch (std::exception &e)
	{
		std::cout << "ups, " << e.what() << std::endl;
	}
	catch (const std::string &e)
	{
		std::cout << "ups, " << e << std::endl;
	}
	catch (...)
	{
		std::cout << "ups, did not see that coming."<< std::endl;
	}
//    sr.Stop();
    return 0;

}
