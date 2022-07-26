#ifndef LMS_FIFO_BUFFER_H
#define LMS_FIFO_BUFFER_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <thread>
#include <queue>
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
        uint32_t overflow;
        uint32_t underflow;
    };

    enum StreamFlags
    {
        SYNC_TIMESTAMP = 1,
        END_BURST = 2,
    };

    //! @brief Returns information about FIFO size and fullness
    BufferInfo GetInfo()
    {
        BufferInfo stats;
        std::unique_lock<std::mutex> lck(lock);
        stats.size = mBufferSize*mPktSize;
        stats.itemsFilled = mElementsFilled*mPktSize;
        stats.overflow = mOverflow;
        stats.underflow = mUnderflow;
        mOverflow = 0;
        mUnderflow = 0;
        return stats;
    }

    //!    @brief Initializes FIFO memory
    RingFIFO() :  mBuffer(nullptr), mPktSize(0), mBufferSize(0)
    {
        Clear();
    }

    ~RingFIFO()
    {
        if (mBuffer)
            delete [] mBuffer;
    };

    void push_packet(SamplesPacket &packet)
    {
        std::unique_lock<std::mutex> lck(lock);

        if (mElementsFilled >= mBufferSize) //buffer might be full, wait for free slots
        {
                mHead = (mHead + 1) % mBufferSize;//advance to next one
                mElementsFilled--;
                mFirst = 0;
                mOverflow++;
        }

        mBuffer[mTail] = std::move(packet);
        mTail  = (mTail + 1) % mBufferSize;//advance to next one
        ++mElementsFilled;

        lck.unlock();
        hasItems.notify_one();
    }

    /** @brief inserts samples to FIFO, operation is thread-safe
    @param buffer pointer to array containing samples data
    @param samplesCount number of samples to insert from each buffer channel
    @param timeout_ms timeout duration for operation
    @param flags optional flags associated with the samples
    @return number of items inserted
    */
    uint32_t push_samples(const complex16_t *buffer, const uint32_t samplesCount, uint64_t timestamp, const uint32_t timeout_ms, const uint32_t flags)
    {
        assert(buffer != nullptr);
        uint32_t samplesTaken = 0;
        std::unique_lock<std::mutex> lck(lock);
        auto t1 = std::chrono::high_resolution_clock::now();
        while (samplesTaken < samplesCount)
        {
            if (mElementsFilled >= mBufferSize) //buffer might be full, wait for free slots
            {
                auto elapsed = std::chrono::high_resolution_clock::now()-t1;
                if(elapsed >= std::chrono::milliseconds(timeout_ms))
                    return samplesTaken;
                hasItems.wait_for(lck, std::chrono::milliseconds(timeout_ms)-elapsed);
            }
            else
            {
                mBuffer[mTail].timestamp = timestamp + samplesTaken - mLast;
                int cnt = samplesCount-samplesTaken;
                if (cnt > mPktSize - mLast)
                {
                    cnt = mPktSize - mLast;
                    mBuffer[mTail].flags = flags & SYNC_TIMESTAMP;
                }
                else
                    mBuffer[mTail].flags = flags;
                memcpy(mBuffer[mTail].samples + mLast,&buffer[samplesTaken],cnt*sizeof(complex16_t));
                samplesTaken+=cnt;
                mLast += cnt;
                mBuffer[mTail].last = mLast;
                if ((mLast == mPktSize) || (mBuffer[mTail].flags&END_BURST))
                {
                    mTail = (mTail+1) % mBufferSize;//advance to next one
                    ++mElementsFilled;
                    mLast = 0;
                }
            }
        }
        lck.unlock();
        hasItems.notify_one();
        return samplesTaken;
    }

    /** @brief Takes samples out of FIFO, operation is thread-safe
        @param buffer pointer to destination arrays for each channel samples data, each array must be big enough to contain \samplesCount number of samples.
        @param samplesCount number of samples to pop
        @param timestamp returns timestamp of the first sample in buffer
        @param timeout_ms timeout duration for operation
        @return number of samples popped
    */
    uint32_t pop_samples(complex16_t* buffer, const uint32_t samplesCount, uint64_t *timestamp, const uint32_t timeout_ms)
    {
        assert(buffer != nullptr);
        uint32_t samplesFilled = 0;
        std::unique_lock<std::mutex> lck(lock);
        while (samplesFilled < samplesCount)
        {
            while (mElementsFilled == 0) //buffer might be empty, wait for packets
            {
                if ((timeout_ms==0) || (hasItems.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout))
                {
                    mUnderflow++;
                    return samplesFilled;
                }
            }
            if(samplesFilled == 0 && timestamp != nullptr)
                *timestamp = mBuffer[mHead].timestamp + mFirst;

            while(mElementsFilled > 0 && samplesFilled < samplesCount)
            {
                int cnt = samplesCount - samplesFilled;
                const int cntbuf = mBuffer[mHead].last - mFirst;
                cnt = cnt > cntbuf ? cntbuf : cnt;

                memcpy(&buffer[samplesFilled],&mBuffer[mHead].samples[mFirst],cnt*sizeof(complex16_t));
                samplesFilled += cnt;

                if (cntbuf == cnt) //packet depleated
                {
                    mHead = (mHead + 1) % mBufferSize;//advance to next one
                    mFirst = 0;
                    --mElementsFilled;
                }
                else
                    mFirst += cnt;
            }
        }
        lck.unlock();
        hasItems.notify_one();
        return samplesFilled;
    }

    void pop_packet(SamplesPacket &packet)
    {
        std::unique_lock<std::mutex> lck(lock);

        while (mElementsFilled == 0) //buffer might be empty, wait for packets
            if (hasItems.wait_for(lck, std::chrono::milliseconds(100)) == std::cv_status::timeout)
            {
                mUnderflow++;
                packet.last = 0;
                packet.flags = 0;
                return;
            }

        packet = std::move(mBuffer[mHead]);
        mHead = (mHead + 1) % mBufferSize;//advance to next one
        --mElementsFilled;
        lck.unlock();
        hasItems.notify_one();
    }

    void Resize(int pktSize, int bufSize = -1)
    {
        Clear();
        std::unique_lock<std::mutex> lck(lock);
        if (bufSize < 0)
           bufSize =  mPktSize*mBufferSize/pktSize;

        if ((unsigned)bufSize == mBufferSize && pktSize == mPktSize)
            return;
        mBufferSize = bufSize;
        mPktSize = pktSize;
        if (mBuffer)
            delete [] mBuffer;

        mBuffer = bufSize == 0 ? nullptr : new SamplesPacket[mBufferSize];
        for (unsigned i = 0; i < mBufferSize; i++)
            mBuffer[i] = SamplesPacket(mPktSize);
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lck(lock);
        mHead = 0;
        mTail = 0;
        mFirst = 0;
        mLast = 0;
        mElementsFilled = 0;
        mOverflow = 0;
        mUnderflow = 0;
    }

protected:
    SamplesPacket* mBuffer;
    int32_t mPktSize;
    uint32_t mBufferSize;
    uint32_t mHead;
    uint32_t mTail;
    int32_t mFirst;
    int32_t mLast;
    uint32_t mElementsFilled;
    uint32_t mOverflow;
    uint32_t mUnderflow;
    std::mutex lock;
    std::condition_variable hasItems;
};

}
#endif
