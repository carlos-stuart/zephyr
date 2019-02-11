/*
 * Copyright (c) 2019 Carlos Stuart (carlosstuart1970@gmail.com)
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief zstd Mutex header
 *
 * Implementations of STL-like mutexes for use with STL functionality such as
 * std::lock_guard, std::unique_lock etc.
 *
 * CONFIG_ZSTD_MUTEX must be defined in Kconfig to include this implementation.
 *
 * This implementation supports dynamic allocation of mutexes when
 * CONFIG_ZSTD_NEW is also defined in Kconfig. This allocation can be either on
 * the system heap or on a memory slab the size of which is defined by the
 * Kconfig variable CONFIG_ZSTD_MUTEX_NEW_SLAB_MAX_COUNT. The choice of how
 * dynamic allocation is performed can be made by setting CONFIG_ZSTD_NEW_TYPE
 * to either CONFIG_ZSTD_NEW_HEAP or CONFIG_ZSTD_NEW_SLAB.
 *
 * Optionally by defining CONFIG_ZSTD_USING_AS_STD these mutex implementations
 * can be added to the std namespace. Note this should only be done on platforms
 * where there is no existing implementation in the std namespace e.g. when
 * using the GCC ARM Embedded toolchain.
 */

#ifndef ZEPHYR_INCLUDE_ZSTD_MUTEX_H_
#define ZEPHYR_INCLUDE_ZSTD_MUTEX_H_
#ifdef __cplusplus
#ifdef CONFIG_ZSTD_MUTEX

#include <kernel.h>

#include <chrono>
#include <exception>
#include <system_error>

namespace zstd
{

/*
 * Import functions from std namespace.
 */
using std::chrono::milliseconds;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::time_point;
using std::chrono::steady_clock;
using std::system_category;
using std::system_error;

/**
 * @brief A standard mutex
 *
 * A mutex implementation that satisifies all the C++ Mutex requirements i.e.
 * Lockable, DefaultConstructable, Destructable, not copyable and not moveable.
 */
class mutex
{
public:
	typedef struct k_mutex *native_handle_type;

	mutex();
	virtual ~mutex() = default;
	mutex(const mutex&) = delete;
	mutex& operator=(const mutex&) = delete;

#if defined(CONFIG_ZSTD_NEW_HEAP) || \
   (defined(CONFIG_ZSTD_NEW_SLAB) && CONFIG_ZSTD_MUTEX_NEW_SLAB_MAX_COUNT > 0)
	static void *operator new(size_t size);
	static void operator delete(void *ptr);
#endif

	void lock();
	bool try_lock();
	void unlock();

	native_handle_type native_handle();

protected:
	struct k_mutex m_mtx;
	struct k_mutex *const m_mtx_ptr;
}; /* class mutex */

/**
 * @brief A recursive mutex
 *
 * A recursive mutex implementation that satisifies all the C++ Mutex
 * requirements i.e. Lockable, DefaultConstructable, Destructable, not copyable
 * and not moveable.
 *
 * A recursive mutex may be locked multiple times by the same thread, however
 * it must also be unlocked an equal number of times before another thread may
 * take ownership.
 */
class recursive_mutex
{
public:
	typedef struct k_mutex *native_handle_type;

	recursive_mutex();
	virtual ~recursive_mutex() = default;
	recursive_mutex(const recursive_mutex&) = delete;
	recursive_mutex& operator=(const recursive_mutex&) = delete;

#if defined(CONFIG_ZSTD_NEW_HEAP) || \
   (defined(CONFIG_ZSTD_NEW_SLAB) && CONFIG_ZSTD_MUTEX_NEW_SLAB_MAX_COUNT > 0)
	void *operator new(size_t size);
	void operator delete(void *ptr);
#endif

	void lock();
	bool try_lock();
	void unlock();

	native_handle_type native_handle();

protected:
	struct k_mutex m_mtx;
	struct k_mutex *const m_mtx_ptr;
}; /* class recursive_mutex */

/**
 * @brief a timed mutex
 *
 * A timed mutex implementation that satisifies all the C++ Mutex requirements
 * i.e. Lockable, DefaultConstructable, Destructable, not copyable and not
 * moveable.
 *
 * A timed mutex can block for or until a specific time; if the mutex is not
 * obtained within this time the timed function return false.
 */
class timed_mutex : public mutex
{
public:
	timed_mutex();
	virtual ~timed_mutex() = default;
	timed_mutex(const timed_mutex&) = delete;
	timed_mutex& operator=(const timed_mutex&) = delete;

#if defined(CONFIG_ZSTD_NEW_HEAP) || \
   (defined(CONFIG_ZSTD_NEW_SLAB) && CONFIG_ZSTD_MUTEX_NEW_SLAB_MAX_COUNT > 0)
	void *operator new(size_t size);
	void operator delete(void *ptr);
#endif

	template <typename Rel, typename Period>
	bool try_lock_for(const duration<Rel, Period>& rel_time)
	{
		if ((m_mtx_ptr->lock_count > 0) &&
			(k_current_get() == m_mtx_ptr->owner)) {
			throw system_error(EDEADLK, system_category());
		}

		u64_t ms = duration_cast<milliseconds>(rel_time).count();

		int rv = k_mutex_lock(m_mtx_ptr, static_cast<u32_t>(ms));

		if (rv == 0) {
			return true;
		} else {
			return false;
		}
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(const time_point<Clock, Duration>& abs_time)
	{
		if ((m_mtx_ptr->lock_count > 0) &&
			(k_current_get() == m_mtx_ptr->owner)) {
			throw system_error(EDEADLK, system_category());
		}

		u64_t ms = duration_cast<milliseconds>(
				abs_time - steady_clock::now()).count();

		int rv = k_mutex_lock(m_mtx_ptr, static_cast<u32_t>(ms));

		if (rv == 0) {
			return true;
		} else {
			return false;
		}
	}

}; /* class timed_mutex */

/**
 * @brief A recursive timed mutex
 *
 * A recursive timed mutex implementation that satisifies all the C++ Mutex
 * requirements i.e. Lockable, DefaultConstructable, Destructable, not copyable
 * and not moveable.
 *
 * A recursive mutex may be locked multiple times by the same thread, however
 * it must also be unlocked an equal number of times before another thread may
 * take ownership.
 *
* A timed mutex can block for or until a specific time; if the mutex is not
 * obtained within this time the timed function return false.
 */
class recursive_timed_mutex : public recursive_mutex
{
public:
	recursive_timed_mutex();
	virtual ~recursive_timed_mutex() = default;
	recursive_timed_mutex(const recursive_timed_mutex&) = delete;
	recursive_timed_mutex& operator=(const recursive_timed_mutex&) = delete;

#if defined(CONFIG_ZSTD_NEW_HEAP) || \
   (defined(CONFIG_ZSTD_NEW_SLAB) && CONFIG_ZSTD_MUTEX_NEW_SLAB_MAX_COUNT > 0)
	void *operator new(size_t size);
	void operator delete(void *ptr);
#endif

	template <typename Rel, typename Period>
	bool try_lock_for(const duration<Rel, Period>& rel_time)
	{
		u64_t ms = duration_cast<milliseconds>(rel_time).count();

		int rv = k_mutex_lock(m_mtx_ptr, static_cast<u32_t>(ms));

		if (rv == 0) {
			return true;
		} else {
			return false;
		}
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(const time_point<Clock, Duration>& abs_time)
	{
		u64_t ms = duration_cast<milliseconds>(
				abs_time - steady_clock::now()).count();

		int rv = k_mutex_lock(m_mtx_ptr, static_cast<u32_t>(ms));

		if (rv == 0) {
			return true;
		} else {
			return false;
		}
	}

}; /* class recursive_timed_mutex */

} /* namespace zstd */

/*
 * Using zstd as std (STL)?
 */
#ifdef CONFIG_ZSTD_USING_AS_STD
namespace std
{
	using mutex                 = zstd::mutex;
	using recursive_mutex       = zstd::recursive_mutex;
	using timed_mutex           = zstd::timed_mutex;
	using recursive_timed_mutex = zstd::recursive_timed_mutex;
}
#endif

#endif /* CONFIG_ZSTD_MUTEX */
#endif /* __cplusplus */
#endif /* ZEPHYR_INCLUDE_ZSTD_MUTEX_H_*/
