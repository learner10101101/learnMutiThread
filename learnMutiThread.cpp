// learnMutiThread.cpp: 定义应用程序的入口点。
//

#include "learnMutiThread.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <deque>
#include <queue>
#include<condition_variable>

using namespace std;

// 多线程应用：生产消费者模式
// 生产者：
// （1）生产数据
// （2）把数据放入缓存队列
// （3）向消费者线程发出通知
// 消费者：
// （1）将数据从缓存队列中取出
// （2）处理数据
class AA//生产消费者类
{
	mutex m_mutex;//互斥锁
	condition_variable m_cond;//条件变量
	queue<string, deque<string>> m_q;//缓存队列，底层容器用deque

public:
	void incache(int num)//生产数据，num指定数据的个数
	{
		lock_guard<mutex> lock(m_mutex);//申请加锁
		for (int ii = 0; ii < num; ii++)
		{
			static int bh = 1;//超女编号
			string message = to_string(bh++) + "号超女";//拼接出一个数据
			m_q.push(message);//把生产出来的数据入队
		}

		//两种通知函数的使用场合：如果每一次生产的数据只有一个，应该使用notify_one()；如果每一次生产的数据可能有多个，应该使用notify_all()
		//m_cond.notify_one();//唤醒一个被当前条件变量阻塞的线程
		m_cond.notify_all();//唤醒全部被当前条件变量阻塞的线程
	}

	void outcache()//消费者线程任务函数
	{
		string message;//存放出队的数据
		while (true)
		{
			string message;
			{//
				//把互斥锁转换成unique_lock<mutex>，并申请加锁
				unique_lock<mutex> lock(m_mutex);

// 				//条件变量虚假唤醒：消费者线程被唤醒后，缓存队列中没有数据
// 				while (m_q.empty())//如果队列非空，不进入循环，直接处理数据，必须用循环，不能用if，防止虚假唤醒
// 				{
// 					m_cond.wait(lock);//等待生产者的唤醒信号//(1)把互斥锁解锁；（2）阻塞，等待被唤醒；（3）给互斥锁加锁，wait()函数返回，执行后续操作，所以需要搭配unique_lock使用，解锁wait()函数加的锁
// 				}
				m_cond.wait(lock, [this] {return !m_q.empty(); });//上面while防止虚假唤醒的等效写法

				//数据元素出队
				message = m_q.front();
				m_q.pop();
				cout << "线程：" << this_thread::get_id() << "，" << message << endl;
			}

			//处理出队的数据（把数据消费掉），这里使用延时函数表示处理数据过程
			this_thread::sleep_for(chrono::milliseconds(1));//假设处理数据需要1毫秒
		}

	}
};

int main()
{
#if 1
	//测试场景：在主程序中生产数据，在t1,t2,t3中消耗数据
	AA aa;

	thread t1(&AA::outcache, &aa);//创建消费者线程t1
	thread t2(&AA::outcache, &aa);//创建消费者线程t2
	thread t3(&AA::outcache, &aa);//创建消费者线程t3

	this_thread::sleep_for(chrono::seconds(2));
	aa.incache(2);//生产3个数据

	this_thread::sleep_for(chrono::seconds(2));
	aa.incache(5);//生产5个数据

	t1.join();
	t2.join();
	t3.join();
#endif

	return 0;
}
