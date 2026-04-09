#include "ThreadPool.h"

void ThreadPool::manager()
{
	while (!m_stop.load())
	{
		this_thread::sleep_for(chrono::seconds(3));
		int idel = m_idlethreads.load();
		int cur = m_curthreads.load();

		if (idel > cur / 2 && cur > m_minthreads)
		{
			m_exitthreads = 2;
			m_condition.notify_all();
			{
				lock_guard<mutex> lock(m_idsMutex);
				for (auto id : m_ids)
				{
					auto it = m_works.find(id);
					if (it != m_works.end())
					{
						(*it).second.join();
						m_works.erase((*it).first);
						cout << "-------------线程" << (*it).first << "销毁了" << endl;
					}
				}

				m_ids.clear();
			}
		}

		if (idel == 0 && cur < m_maxthreads)
		{
			thread t(&ThreadPool::worker,this);
			m_works.emplace(t.get_id(), std::move(t));
		}

	}
}

void ThreadPool::worker()
{
	while (!m_stop.load())
	{
		function<void()> task;

		{
			unique_lock<mutex> lock(m_queueMutex);
			m_condition.wait(lock, [this]() {
				return !m_tasks.empty() || m_stop || m_exitthreads > 0;
				});
			if (m_exitthreads > 0)
			{
				m_curthreads--;
				m_exitthreads--;
				cout << "-------------线程退出了，ID: " << this_thread::get_id() << endl;
				{
					lock_guard<mutex> lock(m_idsMutex);
					m_ids.emplace_back(this_thread::get_id());
				}
				return;
			}
			task = std::move(m_tasks.front());
			m_tasks.pop();
		}

		if (task)
		{
			m_idlethreads--;
			task();
			m_idlethreads++;
		}

	}
}

ThreadPool::ThreadPool(int min, int max):m_maxthreads(max),m_minthreads(min),m_stop(false),
m_idlethreads(min),m_curthreads(min)
{
	//创建管理者线程
	m_manager = make_shared<thread>(&ThreadPool::manager, this);

	//创建生产者线程
	for (int i = 0;i < min;i++)
	{
		thread t(&ThreadPool::worker, this);
		m_works.emplace(t.get_id(), std::move(t));
	}
}

void ThreadPool::addTask(function<void()> task)
{
	{
		unique_lock<mutex> lock(m_queueMutex);
		m_tasks.emplace(task);
	}
	m_condition.notify_one();
}
