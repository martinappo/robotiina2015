#pragma once
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

class ThreadedClass
{
public:
	ThreadedClass(const std::string &name = ""); 
	void Start();
	void WaitForStop();
	bool HasError() { return stop_thread; };

	virtual ~ThreadedClass();
protected:
	boost::thread_group threads;
	boost::atomic<bool> stop_thread;
	std::string name;
	virtual void Run() = 0;

};

