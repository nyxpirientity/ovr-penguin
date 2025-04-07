#ifndef MUTEX_HPP
#define MUTEX_HPP
#include <mutex>
#include <shared_mutex>
#include <cassert>

namespace nyxpiri::ovrpenguin
{
template<typename DataType>
class Mutex
{
public:
    Mutex() {}

    class Guard
    {
    public:
        ~Guard()
        {
            unlock();
        }

        Guard() = default;
        Guard(Guard&&) = default;
        Guard& operator=(Guard&&) = default;

        void unlock()
        {
            if (!mutex)
            {
                return;
            }

            mutex->unlock(*this);
            mutex = nullptr;
        }

        bool has_lock()
        {
            return mutex;
        }

        DataType& data()
        {
            return mutex->data;
        }

    private:
        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;
        friend Mutex;
        Mutex<DataType>* mutex = nullptr;
    };

    [[nodiscard]] Guard lock()
    {
        mutex.lock();
        Guard guard;
        guard.mutex = this;
        return guard;
    }

private:
    void unlock(Guard& guard)
    {
        assert(guard.mutex == this);
        guard.mutex = nullptr;
        mutex.unlock();
    }

private:
    friend Guard;
    std::mutex mutex;
    DataType data;
};

template<typename DataType>
class SharedMutex
{
public:
    SharedMutex() {}

    class WriteGuard
    {
    public:
        ~WriteGuard()
        {
            unlock();
        }

        WriteGuard() = default;
        WriteGuard(WriteGuard&&) = default;
        WriteGuard& operator=(WriteGuard&&) = default;

        void unlock()
        {
            if (!mutex)
            {
                return;
            }

            mutex->write_unlock(*this);
            mutex = nullptr;
        }

        bool has_lock()
        {
            return mutex;
        }

        DataType& data()
        {
            assert(mutex);
            return mutex->data;
        }

    private:
        WriteGuard(const WriteGuard&) = delete;
        WriteGuard& operator=(const WriteGuard&) = delete;
        friend SharedMutex;
        SharedMutex<DataType>* mutex = nullptr;
    };

    class ReadGuard
    {
    public:
        ~ReadGuard()
        {
            unlock();
        }

        ReadGuard() = default;
        ReadGuard(ReadGuard&&) = default;
        ReadGuard& operator=(ReadGuard&&) = default;

        void unlock()
        {
            if (!mutex)
            {
                return;
            }

            mutex->read_unlock(*this);
            mutex = nullptr;
        }

        bool has_lock()
        {
            return mutex;
        }

        const DataType& data()
        {
            return mutex->data;
        }

    private:
        ReadGuard(const WriteGuard&) = delete;
        ReadGuard& operator=(const ReadGuard&) = delete;
        friend SharedMutex;
        SharedMutex<DataType>* mutex = nullptr;
    };

    [[nodiscard]] WriteGuard write_lock()
    {
        mutex.lock();
        WriteGuard guard;
        guard.mutex = this;
        return guard;
    }

    [[nodiscard]] ReadGuard read_lock()
    {
        mutex.lock_shared();
        ReadGuard guard;
        guard.mutex = this;
        return guard;
    }

private:
    void write_unlock(WriteGuard& guard)
    {
        assert(guard.mutex == this);
        guard.mutex = nullptr;
        mutex.unlock();
    }

    void read_unlock(ReadGuard& guard)
    {
        assert(guard.mutex == this);
        guard.mutex = nullptr;
        mutex.unlock_shared();
    }

private:
    friend WriteGuard;
    std::shared_mutex mutex;
    DataType data;
};

} // namespace nyxpiri::ovrpenguin

#endif // MUTEX_HPP
