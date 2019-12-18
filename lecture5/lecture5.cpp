/*********************************************************
** 课时5大纲：创建多个线程，数据共享问题分析，案例代码
** （1）创建和等待多个线程
** （2）数据共享问题分析
** （2.1）只读的数据
** （2.2）有读有写
** （2.3）其他案例
** （3）共享数据的保护案例代码
*********************************************************/


/*********************************************************
** 课时6大纲：创建多个线程，数据共享问题分析，案例代码
** （1）创建和等待多个线程
** （2）数据共享问题分析
** （2.1）只读的数据
** （2.2）有读有写
** （2.3）其他案例
** （3）共享数据的保护案例代码
*********************************************************/


#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <iostream>
#include <vector>

using namespace std;

//vector<int> g_v = { 1,2,3 }; //共享数据，只读

//线程入口函数
//void myprint(int inum)
//{
//	//cout << "myprint线程开始执行了，线程编号=" << inum << endl;
//	////...干各种事情
//	//cout << "myprint线程结束执行了，线程编号=" << inum << endl;
//
//	cout << "id为" << std::this_thread::get_id() << "的线程 打印g_v值" << g_v[0] << g_v[1] << g_v[2] << endl;
//	return;
//}

class A
{
public:
	//把收到的消息（玩家命令）入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			cout << "inMsgRecvQueue()执行，插入一个元素" << i << endl;

			//{
			std::lock_guard<std::mutex> sbguard1(my_mutex1);
			std::lock_guard<std::mutex> sbguard2(my_mutex2);
			//my_mutex1.lock(); //实际工程中这两个锁头不一定挨着，可能他们需要保护不同的数据共享块；
			//my_mutex2.lock();
			msgRecvQueue.push_back(i); //假设这个数字i就是我收到的命令，我直接弄到消息队列里边来;
			//my_mutex2.unlock();
			//my_mutex1.unlock();
			//}
			//my_mutex.unlock();
			//....
			//其他处理代码
		}
		return;
	}

	bool outMsgLULProc(int &command)
	{
		std::lock_guard<std::mutex> sbguard1(my_mutex1); //sbguard是老师随便起的对象名
													   //lock_guard构造函数里执行了mutex::lock();
													   //lock_guard析构函数里执行了mutex::unlock();
		std::lock_guard<std::mutex> sbguard2(my_mutex2);
		//my_mutex1.lock();
		//my_mutex2.lock();
		if (!msgRecvQueue.empty())
		{
			//消息不为空
			int command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在；
			msgRecvQueue.pop_front(); //移除第一个元素，但不返回；
			//my_mutex1.unlock(); //注释掉会在这里卡住，报异常；
			//my_mutex2.unlock();
			return true;
		}
		//my_mutex1.unlock();
		//my_mutex2.unlock();
		return false;
	}

	//把数据从消息队列中取出的线程；
	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; i++)
		{
			bool result = outMsgLULProc(command);
			if (result == true)
			{
				cout << "outMsgRecvQueue()执行，取出一个元素" << command << endl;
				//可以考虑进行命令（数据）处理
				//....
			}
			else
			{
				//消息队列为空
				cout << "outMsgRecvQueue()执行，但目前消息队列中为空" << i << endl;
			}
		}
		cout << "end" << endl;
	}

private:
	std::list<int> msgRecvQueue; //容器（消息队列），专门用于代表玩家给咱们发送过来的命令。
	std::mutex my_mutex1; //创建了一个互斥量（一把锁头）
	std::mutex my_mutex2; //创建了一个互斥量
};

int main()
{
	//一：创建和等待多个线程

	//vector<thread> mythreads;
	//创建10个线程，每个线程使用统一的入口函数myprint。
	//a)多个线程执行顺序是乱的，跟操作系统内部对线程的运行调度机制有关；
	//b)主线程等待所有子线程运行结束，最后主线程结束，老师推荐这种join()的写法，更容易写出稳定的程序；
	//c)咱们把thread对象放入到容器里管理，看起来像个thread对象数组，这对我们一次创建大量的线程并对大量线程进行管理很方便。
	//for (int i = 0; i < 10; i++)
	//{
	//	mythreads.push_back(thread(myprint, i)); //创建10个线程，同时这10个线程已经开始执行
	//}
	//for (auto iter = mythreads.begin(); iter != mythreads.end(); iter++)
	//{
	//	iter->join(); //等待10个线程都返回
	//}
	//cout << "I Love China!" << endl; //最后执行这句，整个进程退出

	//二：数据共享问题分析
	//（2.1）只读的数据：是安全稳定的，不需要什么特别处理手段。直接读就可以；
	//（2.2）有读有写:2个线程写，8个线程读，如果代码没有特别的处理，那程序肯定崩溃；
		//最简单的不崩溃处理就是，读的时候不能写，写的时候不能读。2个线程不能同时写，8个线程不能同时读；
		//写的动作分10小步；由于任务切换，导致各种诡异事情发生（最可能的诡异事情还是程序崩溃）；
	//（2.3）其他案例

	//三：共享数据的保护案例代码
	//网络游戏服务器。两个自己创建的线程，一个线程负责收集玩家命令（用一个数字代表玩家发来的命令），并把命令数据写到一个队列中。
		//另外一个线程，从队列中取出玩家发送来的命令，解析，然后执行玩家需要的动作；
	//vector和list list：频繁地按顺序插入和删除数据时效率高；vector容器随机地插入和删除数据效率高。
	//准备用成员函数作为线程函数的方法来写线程；
	//代码化解决问题：老师， 引入一个c++解决多线程保护共享数据问题的第一个概念“互斥量”，往脑袋里记这个词；


	A myobja;
	std::thread myOutMsgObj(&A::outMsgRecvQueue, &myobja); //第二个参数是引用,才能保证线程里用的是同一个对象；
	std::thread myInMsgObj(&A::inMsgRecvQueue, &myobja);
	myOutMsgObj.join();
	myInMsgObj.join();

	//保护共享数据，操作时，某个线程用代码把共享数据锁住，操作，解锁；
		//其他想操作共享数据的线程必须等待解锁，锁定住，操作，解锁。
	//“互斥量”
	//一：互斥量（mutex）的基本概念
	//互斥量是个类对象。理解成一把锁，多个线程尝试用lock()成员函数来加锁这把锁头，只有一个线程能锁定成功（成功的标志是lock()函数返回）
				//如果没锁成功，那么流程卡在lock()这里不断地尝试去锁这把锁头；
	//互斥量使用要小心，保护数据不多也不少，少了，没达到保护效果，多了，影响效率；

	//二：互斥量的用法
	//(2.1)lock(),unlock()
	//步骤：先lock(),操作共享数据，unlock()；
	//lock()和unlock()要成对使用，有lock()必然要有unlock()，每调用一次lock()，必然应该调用一次unlock()；
		//不应该也不允许调用一次lock()却调用两次unlock()，也不允许调用两次lock()却只调用一次unlock()；这些非对称数量的调用都会导致代码不稳定甚至崩溃。
	//有lock，忘记unlock的问题，非常难排查；
	//为了防止大家忘记unlock()，引入了一个叫std::lock_guard的类模板；你忘记unlock()不要紧，我替你unlock()；
	//学习过智能指针(unique_ptr<>)；你忘记释放内存不要紧，我替你释放；	保姆；

	//（2.2）std::lock_guard类模板:直接取代lock()和unlock()；也就是说，你用了lock_guard之后，就不能使用lock()和unlock()了。


	//三：死锁
	//张三：站在北京 等李四，不挪窝；
	//李四：站在深圳 等张三，不挪窝；
	//c++中：
	//比如我有两把锁（死锁这个问题 是由至少两个锁头也就是两个互斥量才能产生）；金锁（JinLock），银锁（YinLock）；
	//两个线程A，B
	//（1）线程A执行的时候，这个线程先锁 金锁，把金锁lock()成功了，然后它去lock()银锁。。。
	//出现上下文切换
	//（2）线程B执行了，这个线程先锁 银锁，因为银锁还没有被锁，所以银锁会lock()成功，线程B要去lock()金锁。。。
	//此时此刻，死锁就产生了；
	//（3）线程A因为拿不到银锁头，流程走不下去（所有后边代码有解锁金锁头的但是流程走不下去，所以金锁头解不开）
	//（4）线程B因为拿不到金锁头，流程走不下去（所有后边代码有解锁银锁头的但是流程走不下去，所以银锁头解不开）
	//大家都晾在这里，你等我，我等你；

	//(3.1)死锁演示
	//(3.2)死锁的一般解决方案
	//只要保证这两个互斥量上锁的顺序一致就不会死锁。
	//(3.3)std::lock()函数模板
	//能力：一次锁住两个或者两个以上的互斥量（至少两个，多了不限，1个不行）；
	//它不存在这种因为在多个线程中 因为锁的顺序问题导致死锁的风险问题；
	//std::lock():如果互斥量有一个没锁住，他就在那里等着，等所有互斥量都锁住，他才能往下走（返回）；
	//要么两个互斥量都锁住，要么两个互斥量都没锁住。如果只锁了一个，另外一个没锁成功，则它立即把已经锁住的解锁。






	return 0;
}