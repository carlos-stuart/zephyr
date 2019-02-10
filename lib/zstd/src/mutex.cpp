/*
 * Copyright (c) 2019 Carlos Stuart (carlosstuart1970@gmail.com)
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief zstd Mutex source
 */

#ifdef CONFIG_ZSTD_MUTEX
#include <zstd/mutex.h>

namespace zstd
{

/*
 * Import functions from std namespace.
 */
using std::bad_alloc;

/*
 * Slab based new/delete configuration.
 */
#ifdef CONFIG_ZSTD_NEW_SLAB

#define MAX(a, b)      ((a > b) ? a : b)
#define MAX_SIZE(a, b) MAX(sizeof(a), sizeof(b))

#define MAX_MUTEX_SIZE_NON_TIMED MAX_SIZE(mutex, recursive_mutex)
#define MAX_MUTEX_SIZE_TIMED     MAX_SIZE(timed_mutex, recursive_timed_mutex)

#define MAX_MUTEX_SIZE MAX(MAX_MUTEX_SIZE_NON_TIMED, MAX_MUTEX_SIZE_TIMED)

static K_MEM_SLAB_DEFINE(mutex_slab, MAX_MUTEX_SIZE, \
                  CONFIG_ZSTD_MUTEX_NEW_SLAB_MAX_COUNT, 1);

#endif /* CONFIG_ZSTD_NEW_SLAB */

/**
 * @brief default mutex constructor
 */
mutex::mutex() : m_mtx_ptr{&m_mtx}
{
	k_mutex_init(&m_mtx);
}

#ifdef CONFIG_ZSTD_NEW_HEAP

/**
 * @brief heap based new operator
 */
void *mutex::operator new(size_t size)
{
	void *ptr = k_calloc(1, size);

	if (ptr == nullptr) {
		throw bad_alloc();
	}

	return ptr;
}

/**
 * @brief heap based delete operator
 */
void mutex::operator delete(void *ptr)
{
	free(ptr);
}

#elif CONFIG_ZSTD_NEW_SLAB

/**
 * @brief slab based new operator
 */
void *mutex::operator new(size_t size)
{
	void *ptr = nullptr;

	int rv = k_mem_slab_alloc(&mutex_slab, &ptr, K_NO_WAIT);

	if (rv != 0) {
		throw bad_alloc();
	}

	return ptr;
}

/**
 * @brief slab based delete operator
 */
void mutex::operator delete(void *ptr)
{
	k_mem_slab_free(&mutex_slab, &ptr);
}

#endif /* CONFIG_ZSTD_NEW_SLAB */

/**
 * @brief block until the lock is obtained
 */
void mutex::lock()
{
	if ((m_mtx_ptr->lock_count > 0) &&
		(k_current_get() == m_mtx_ptr->owner)) {
		throw system_error(EDEADLK, system_category());
	}

	int rv = k_mutex_lock(m_mtx_ptr, K_FOREVER);

	if (rv != 0) {
		throw system_error(-rv, system_category());
	}
}

/**
 * @brief try to lock the mutex but return reurn immediately if already locked
 */
bool mutex::try_lock()
{
	if ((m_mtx_ptr->lock_count > 0) &&
		(k_current_get() == m_mtx_ptr->owner)) {
		throw system_error(EDEADLK, system_category());
	}

	int rv = k_mutex_lock(m_mtx_ptr, 0);

	if (rv == 0) {
		return true;
	} else {
		return false;
	}
}

void mutex::unlock()
{
	k_mutex_unlock(m_mtx_ptr);
}

struct k_mutex *mutex::native_handle()
{
	return m_mtx_ptr;
}

/*******************************************************************************
 * zstd::recursive_mutex
 ******************************************************************************/
recursive_mutex::recursive_mutex() : m_mtx_ptr{&m_mtx}
{
	k_mutex_init(&m_mtx);
}

#ifdef CONFIG_ZSTD_NEW_HEAP
void *recursive_mutex::operator new(size_t size)
{
	void *ptr = k_calloc(1, size);

	if (ptr == nullptr) {
		throw bad_alloc();
	}

	return ptr;
}

void recursive_mutex::operator delete(void *ptr)
{
	free(ptr);
}
#elif CONFIG_ZSTD_NEW_SLAB
void *recursive_mutex::operator new(size_t size)
{
	void *ptr = nullptr;

	int rv = k_mem_slab_alloc(&mutex_slab, &ptr, K_NO_WAIT);

	if (rv != 0) {
		throw bad_alloc();
	}

	return ptr;
}

void recursive_mutex::operator delete(void *ptr)
{
	k_mem_slab_free(&mutex_slab, &ptr);
}
#endif

void recursive_mutex::lock()
{
	int rv = k_mutex_lock(m_mtx_ptr, K_FOREVER);

	if (rv != 0) {
		throw system_error(-rv, system_category());
	}
}

bool recursive_mutex::try_lock()
{
	int rv = k_mutex_lock(m_mtx_ptr, 0);

	if (rv == 0) {
		return true;
	} else {
		return false;
	}
}

void recursive_mutex::unlock()
{
	k_mutex_unlock(m_mtx_ptr);
}

struct k_mutex *recursive_mutex::native_handle()
{
	return m_mtx_ptr;
}

/*******************************************************************************
 * zstd::timed_mutex
 ******************************************************************************/
timed_mutex::timed_mutex() : mutex()
{};

#ifdef CONFIG_ZSTD_NEW_HEAP
void *timed_mutex::operator new(size_t size)
{
	void *ptr = k_calloc(1, size);

	if (ptr == nullptr) {
		throw bad_alloc();
	}

	return ptr;
}

void timed_mutex::operator delete(void *ptr)
{
	free(ptr);
}
#elif CONFIG_ZSTD_NEW_SLAB
void *timed_mutex::operator new(size_t size)
{
	void *ptr = nullptr;

	int rv = k_mem_slab_alloc(&mutex_slab, &ptr, K_NO_WAIT);

	if (rv != 0) {
		throw bad_alloc();
	}

	return ptr;
}

void timed_mutex::operator delete(void *ptr)
{
	k_mem_slab_free(&mutex_slab, &ptr);
}
#endif

/*******************************************************************************
 * zstd::recursive_timed_mutex
 ******************************************************************************/
recursive_timed_mutex::recursive_timed_mutex() : recursive_mutex()
{};

#ifdef CONFIG_ZSTD_NEW_HEAP
void *recursive_timed_mutex::operator new(size_t size)
{
	void *ptr = k_calloc(1, size);

	if (ptr == nullptr) {
		throw bad_alloc();
	}

	return ptr;
}

void recursive_timed_mutex::operator delete(void *ptr)
{
	free(ptr);
}
#elif CONFIG_ZSTD_NEW_SLAB
void *recursive_timed_mutex::operator new(size_t size)
{
	void *ptr = nullptr;

	int rv = k_mem_slab_alloc(&mutex_slab, &ptr, K_NO_WAIT);

	if (rv != 0) {
		throw bad_alloc();
	}

	return ptr;
}

void recursive_timed_mutex::operator delete(void *ptr)
{
	k_mem_slab_free(&mutex_slab, &ptr);
}
#endif

} /* namespace zstd */

#endif /* CONFIG_ZSTD_MUTEX */
