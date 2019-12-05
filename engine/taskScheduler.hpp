#pragma once

#include <thread>
#include <memory>
#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <thread>
#include <condition_variable>
#include <boost/lockfree/queue.hpp>

// the scheduler will split the container to these ranges 
struct TaskRange
{
	size_t start;
	size_t end;
};

// the scheduler will accept references to classes derived from this 
class RangedTask
{
	// friended to taskscheduler to access private var
	friend class TaskScheduler;

	// the number of partitions the task scheduler still has to process, 
	// not the number of elements in the range  
	std::atomic<int> rangesLeftToProcess;
	bool isComplete() const;

public:
	size_t rangeSize = 1;
	virtual void execute( const TaskRange& range ) = 0;
};

// the scheduler will distribute these to the queues 
struct PartitionedTaskSet
{
	TaskRange range;
	RangedTask* task;
};

// accepts RangedTasks and runs it's execute() concurrently
class TaskScheduler
{
	using TaskQueue_t = boost::lockfree::queue<PartitionedTaskSet,
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<64>>;

	bool					isShuttingDown{ false };

	std::vector<std::thread> threads;
	TaskQueue_t*			taskQueues;

	// for shutdown and new tasks 
	std::mutex				convarMutex;
	std::condition_variable threadEvent;

	// for shutting down 
	std::atomic<int>		runningThreads;
	size_t					numThreads;

	// parses the taskset into chunks 
	std::vector<PartitionedTaskSet> divideTask( RangedTask* task );

	// the threads will run this function 
	void threadFn( const size_t queueIndex );

	static std::unique_ptr<TaskScheduler> _instance;
public:
	void initialize( const size_t nThreads = std::thread::hardware_concurrency() );
	
	void execute( RangedTask* task );
	void waitFor( RangedTask* task );

	// shuts down the system and waits for threads to join
	void shutdown();

	static TaskScheduler* instance();

	~TaskScheduler();
};