#### 一、互斥量和条件变量简介

　　互斥量(**mutex**)从本质上说是一把锁，在访问共享资源前对互斥量进行加锁，在访问完成后释放互斥量上的锁。在互斥量进行加锁以后，任何其它试图再次对互斥量加锁的线程将会阻塞直到当前线程释放该[互斥锁](https://so.csdn.net/so/search?q=%E4%BA%92%E6%96%A5%E9%94%81&spm=1001.2101.3001.7020)。如果释放互斥锁时有多个线程阻塞，所有在该互斥锁上的阻塞线程都会变成可运行状态，第一个变为可运行状态的线程可以对互斥锁加锁，其它线程将会看到互斥锁依然被锁住，只能回去再次等待它重新变为可用。

　　条件变量(**cond**)是在多线程程序中用来实现“等待--》唤醒”逻辑的常用的方法。条件变量是利用线程间共享的全局变量进行同步的一种机制，主要包括两个动作：一个线程等待“条件变量的条件成立”而挂起；另一个线程使“条件成立”而发出信号。条件变量的使用总是和一个互斥锁结合在一起。线程在改变条件状态前用**pthread\_mutex\_lock()**必须首先锁住互斥量，在更新条件等待队列以前，**mutex**保持锁定状态，并在线程挂起进入等待前解锁，在条件满足(**pthread\_cond\_wait()**有返回值)后，**mutex**将被重新加锁，以与进入**pthread\_cond\_wait()**前的加锁动作一样。这个过程可以表示为："**block-->unlock-->wait() return-->lock**"。这**pthread\_mutex\_lock()**和**pthread\_cond\_wait()**是原子操作（所谓原子操作是指不会被线程调度机制打断的操作；这种操作一旦开始，就一直运行到结束，中间不会有任何context switch(切换到另一个线程)）。

 **1、创建和注销**     
       条件变量和互斥锁一样，都有静态动态两种创建方式，静态方式使用PTHREAD\_COND\_INITIALIZER常量，如下：     

```cpp
  pthread_cond_t   cond=PTHREAD_COND_INITIALIZER
```

      动态方式调用pthread\_cond\_init()函数，API定义如下：     

```cpp
  int   pthread_cond_init(pthread_cond_t   *cond,   pthread_condattr_t   *cond_attr)
```

      尽管POSIX标准中为条件变量定义了属性，但在LinuxThreads中没有实现，因此cond\_attr值通常为NULL，且被忽略。     
      注销一个条件变量需要调用pthread\_cond\_destroy()，只有在没有线程在该条件变量上等待的时候才能注销这个条件变量，否则返回EBUSY。因为Linux实现的条件变量没有分配什么资源，所以注销动作只包括检查是否有等待线程。API定义如下：

```cpp
  int   pthread_cond_destroy(pthread_cond_t   *cond)
```

   **2、等待和激发**  

```cpp
int   pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)   

int   pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)    
```

       等待条件有两种方式：无条件等待pthread\_cond\_wait()和计时等待pthread\_cond\_timedwait()，其中计时等待方式如果在给定时刻前条件没有满足，则返回ETIMEOUT，结束等待，其中abstime以与time()系统调用相同意义的绝对时间形式出现，0表示格林尼治时间1970年1月1日0时0分0秒。    
       无论哪种等待方式，都必须和一个互斥锁配合，以防止多个线程同时请求pthread\_cond\_wait(（pthread\_cond\_timedwait()，下同）的竞争条件（Race   Condition）。mutex互斥锁必须是普通锁（PTHREAD\_MUTEX\_TIMED\_NP）或者适应锁（PTHREAD\_MUTEX\_ADAPTIVE\_NP），且在调用pthread\_cond\_wait()前必须由本线程加锁（pthread\_mutex\_lock()），而在更新条件等待队列以前，mutex保持锁定状态，并在线程挂起进入等待前解锁。在条件满足从而离开pthread\_cond\_wait()之前，mutex将被重新加锁，以与进入pthread\_cond\_wait()前的加锁动作对应。     
        激发条件有两种形式，pthread\_cond\_signal()激活一个等待该条件的线程，存在多个等待线程时按入队顺序激活其中一个；而pthread\_cond\_broadcast()则激活所有等待线程。  

#### 二、为什么存在条件变量

　　首先，举个例子：在应用程序中有4个进程thread1，thread2，thread3和thread4，有一个int类型的全局变量iCount。iCount初始化为0，thread1和thread2的功能是对iCount的加1，thread3的功能是对iCount的值减1，而thread4的功能是当iCount的值大于等于100时，打印提示信息并重置iCount=0。

　　如果使用互斥量，线程代码大概应是下面的样子：

```cpp
thread1/2：

while (1)

{

pthread_mutex_lock(&mutex);

iCount++;

pthread_mutex_unlock(&mutex);

}

thread4:

while(1)

{

pthead_mutex_lock(&mutex);

if (100 <= iCount)

{

printf("iCount >= 100\r\n");

iCount = 0;

pthread_mutex_unlock(&mutex);

}

else

{

pthread_mutex_unlock(&mutex);

}

}
```

　     在上面代码中由于thread4并不知道什么时候iCount会大于等于100，所以就会一直在循环判断，但是每次判断都要加锁、解锁(即使本次并没有修改iCount)。这就带来了问题一：CPU浪费严重。所以在代码中添加了sleep()，这样让每次判断都休眠一定时间。但这又带来的问题二：如果sleep()的时间比较长，导致thread4处理不够及时，等iCount到了很大的值时才重置。对于上面的两个问题，可以使用条件变量来解决。

　　首先看一下使用条件变量后，线程代码大概的样子：

```cpp
thread1/2:

while(1)

{

pthread_mutex_lock(&mutex);

iCount++;

pthread_mutex_unlock(&mutex);

if (iCount >= 100)

{

pthread_cond_signal(&cond);

}

}

thread4:

while (1)

{

pthread_mutex_lock(&mutex);

while(iCount < 100)

{

pthread_cond_wait(&cond, &mutex);

}

printf("iCount >= 100\r\n");

iCount = 0;

pthread_mutex_unlock(&mutex);

}
```

　    从上面的代码可以看出thread4中，当iCount < 100时，会调用pthread\_cond\_wait。而pthread\_cond\_wait在上面应经讲到它会释放mutex，然后等待条件变为真返回。当返回时会再次锁住mutex。因为pthread\_cond\_wait会等待，从而不用一直的轮询，减少CPU的浪费。在thread1和thread2中的函数pthread\_cond\_signal会唤醒等待cond的线程（即thread4），这样当iCount一到大于等于100就会去唤醒thread4。从而不致出现iCount很大了，thread4才去处理。

　　需要注意的一点是在thread4中使用的**while** (iCount < 100),而不是**if** (iCount < 100)。这是因为在pthread\_cond\_singal()和pthread\_cond\_wait()返回之间有时间差，假如在时间差内，thread3又将iCount减到了100以下了，那么thread4在pthread\_cond\_wait()返回之后，显然应该再检查一遍iCount的大小，这就是while的用意，如果是if，则会直接往下执行，不会再次判断。

　　感觉可以总结为：条件变量用于某个线程需要在某种条件成立时才去保护它将要操作的临界区，这种情况从而避免了线程不断轮询检查该条件是否成立而降低效率的情况，这是实现了效率提高。在条件满足时，自动退出阻塞，再加锁进行操作。

　　IBM上有个关于条件变量的文章：[https://www.ibm.com/developerworks/cn/linux/thread/posix\_thread3/](https://www.ibm.com/developerworks/cn/linux/thread/posix_thread3/) ,也可以看看。

补充：

1、

问：条件变量为什么要与互斥锁一起使用呢？

答：这是为了应对线程4在调用pthread\_cond\_wait()但线程4还没有进入wait cond的状态的时候，此时线程2调用了 cond\_singal 的情况。 如果不用mutex锁的话，这个cond\_singal就丢失了。加了锁的情况是，线程2必须等到 mutex 被释放（也就是 pthread\_cod\_wait() 释放锁并进入wait\_cond状态 ，此时线程2上锁） 的时候才能调用cond\_singal

2、

调用pthread\_cond\_signal后要立刻释放互斥锁（也可以将pthread\_cond\_signal放在pthread\_mutex\_lock和pthread\_mutex\_unlock之后），因为pthread\_cond\_wait的最后一步是要将指定的互斥量重新锁住，如果pthread\_cond\_signal之后没有释放互斥锁，pthread\_cond\_wait仍然要阻塞。

参考：[http://blog.csdn.net/bolike/article/details/9025389](http://blog.csdn.net/bolike/article/details/9025389)

[https://blog.csdn.net/hairetz/article/details/4535920](https://blog.csdn.net/hairetz/article/details/4535920)