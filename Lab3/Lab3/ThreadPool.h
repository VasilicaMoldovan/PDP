#pragma once

#include<thread>
#include<vector>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<future>

#define MAX_THREADS std::thread::hardware_concurrency() - 1;

class ThreadPool {
public:

    //getInstance to allow the second constructor to be called
    static ThreadPool& getInstance(int numThreads) {
        static ThreadPool instance(numThreads);

        return instance;
    }

    template <typename Func, typename... Args >
    inline auto push(Func&& f, Args&&... args) {

        typedef decltype(f(args...)) retType;

        std::packaged_task<retType()> task(std::move(std::bind(f, args...)));

        // lock jobqueue mutex, add job to the job queue 
        std::unique_lock<std::mutex> lock(JobMutex);

        //get the future from the task before the task is moved into the jobqueue
        std::future<retType> future = task.get_future();

        //place the job into the queue
        JobQueue.emplace(std::make_shared<AnyJob<retType> >(std::move(task)));

        //notify a thread that there is a new job
        thread.notify_one();

        //return the future for the function so the user can get the return value
        return future;
    }

    inline size_t getThreadCount() {
        return numThreads;
    }

private:

    //used polymorphism to store any type of function in the job queue
    class Job {
    private:
        std::packaged_task<void()> func;
    public:
        virtual ~Job() {}
        virtual void execute() = 0;
    };

    template <typename RetType>
    class AnyJob : public Job {
    private:
        std::packaged_task<RetType()> func;
    public:
        AnyJob(std::packaged_task<RetType()> func) : func(std::move(func)) {}
        void execute() {
            func();
        }
    };
    
    size_t numThreads; // number of threads in the pool
    std::vector<std::thread> Pool; //the actual thread pool
    std::queue<std::shared_ptr<Job>> JobQueue; //tasks queue
    std::condition_variable thread;// used to notify threads about available jobs
    std::mutex JobMutex; // used to push/pop jobs to/from the queue
    
    /* infinite loop function */
    inline void threadManager() {
        while (true) {

            std::unique_lock<std::mutex> lock(JobMutex);
            thread.wait(lock, [this] {return !JobQueue.empty(); });

            if (JobQueue.size() < 1)
                continue;

            (*JobQueue.front()).execute();

            JobQueue.pop();
        }
    }

    ThreadPool();

    inline ThreadPool(size_t numThreads) : numThreads(numThreads) {
        int tmp = MAX_THREADS;
        if (numThreads > tmp) {
            numThreads = tmp;
        }
        Pool.reserve(numThreads);
        for (int i = 0; i != numThreads; ++i) {
            Pool.emplace_back(std::thread(&ThreadPool::threadManager, this));
            Pool.back().detach();
        }
    }
};