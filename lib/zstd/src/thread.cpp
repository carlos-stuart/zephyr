#ifdef CONFIG_ZSTD_THREAD
#include <zstd/thread.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(zstd_thread, CONFIG_ZSTD_LOG_LEVEL);

namespace zstd
{

/*******************************************************************************
 * Slab based new/delete?
 ******************************************************************************/
#ifdef CONFIG_ZSTD_NEW_SLAB

static K_MEM_SLAB_DEFINE(thread_slab, sizeof(thread), \
                  CONFIG_ZSTD_THREAD_NEW_SLAB_MAX_COUNT, 1);

#endif

/*******************************************************************************
 * Thread stack pool
 ******************************************************************************/
/* Not sure why max isn't already defined? */
#define max(a, b) ((a > b) ? a : b)
static K_THREAD_STACK_ARRAY_DEFINE(thread_stack_pool,		 \
				   CONFIG_ZSTD_THREAD_MAX_COUNT, \
				   CONFIG_ZSTD_THREAD_STACK_SIZE);
#undef max

/*******************************************************************************
 * zstd::thread::id
 ******************************************************************************/
thread::id::id() noexcept : m_id{0}
{}

thread::id::id(native_handle_type id) noexcept : m_id{id}
{}

/*******************************************************************************
 * zstd::thread
 ******************************************************************************/
unsigned int thread::hardware_concurrency() noexcept
{
	return CONFIG_ZSTD_THREAD_MAX_COUNT;
}

/*******************************************************************************
 * zstd::this_thread
 ******************************************************************************/
namespace this_thread
{
	thread::id get_id() noexcept
	{
		return thread::id(k_current_get());
	}

	void yield() noexcept
	{
		k_yield();
	}

} /* namespace this_thread */

} /* namespace zstd */

#endif /* CONFIG_ZSTD_THREAD */
