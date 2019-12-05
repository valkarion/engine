#include "taskScheduler.hpp"

std::unique_ptr<TaskScheduler> TaskScheduler::_instance = std::make_unique<TaskScheduler>();

TaskScheduler* TaskScheduler::instance()
{
	return _instance.get();
}

bool RangedTask::isComplete() const
{
	return rangesLeftToProcess.load() == 0;
}

std::vector<PartitionedTaskSet> TaskScheduler::divideTask( RangedTask* task )
{
	std::vector<PartitionedTaskSet> res;

	const size_t range = task->rangeSize;
	const size_t partitionSize = range / numThreads;

	PartitionedTaskSet pts;
	pts.task = task;

	TaskRange tr;
	for( size_t i = 0; i < numThreads; i++ )
	{
		tr.start = i * partitionSize;

		if( i != numThreads - 1 )
		{
			tr.end = ( i + 1 ) * partitionSize;
		}
		else
		{
			tr.end = range;
		}

		pts.range = tr;
		res.push_back( pts );
	}

	return res;
};

void TaskScheduler::threadFn( const size_t queueIndex )
{
	while( !isShuttingDown )
	{
		{
			std::unique_lock<std::mutex> lock( convarMutex );
			threadEvent.wait( lock );
		}

		if( !taskQueues[queueIndex].empty() )
		{
			PartitionedTaskSet pTask;
			taskQueues[queueIndex].pop( pTask );
			pTask.task->execute( pTask.range );
			pTask.task->rangesLeftToProcess.fetch_sub( 1 );
		}
	}

	--runningThreads;
}

void TaskScheduler::initialize( const size_t nThreads )
{
	numThreads = nThreads;
	runningThreads = 0;

	taskQueues = new TaskQueue_t[nThreads];
	for( size_t i = 0; i < nThreads; i++ )
	{
		threads.emplace_back( std::thread( [&, i]() { threadFn( i ); } ) );
		++runningThreads;
	}
};

void TaskScheduler::shutdown()
{
	{
		{
			std::unique_lock<std::mutex> lock( convarMutex );
			isShuttingDown = true;
		}
		threadEvent.notify_all();
	}

	for( auto& it : threads )
	{
		it.join();
	}

	delete[] taskQueues;
}

void TaskScheduler::execute( RangedTask* task )
{
	std::vector<PartitionedTaskSet> pts = divideTask( task );
	task->rangesLeftToProcess = (int)pts.size();
	for( size_t i = 0; i < pts.size(); i++ )
	{
		taskQueues[i % numThreads].push( pts[i] );
	}

	std::unique_lock<std::mutex> lock( convarMutex );
	threadEvent.notify_all();
}

void TaskScheduler::waitFor( RangedTask* task )
{
	while( !task->isComplete() )
	{
	}
}

TaskScheduler::~TaskScheduler()
{
	if( !isShuttingDown )
	{
		shutdown();
	}
}