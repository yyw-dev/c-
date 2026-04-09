#pragma once

#include<iostream>
#include<queue>
#include<thread>
#include<memory>
#include<vector>
#include<mutex>
#include<functional>
#include<atomic>
#include<condition_variable>
#include<unordered_map>
using namespace std;

class ThreadPool
{
public:
	ThreadPool(int min = 2, int max = thread::hardware_concurrency());
	~ThreadPool();
	void manager();
	void worker();

	void addTask(function<void()> task);
private:
	shared_ptr<thread> m_manager;
	vector<thread::id> m_ids;
	unordered_map<thread::id,thread> m_works;
	atomic<int> m_minthreads;
	atomic<int> m_maxthreads;
	atomic<int> m_curthreads;
	atomic<int>	m_idlethreads;
	atomic<int> m_exitthreads;
	atomic<bool> m_stop;
	queue<function<void()>> m_tasks;
	mutex m_queueMutex;
	mutex m_idsMutex;
	condition_variable m_condition;
};