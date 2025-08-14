#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <stdexcept>
#include <cmath>
#include <numeric>

/*
 * Copied and slightly adapted from
 * nifty/include/nifty/parallel/threadpool.hxx
 */



namespace z5 {
namespace util {

/** \addtogroup ParallelProcessing Functions and classes for parallel processing.
*/

//@{

    /**\brief Option base class for parallel algorithms.

        <b>\#include</b> \<nifty/parallel/threadpool.hxx\><br>
        Namespace: nifty::parallel
    */
class ParallelOptions
{
  public:

        /** Constants for special settings.
        */
    enum {
        Auto       = -1, ///< Determine number of threads automatically (from <tt>std::thread::hardware_concurrency()</tt>)
        Nice       = -2, ///< Use half as many threads as <tt>Auto</tt> would.
        NoThreads  =  0  ///< Switch off multi-threading (i.e. execute tasks sequentially)
    };

    ParallelOptions(int nT = Auto)
    :   numThreads_(actualNumThreads(nT))
    {}

        /** \brief Get desired number of threads.

            <b>Note:</b> This function may return 0, which means that multi-threading
            shall be switched off entirely. If an algorithm receives this value,
            it should revert to a sequential implementation. In contrast, if
            <tt>numThread() == 1</tt>, the parallel algorithm version shall be
            executed with a single thread.
        */
    int getNumThreads() const
    {
        return numThreads_;
    }

        /** \brief Get desired number of threads.

            In contrast to <tt>numThread()</tt>, this will always return a value <tt>>=1</tt>.
        */
    int getActualNumThreads() const
    {
        return (std::max)(1,numThreads_);
    }

        /** \brief Set the number of threads or one of the constants <tt>Auto</tt>,
                   <tt>Nice</tt> and <tt>NoThreads</tt>.

            Default: <tt>ParallelOptions::Auto</tt> (use system default)

            This setting is ignored if the preprocessor flag <tt>NIFTY_NO_PARALLELISM</tt>
            is defined. Then, the number of threads is set to 0 and all tasks revert to
            sequential algorithm implementations. The same can be achieved at runtime
            by passing <tt>n = 0</tt> to this function. In contrast, passing <tt>n = 1</tt>
            causes the parallel algorithm versions to be executed with a single thread.
            Both possibilities are mainly useful for debugging.
        */
    ParallelOptions & numThreads(const int n)
    {
        numThreads_ = actualNumThreads(n);
        return *this;
    }


  private:
        // helper function to compute the actual number of threads
    static std::size_t actualNumThreads(const int userNThreads)
    {
        #ifdef NIFTY_NO_PARALLELISM
            return 0;
        #else
            return userNThreads >= 0
                       ? userNThreads
                       : userNThreads == Nice
                               ? std::thread::hardware_concurrency() / 2
                               : std::thread::hardware_concurrency();
        #endif
    }

    int numThreads_;
};

/********************************************************/
/*                                                      */
/*                      ThreadPool                      */
/*                                                      */
/********************************************************/

    /**\brief Thread pool class to manage a set of parallel workers.

        <b>\#include</b> \<nifty/parallel/threadpool.hxx\><br>
        Namespace: nifty::parallel
    */
class ThreadPool
{
  public:

    /** Create a thread pool from ParallelOptions. The constructor just launches
        the desired number of workers. If the number of threads is zero,
        no workers are started, and all tasks will be executed in synchronously
        in the present thread.
     */
    ThreadPool(const ParallelOptions & options)
    :   stop(false),
        busy(0),
        processed(0)
    {
        init(options);
    }

    /** Create a thread pool with n threads. The constructor just launches
        the desired number of workers. If \arg n is <tt>ParallelOptions::Auto</tt>,
        the number of threads is determined by <tt>std::thread::hardware_concurrency()</tt>.
        <tt>ParallelOptions::Nice</tt> will create half as many threads.
        If <tt>n = 0</tt>, no workers are started, and all tasks will be executed
        synchronously in the present thread. If the preprocessor flag
        <tt>NIFTY_NO_PARALLELISM</tt> is defined, the number of threads is always set
        to zero (i.e. synchronous execution), regardless of the value of \arg n. This
        is useful for debugging.
     */
    ThreadPool(const int n)
    :   stop(false),
        busy(0),
        processed(0)
    {
        init(ParallelOptions().numThreads(n));
    }

    /**
     * The destructor joins all threads.
     */
    ~ThreadPool();

    /**
     * Enqueue a task that will be executed by the thread pool.
     * The task result can be obtained using the get() function of the returned future.
     * If the task throws an exception, it will be raised on the call to get().
     */
    template<class F>
    std::future<typename std::invoke_result_t<F, int>>  enqueueReturning(F&& f) ;

    /**
     * Enqueue function for tasks without return value.
     * This is a special case of the enqueueReturning template function, but
     * some compilers fail on <tt>std::result_of<F(int)>::type</tt> for void(int) functions.
     */
    template<class F>
    std::future<void> enqueue(F&& f) ;

    /**
     * Block until all tasks are finished.
     */
    void waitFinished()
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        finish_condition.wait(lock, [this](){ return tasks.empty() && (busy == 0); });
    }

    /**
     * Return the number of worker threads.
     */
    std::size_t nThreads() const
    {
        return workers.size();
    }

private:

    // helper function to init the thread pool
    void init(const ParallelOptions & options);

    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;

    // the task queue
    std::queue<std::function<void(int)> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable worker_condition;
    std::condition_variable finish_condition;
    bool stop;
    std::atomic<unsigned int> busy, processed;
};

inline void ThreadPool::init(const ParallelOptions & options)
{
    const std::size_t actualNThreads = options.getNumThreads();
    for(std::size_t ti = 0; ti<actualNThreads; ++ti)
    {
        workers.emplace_back(
            [ti,this]
            {
                for(;;)
                {
                    std::function<void(int)> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);

                        // will wait if : stop == false  AND queue is empty
                        // if stop == true AND queue is empty thread function will return later
                        //
                        // so the idea of this wait, is : If where are not in the destructor
                        // (which sets stop to true, we wait here for new jobs)
                        this->worker_condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                        if(!this->tasks.empty())
                        {
                            ++busy;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                            lock.unlock();
                            task(ti);
                            ++processed;
                            --busy;
                            finish_condition.notify_one();
                        }
                        else if(stop)
                        {
                            return;
                        }
                    }
                }
            }
        );
    }
}

inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    worker_condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

template<class F>
inline std::future<typename std::invoke_result_t<F, int>>
ThreadPool::enqueueReturning(F&& f)
{
    typedef typename std::invoke_result_t<F, int> result_type;
    typedef std::packaged_task<result_type(int)> PackageType;

    auto task = std::make_shared<PackageType>(f);
    auto res = task->get_future();

    if(workers.size()>0){
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace(
                [task](int tid)
                {
                    (*task)(tid);
                }
            );
        }
        worker_condition.notify_one();
    }
    else{
        (*task)(0);
    }

    return res;
}

template<class F>
inline std::future<void>
ThreadPool::enqueue(F&& f)
{
    typedef std::packaged_task<void(int)> PackageType;

    auto task = std::make_shared<PackageType>(f);
    auto res = task->get_future();
    if(workers.size()>0){
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace(
                [task](int tid)
                {
                    (*task)(tid);
                }
            );
        }
        worker_condition.notify_one();
    }
    else{
        (*task)(0);
    }
    return res;
}

/********************************************************/
/*                                                      */
/*                   parallel_foreach                   */
/*                                                      */
/********************************************************/

// nItems must be either zero or std::distance(iter, end).
template<class ITER, class F>
inline void parallel_foreach_impl(
    ThreadPool & pool,
    const std::ptrdiff_t nItems,
    ITER iter,
    ITER end,
    F && f,
    std::random_access_iterator_tag
){
    std::ptrdiff_t workload = std::distance(iter, end);

    // NIFTY_CHECK(workload == nItems || nItems == 0, "parallel_foreach(): Mismatch between num items and begin/end.");
    const float workPerThread = float(workload)/pool.nThreads();
    const std::ptrdiff_t chunkedWorkPerThread = (std::max<std::ptrdiff_t>)(int(workPerThread/3.0f + 0.5f), 1);

    std::vector<std::future<void> > futures;
    while(iter<end)
    {
        const std::size_t lc = (std::min)(workload, chunkedWorkPerThread);
        workload-=lc;
        futures.emplace_back(
            pool.enqueue(
                [&f, iter=iter, lc=lc]
                (int id)
                {
                    for(std::size_t i=0; i<lc; ++i)
                        f(id, iter[i]);
                }
            )
        );

        iter+=lc;

        if(workload==0)
            break;
    }
    for (auto & fut : futures)
    {
        fut.get();
    }
}



// nItems must be either zero or std::distance(iter, end).
template<class ITER, class F>
inline void parallel_foreach_impl(
    ThreadPool & pool,
    const std::ptrdiff_t nItems,
    ITER iter,
    ITER end,
    F && f,
    std::forward_iterator_tag
){
    if (nItems == 0)
        nItems = std::distance(iter, end);

    std::ptrdiff_t workload = nItems;
    const float workPerThread = float(workload)/pool.nThreads();
    const std::ptrdiff_t chunkedWorkPerThread = (std::max<std::ptrdiff_t>)(int(workPerThread/3.0f+0.5f), 1);

    std::vector<std::future<void> > futures;
    for(;;)
    {
        const std::size_t lc = (std::min)(chunkedWorkPerThread, workload);
        workload -= lc;
        futures.emplace_back(
            pool.enqueue(
                [&f, iter, lc]
                (int id)
                {
                    auto iterCopy = iter;
                    for(std::size_t i=0; i<lc; ++i){
                        f(id, *iterCopy);
                        ++iterCopy;
                    }
                }
            )
        );
        for (std::size_t i = 0; i < lc; ++i)
        {
            ++iter;
            if (iter == end)
            {
                // NIFTY_CHECK_OP(workload, ==, 0, "parallel_foreach(): Mismatch between num items and begin/end.");
                break;
            }
        }
        if(workload==0)
            break;
    }
    for (auto & fut : futures)
        fut.get();
}



// nItems must be either zero or std::distance(iter, end).
template<class ITER, class F>
inline void parallel_foreach_impl(
    ThreadPool & pool,
    const std::ptrdiff_t nItems,
    ITER iter,
    ITER end,
    F && f,
    std::input_iterator_tag
){
    std::size_t num_items = 0;
    std::vector<std::future<void> > futures;
    for (; iter != end; ++iter)
    {
        auto item = *iter;
        futures.emplace_back(
            pool.enqueue(
                [&f, &item](int id){
                    f(id, item);
                }
            )
        );
        ++num_items;
    }
    // NIFTY_CHECK(num_items == nItems || nItems == 0, "parallel_foreach(): Mismatch between num items and begin/end.");
    for (auto & fut : futures)
        fut.get();
}

// Runs foreach on a single thread.
// Used for API compatibility when the number of threads is 0.
template<class ITER, class F>
inline void parallel_foreach_single_thread(
    ITER begin,
    ITER end,
    F && f,
    const std::ptrdiff_t nItems = 0
){
    std::size_t n = 0;
    for (; begin != end; ++begin)
    {
        f(0, *begin);
        ++n;
    }
    // NIFTY_CHECK(n == nItems || nItems == 0, "parallel_foreach(): Mismatch between num items and begin/end.");
}

/** \brief Apply a functor to all items in a range in parallel.

    Create a thread pool (or use an existing one) to apply the functor \arg f
    to all items in the range <tt>[begin, end)</tt> in parallel. \arg f must
    be callable with two arguments of type <tt>std::size_t</tt> and <tt>T</tt>, where
    the first argument is the thread index (starting at 0) and T is convertible
    from the iterator's <tt>reference_type</tt> (i.e. the result of <tt>*begin</tt>).

    If the iterators are forward iterators (<tt>std::forward_iterator_tag</tt>), you
    can provide the optional argument <tt>nItems</tt> to avoid the a
    <tt>std::distance(begin, end)</tt> call to compute the range's length.

    Parameter <tt>nThreads</tt> controls the number of threads. <tt>parallel_foreach</tt>
    will split the work into about three times as many parallel tasks.
    If <tt>nThreads = ParallelOptions::Auto</tt>, the number of threads is set to
    the machine default (<tt>std::thread::hardware_concurrency()</tt>).

    If <tt>nThreads = 0</tt>, the function will not use threads,
    but will call the functor sequentially. This can also be enforced by setting the
    preprocessor flag <tt>NIFTY_NO_PARALLELISM</tt>, ignoring the value of
    <tt>nThreads</tt> (useful for debugging).

    <b> Declarations:</b>

    \code
    namespace nifty {
    namespace parallel{
        // pass the desired number of threads or ParallelOptions::Auto
        // (creates an internal thread pool accordingly)
        template<class ITER, class F>
        void parallel_foreach(int64_t nThreads,
                              ITER begin, ITER end,
                              F && f,
                              const uint64_t nItems = 0);

        // use an existing thread pool
        template<class ITER, class F>
        void parallel_foreach(ThreadPool & pool,
                              ITER begin, ITER end,
                              F && f,
                              const uint64_t nItems = 0);

        // pass the integers from 0 ... (nItems-1) to the functor f,
        // using the given number of threads or ParallelOptions::Auto
        template<class F>
        void parallel_foreach(int64_t nThreads,
                              uint64_t nItems,
                              F && f);

        // likewise with an existing thread pool
        template<class F>
        void parallel_foreach(ThreadPool & threadpool,
                              uint64_t nItems,
                              F && f);
    }
    }
    \endcode

    <b>Usage:</b>

    \code
    #include <iostream>
    #include <algorithm>
    #include <vector>
    #include <nifty/parallel/threadpool.hxx>

    using namespace std;
    using namespace nifty::parallel;

    int main()
    {
        std::size_t const n_threads = 4;
        std::size_t const n = 2000;
        vector<int> input(n);

        auto iter = input.begin(),
             end  = input.end();

        // fill input with 0, 1, 2, ...
        iota(iter, end, 0);

        // compute the sum of the elements in the input vector.
        // (each thread computes the partial sum of the items it sees
        //  and stores the sum at the appropriate index of 'results')
        vector<int> results(n_threads, 0);
        parallel_foreach(n_threads, iter, end,
            // the functor to be executed, defined as a lambda function
            // (first argument: thread ID, second argument: result of *iter)
            [&results](std::size_t thread_id, int items)
            {
                results[thread_id] += items;
            }
        );

        // collect the partial sums of all threads
        int sum = accumulate(results.begin(), results.end(), 0);

        cout << "The sum " << sum << " should be equal to " << (n*(n-1))/2 << endl;
    }
    \endcode
 */
//doxygen_overloaded_function(template <...> void parallel_foreach)

template<class ITER, class F>
inline void parallel_foreach(
    ThreadPool & pool,
    ITER begin,
    ITER end,
    F && f,
    const std::ptrdiff_t nItems = 0)
{
    if(pool.nThreads()>1)
    {
        parallel_foreach_impl(pool,nItems, begin, end, f,
            typename std::iterator_traits<ITER>::iterator_category());
    }
    else
    {
        parallel_foreach_single_thread(begin, end, f, nItems);
    }
}

template<class ITER, class F>
inline void parallel_foreach(
    int64_t nThreads,
    ITER begin,
    ITER end,
    F && f,
    const std::ptrdiff_t nItems = 0)
{

    ThreadPool pool(nThreads);
    parallel_foreach(pool, begin, end, f, nItems);
}

template<class F>
inline void parallel_foreach(
    int64_t nThreads,
    std::ptrdiff_t nItems,
    F && f)
{
    std::vector<int64_t> indices(nItems);
    std::iota(indices.begin(), indices.end(), 0);

    parallel_foreach(nThreads, indices.begin(), indices.end(), std::forward<F>(f), nItems);

}


template<class F>
inline void parallel_foreach(
    ThreadPool & threadpool,
    std::ptrdiff_t nItems,
    F && f)
{
    std::vector<int64_t> indices(nItems);
    std::iota(indices.begin(), indices.end(), 0);

    parallel_foreach(threadpool, indices.begin(), indices.end(), std::forward<F>(f), nItems);
}

//@}

} // namespace parallel
} // namespace nifty
