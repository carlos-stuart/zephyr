#ifndef ZEPHYR_INCLUDE_ZSTD_THREAD_H_
#define ZEPHYR_INCLUDE_ZSTD_THREAD_H_
#ifdef __cplusplus
#ifdef CONFIG_ZSTD_THREAD

#include <kernel.h>

#include <chrono>

namespace zstd
{

using std::chrono::milliseconds;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::time_point;
using std::chrono::steady_clock;

/*******************************************************************************
 * zstd::thread
 ******************************************************************************/
class thread
{
public:
	typedef k_tid_t native_handle_type;

	class id
	{
	public:
		id() noexcept;
		explicit id(native_handle_type id) noexcept;

	private:
		native_handle_type m_id;

		friend class thread;
	};

	thread() noexcept;

	template <class F, class ...Args>
	explicit thread(F&& f, Args&&... args);

	~thread();

	thread(const thread&) = delete;
	thread(thread&&) noexcept;

	thread& operator=(const thread&) = delete;
	thread& operator=(thread&&) noexcept;

	void swap(thread&) noexcept;
	bool joinable() const noexcept;
	void join();
	void detach();
	id get_id() const noexcept;
	native_handle_type native_handle();

	static unsigned int hardware_concurrency() noexcept;

private:
	id m_id;

}; /* class thread */

/*******************************************************************************
 * zstd::this_thread
 ******************************************************************************/
namespace this_thread
{
	thread::id get_id() noexcept;
	void yield() noexcept;

	template <class Rep, class Period>
	void sleep_for(const duration<Rep, Period>& rel_time)
	{
		u64_t ms = duration_cast<milliseconds>(rel_time).count();

		k_sleep(static_cast<u32_t>(ms));
	}

	template <class Clock, class Duration>
	void sleep_until(const time_point<Clock, Duration>& abs_time)
	{
		u64_t ms = duration_cast<milliseconds>(
				abs_time - steady_clock::now()).count();

		k_sleep(static_cast<u32_t>(ms));
	}

} /* namespace this_thread */

} /* namespace zstd */

/*******************************************************************************
 * Using zstd as std (STL)?
 ******************************************************************************/
#ifdef CONFIG_ZSTD_USING_AS_STD
namespace std
{
	using thread = zstd::thread;

	namespace this_thread = zstd::this_thread;
}
#endif

#endif /* CONFIG_ZSTD_THREAD */
#endif /* __cplusplus */
#endif /* ZEPHYR_INCLUDE_ZSTD_THREAD_H_*/
