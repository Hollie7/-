#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX_LINE 1024
int main()
{
    /*缓冲区*/
    char buf[MAX_LINE];
    /*文件指针*/
    FILE*fp_count,*fp;
    /*行字符个数*/
    int len;
    int count=0;
    bool is_lastest=false;

    /*获取最新的日志的标志*/
    if((fp_count =fopen("/var/log/kern.log","r")) == NULL)
    {
        perror("fail to read");
        exit(1) ;
    }
    /*逐行读取日志*/
    while(fgets(buf,MAX_LINE,fp_count) != NULL)
    {
        len= strlen(buf);
        buf[len-1] = '\0';  /*去掉换行符*/
        int i;
        for(i=0;i<len-11;i++)
        {
            /*判断是否符合所作标记*/
            if(buf[i]=='b'&&buf[i+1]=='e'
            &&buf[i+2]=='g'&&buf[i+3]=='i'
            &&buf[i+4]=='n'&&buf[i+5]=='R'
            &&buf[i+6]=='E'&&buf[i+7]=='P'
            &&buf[i+8]=='O'&&buf[i+9]=='R'&&buf[i+10]=='T')
            {
                /*检测到一个符合条件的日志*/
                count++;
            }
        }
    }
    
    /*打开日志文件*/
    if((fp =fopen("/var/log/kern.log","r")) == NULL)
    {
        perror("fail to read");
        exit(1) ;
    }
    /*逐行读取日志*/
    while(fgets(buf,MAX_LINE,fp) != NULL)
    {
        len= strlen(buf);
        buf[len-1] = '\0';  /*去掉换行符*/
        int i;
        bool is_report=false;
        for(i=0;i<len-11;i++)
        {
            if(buf[i]=='b'&&buf[i+1]=='e'
            &&buf[i+2]=='g'&&buf[i+3]=='i'
            &&buf[i+4]=='n'&&buf[i+5]=='R'&&buf[i+6]=='E'
                &&buf[i+7]=='P'&&buf[i+8]=='O'
                &&buf[i+9]=='R'&&buf[i+10]=='T')
            {
                count--;
            }
        }
        /*检测最新的日志*/
        if(count==0)is_lastest=true;
        if(is_lastest)
        {
            for(i=0;i<len-8;i++)
            {
                /*判断是否符合所作标记MYREPORT*/
                if(buf[i]=='M'&&buf[i+1]=='Y'
                &&buf[i+2]=='R'&&buf[i+3]=='E'
                &&buf[i+4]=='P'&&buf[i+5]=='O'
                &&buf[i+6]=='R'&&buf[i+7]=='T')
                {
                    is_report=true;
                    break;
                }
            }
            /*输出日志*/
            if(is_report)printf("%s\n",buf+i);
        }
    }
    return 0;
}
