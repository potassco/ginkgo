#ifndef __UTILS__SEMAPHORE_H
#define __UTILS__SEMAPHORE_H

#include <mutex>
#include <condition_variable>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Semaphore
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Semaphore
{
	public:
		Semaphore();

		void notify();
		void notifyFinished();

		void wait();
		std::cv_status waitFor(const std::chrono::milliseconds &relativeTime);

		size_t count() const;
		bool isActive() const;

		void reset();

	private:
		mutable std::mutex m_mutex;
		std::condition_variable m_condition;

		size_t m_count;
		bool m_isActive;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
