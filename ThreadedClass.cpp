
#include "ThreadedClass.h"


ThreadedClass::ThreadedClass(const std::string &name): name(name)
{
	stop_thread = false;
	running = false;
}

void ThreadedClass::Start()
{
	assert(!name.empty());
	stop_thread = false;
	running = true;
	threads.create_thread(boost::bind(&ThreadedClass::Run, this));
};
void ThreadedClass::WaitForStop()
{
	std::cout << "Stoping thread: " << name << std::endl;
	stop_thread = true;
	threads.join_all();
	running = false;
};

ThreadedClass::~ThreadedClass()
{
	if(!stop_thread) {
		std::cout << "thread: " << name << " not stoped" << std::endl;
		WaitForStop();
	}
	std::cout << "Exiting thread " << name << std::endl;
}
