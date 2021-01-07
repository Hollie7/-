#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/init_task.h>
#include <linux/jiffies.h>
#include <linux/err.h>
#include <asm/uaccess.h>
/*模块名字*/
#define MODULE_NAME "thread_print"
/*进程指针*/
struct task_struct *report_task;
/*report的间隔*/
long report_interval = 5;
/*report函数，遍历操作系统所有进程*/
static void report(void)
{
    /*所有状态的进程的总数*/
    int pro_total=0;
    /*TASK_RUNNING状态的进程的总数*/
    int pro_running=0;
    /*TASK_INTERRUPTIBLE状态的进程的总数*/
    int pro_interruptible=0;
    /*TASK_UNINTERRUPTIBLE状态的进程的总数*/
    int pro_uninterruptible=0;
    /*TASK_STOPPED状态的进程的总数*/
    int pro_stopped=0;
    /*TASK_TRACED状态的进程的总数*/
    int pro_traced=0;
    /*TASKZOMBIE状态的进程的总数*/
    int pro_zombie=0;
    /*TASK_DEAD状态的进程的总数*/
    int pro_dead=0;
    /*UNKNOWN状态的进程的总数*/
    int pro_unknown=0;

    /*用于遍历的进程指针*/
    struct task_struct *p = &init_task;

    /*进程的state,exit_state*/
    long old_state,old_exit_state;
    /*进程的pid*/
    pid_t pid;
    /*进程的名字*/
    char name[20];
    /*进程的父进程*/
    struct task_struct *parent;
    /*进程的父进程的名字*/
    char parent_name[20];

    /*访问 RCU 链表项，为短小读侧临界部分抑制内核抢占，该函数与rcu_read_lock配对使用,用以标记读者退出读端临界区*/
    rcu_read_lock();
    /*-----------------------------------------------读端临界区--------------------------------------------------*/
    /*进程的父进程的名字*/
    printk("beginREPORT\n");

    for(p=&init_task;(p=next_task(p))!=&init_task;)
    {
        /*总进程数目+1*/
        pro_total++;
        /*获得进程的state*/
        old_state=p->state;
        /*获得进程的exit_state*/
        old_exit_state=p->exit_state;
        /*获得进程的pid*/
        pid=p->pid;
        /*获得进程的名字*/
        sprintf(name,"%s",p->comm);
        /*获得进程的父进程的名字*/
        parent=p->real_parent;
        sprintf(parent_name,"%s",parent->comm);
        /*统计各种状态的进程数目*/
        /*exit_state只包含EXIT_ZOMBIE和EXIT_DEAD状态*/
        switch(old_exit_state)
        {
            case EXIT_ZOMBIE :
                /*使用printk函数打印有关变量的值*/
                printk("MYREPORT:task name : %s, pid: %ld ,parent name: %s,state: TASK_ZOMBIE.\n",
                        name,pid,parent_name);
                pro_zombie++;
                break;
            case EXIT_DEAD :
                /*使用printk函数打印有关变量的值*/
                printk("MYREPORT:task name : %s, pid: %ld ,parent name: %s,state: TASK_DEAD.\n",
                        name,pid,parent_name);
                pro_dead++;
                break;
            default:
                break;
        }
        /*如果exit_state！=0，后面的统计没有必要进行*/
        if(old_exit_state)continue;
        switch(old_state)
        {
            case TASK_RUNNING :
                /*使用printk函数打印有关变量的值*/
                printk("MYREPORT:task name : %s, pid: %ld ,parent name: %s,state: TASK_RUNNING.\n",
                        name,pid,parent_name);
                pro_running++;
                break;
            case TASK_INTERRUPTIBLE :
                /*使用printk函数打印有关变量的值*/
                printk("MYREPORT:task name : %s, pid: %ld ,parent name: %s,state: TASK_INTERRUPTIBLE.\n",
                        name,pid,parent_name);
                pro_interruptible++;
                break;
            case TASK_UNINTERRUPTIBLE :
                /*使用printk函数打印有关变量的值*/
                printk("MYREPORT:task name : %s, pid: %ld ,parent name: %s,state: TASK_UNINTERRUPTIBLE.\n",
                        name,pid,parent_name);
                pro_uninterruptible++;
                break;
            case TASK_STOPPED :
                /*使用printk函数打印有关变量的值*/
                printk("MYREPORT:task name : %s, pid: %ld ,parent name: %s,state: TASK_STOPPED.\n",
                        name,pid,parent_name);
                pro_stopped++;
                break;
            case TASK_TRACED :
                /*使用printk函数打印有关变量的值*/
                printk("MYREPORT:task name : %s, pid: %ld,parent name: %s,state: TASK_TRACED.\n",
                        name,pid,parent_name);
                pro_traced++;
                break;  
            default:
                /*使用printk函数打印有关变量的值*/
                pro_unknown++;
                printk("MYREPORT:task name : %s, pid: %ld ,parent name: %s,state: TASK_UNKNOWN.\n",
                        name,pid,parent_name);
                break;
        }
    }
    /*-----------------------------------------------读端临界区--------------------------------------------------*/
    rcu_read_unlock();

    /*输出统计系统中进程个数*/
    printk("MYREPORT:total tasks: %4d\n",pro_total);
    printk("MYREPORT:TASK_RUNNING: %4d\n",pro_running);
    printk("MYREPORT:TASK_INTERRUPTIBLE: %4d\n",pro_interruptible);
    printk("MYREPORT:TASK_UNINTERRUPTIBLE: %4d\n",pro_uninterruptible);
    printk("MYREPORT:TASK_STOPPED: %4d\n",pro_stopped);
    printk("MYREPORT:TASK_TRACED: %4d\n",pro_traced);
    printk("MYREPORT:TASK_ZOMBIE: %4d\n",pro_zombie);
    printk("MYREPORT:TASK_DEAD: %4d\n",pro_dead);
    printk("MYREPORT:unknown state: %4d\n",pro_unknown);
    return;
}

static int reportd(void *p)
{
    long i;
    /*设置普通进程的优先级*/
    set_user_nice(current,19);
    /*设置进程的状态,设置的状态为 TASK_INTERRUPTIBLE*/
    set_current_state(TASK_INTERRUPTIBLE);
    /*设置普通进程的优先级*/
    while(!kthread_should_stop())
    {
        /*设置进程的状态,设置的状态为 TASK_RUNNING*/
        __set_current_state(TASK_RUNNING);
        /*设置report_interval*/
        i=report_interval;
        if(i==0)
        {
            i=1;
        }else{
            /*打印*/
            report();
        }
        /*设置进程的状态,设置的状态为 TASK_INTERRUPTIBLE*/
        set_current_state(TASK_INTERRUPTIBLE);
        /*设置普通进程的优先级*/
        schedule_timeout(i*HZ);
    }
    /*设置普通进程的优先级*/
    __set_current_state(TASK_RUNNING);
    return 0;
}

static int __init myReport_init(void)
{
    /*开始 kernel thread*/
    report_task=kthread_run(reportd,NULL,"reportd");
    /*判断kreportd create是否成功*/
    if(IS_ERR(report_task))
    {
        printk(KERN_ERR"kreportd create failed\n");
        return PTR_ERR(report_task);
    }else
    {
        printk("kreportd create successful\n");
    }
    /*打印初始化输出*/
    printk(KERN_INFO"%s initialised\n",
            MODULE_NAME);
    return 0;
}

static void __exit myReport_cleanup(void)
{
    /*打印注销输出*/
    printk(KERN_INFO"%s removed\n",
            MODULE_NAME);
}
/*注册模块*/
module_init(myReport_init);
/*注销模块*/
module_exit(myReport_cleanup);
/*注销模块*/
MODULE_LICENSE("GPL");
