#ifndef LMS_FIFO_BUFFER_H
#define LMS_FIFO_BUFFER_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include "dataTypes.h"
#include <cmath>
#include <assert.h>

namespace lime{

class RingFIFO
{
public:

    struct BufferInfo
    {
        uint32_t size;
        uint32_t itemsFilled;
    };
    
    enum StreamFlags
    {
        SYNC_TIMESTAMP = 1,
        END_BURST = 2,
        OVERWRITE_OLD = 4,
    };

    //! @brief Returns information about FIFO size and fullness
    BufferInfo GetInfo()
    {
        std::unique_lock<std::mutex> lck(lock);
        BufferInfo stats;
        stats.size = mBufferSize*mBuffer->maxSamplesInPacket;
        stats.itemsFilled = mElementsFilled*mBuffer->maxSamplesInPacket;
        return stats;
    }

    //!    @brief Initializes FIFO memory
    RingFIFO(const uint32_t bufLength) : mBufferSize(1+(bufLength-1)/mBuffer->maxSamplesInPacket)
    {
        mBuffer = new SamplesPacket[mBufferSize];
        Clear();
    }

    ~RingFIFO()
    {
        delete []mBuffer;
    };

    /** @brief inserts samples to FIFO, operation is thread-safe
    @param buffer pointers to arrays containing samples data of each channel
    @param samplesCount number of samples to insert from each buffer channel
    @param channelsCount number of channels to insert
    @param timeout_ms timeout duration for operation
    @param flags optional flags associated with the samples
    @return number of items inserted
    */
    uint32_t push_samples(const complex16_t *buffer, const uint32_t samplesCount, const uint8_t channelsCount, uint64_t timestamp, const uint32_t timeout_ms, const uint32_t flags = 0)
    {
        assert(buffer != nullptr);
        uint32_t samplesTaken = 0;
        std::unique_lock<std::mutex> lck(lock);
        auto t1 = std::chrono::high_resolution_clock::now();
        while (samplesTaken < samplesCount)
        {
            if (mElementsFilled >= mBufferSize) //buffer might be full, wait for free slots
            {
                auto t2 = std::chrono::high_resolution_clock::now();
                if(t2-t1 >= std::chrono::milliseconds(timeout_ms))
                    return samplesTaken;
                if(flags & OVERWRITE_OLD)
                {
                    int dropElements = 1+(samplesCount-samplesTaken)/SamplesPacket::maxSamplesInPacket;
                    mHead = (mHead + dropElements) & (mBufferSize - 1);//advance to next one
                    mElementsFilled -= dropElements;
                }
                else  //there is no space, wait on CV to give pop_samples the thread context
                {
                    hasItems.wait_for(lck, std::chrono::milliseconds(timeout_ms));
                }
            }
            else
            {
                mBuffer[mTail].timestamp = timestamp + samplesTaken;
                int cnt = samplesCount-samplesTaken;
                if (cnt > SamplesPacket::maxSamplesInPacket)
                {
                    cnt = SamplesPacket::maxSamplesInPacket;
                    mBuffer[mTail].flags = flags & SYNC_TIMESTAMP;
                }
                else
                    mBuffer[mTail].flags = flags;
                memcpy(mBuffer[mTail].samples,&buffer[samplesTaken],cnt*sizeof(complex16_t));
                samplesTaken+=cnt;
                mBuffer[mTail].last = cnt;
                mBuffer[mTail++].first = 0;
                mTail  &= (mBufferSize - 1);//advance to next one
                ++mElementsFilled;
            }
        }
        lck.unlock();
        hasItems.notify_one();
        return samplesTaken;
    }

    /** @brief Takes samples out of FIFO, operation is thread-safe
        @param buffer pointers to destination arrays for each channel's samples data, each array must be big enough to contain \samplesCount number of samples.
        @param samplesCount number of samples to pop
        @param channelsCount number of channels to pop
        @param timestamp returns timestamp of the first sample in buffer
        @param timeout_ms timeout duration for operation
        @param flags optional flags associated with the samples
        @return number of samples popped
    */
    uint32_t pop_samples(complex16_t* buffer, const uint32_t samplesCount, const uint8_t channelsCount, uint64_t *timestamp, const uint32_t timeout_ms, uint32_t *flags = nullptr)
    {
        assert(buffer != nullptr);
        uint32_t samplesFilled = 0;
        if (flags != nullptr) *flags = 0;
        std::unique_lock<std::mutex> lck(lock);
        while (samplesFilled < samplesCount)
        {
            while (mElementsFilled == 0) //buffer might be empty, wait for packets
            {
                if (timeout_ms == 0)
                    return samplesFilled;
                if (hasItems.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                    return samplesFilled;
            }
            if(samplesFilled == 0 && timestamp != nullptr)
                *timestamp = mBuffer[mHead].timestamp + mBuffer[mHead].first;

            while(mElementsFilled > 0 && samplesFilled < samplesCount)
            {
                const bool hasEOB = mBuffer[mHead].flags & END_BURST;
                if (flags != nullptr) *flags |= mBuffer[mHead].flags;
                const int first = mBuffer[mHead].first;

                int cnt = samplesCount - samplesFilled;
                const int cntbuf = mBuffer[mHead].last - first;
                cnt = cnt > cntbuf ? cntbuf : cnt;

                memcpy(&buffer[samplesFilled],&mBuffer[mHead].samples[first],cnt*sizeof(complex16_t));
                samplesFilled += cnt;

                if (cntbuf == cnt) //packet depleated
                {
                    mHead = (mHead + 1) & (mBufferSize - 1);//advance to next one
                    --mElementsFilled;
                }
                else
                    mBuffer[mHead].first += cnt;

                //leave the loop early when end of burst is encountered
                //so that the calling loop can flush out the buffer
                if (hasEOB) goto done;
            }
        }
        done:
        lck.unlock();
        hasItems.notify_one();
        return samplesFilled;
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lck(lock);
        mHead = 0;
        mTail = 0;
        mElementsFilled = 0;
    }

protected:
    const uint32_t mBufferSize;
    SamplesPacket* mBuffer;
    uint32_t mHead;
    uint32_t mTail;
    uint32_t mElementsFilled;
    std::mutex lock;
    std::condition_variable hasItems;
};

//https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template <typename T>
class ConcurrentQueue
{
private:
    std::queue<T> mQueue;
    std::mutex mMutex;
    std::condition_variable mCond;
public:
    void push(T const& data)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mQueue.push(data);
        lock.unlock();
        mCond.notify_one();
    }

    bool empty() const
    {
        std::unique_lock<std::mutex> lock(mMutex);
        return mQueue.empty();
    }

    bool try_pop(T& popped_value)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        if(mQueue.empty())
        {
            return false;
        }
        popped_value=mQueue.front();
        mQueue.pop();
        return true;
    }

    void wait_and_pop(T& popped_value)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        while(mQueue.empty())
        {
            mCond.wait(lock);
        }
        popped_value=mQueue.front();
        mQueue.pop();
    }

    bool wait_and_pop(T& popped_value, const int timeout_ms)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        while(mQueue.empty())
        {
            if (mCond.wait_for(lock, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout)
                return false;
        }
        popped_value=mQueue.front();
        mQueue.pop();
        return true;
    }
};

}
#endif
