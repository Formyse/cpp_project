/*
 *  execsql.cpp，本程序用于把全国站点参数数据保存到数据库T_ZHOBTCODE表中。
 *  作者：李响。
*/
#include "_public.h"
#include "_mysql.h"

CLogFile logfile;

connection conn;

CPActive PActive;

void EXIT(int sig);

int main(int argc,char *argv[])
{
  // 帮助文档。
  if (argc!=5)
  {
    printf("\n");
    printf("Using:./execsql sqlfile charset logfile\n");

    printf("Example:/project/tools1/bin/procctl 120 /project/tools1/bin/execsql /project/idc2/sql/cleardata.sql \"127.0.0.1,root,Lx2022,mysql,3306\" utf8 /log/idc/execsql.log\n\n");

    printf("这是一个工具程序，用于执行一个sql脚本文件。\n");
    printf("sqlfile sql脚本文件名，每条sql语句可以多行书写，分号表示一条sql语句的结束，不支持注释。\n");
    printf("connstr 数据库连接参数：ip,username,password,dbname,port\n");
    printf("charset 数据库的字符集。\n");
    printf("logfile 本程序运行的日志文件名。\n");

    return -1;
  }
  // 处理程序退出的信号。
  // 关闭全部的信号的输入输出。
  // 设置信号，在shell状态下可用 kill + 进程号  正常终止这些进程。
  // 但请不要用kill -9 + 进程号 强行终止。
  CloseIOAndSignal(true);signal(SIGINT,EXIT); signal(SIGTERM,EXIT);  

  // 打开日志文件。
  if (logfile.Open(argv[4],"a+")==false)
  {
    printf("打开日志文件失败（%s）。\n",argv[4]); return -1;
  }

  PActive.AddPInfo(500,"execsql");  // 进程的心跳，10妙足够。
  // 注意，在调试程序的时候，可以启用类似以下的代码，防止超时。
  // PActive.AddPInfo(5000,"execsql");

  // 连接数据库，不启用事务。
  if (conn.connecttodb(argv[2],argv[3],1)!=0)
  {
    logfile.Write("connect database(%s) failed,\n%s\n",argv[2],conn.m_cda.message); return -1;
  }

  logfile.Write("connect database(%s) ok.\n",argv[2]);
  
  CFile File;

  // 打开SQL文件。
  if (File.Open(argv[1],"r")==false)
  {
    logfile.Write("File.Open(%s) failed.\n",argv[1]); EXIT(-1);
  }

  char strsql[1001];   // 存放从SQL文件中读取的SQL语句。

  while (true)
  {
    memset(strsql,0,sizeof(strsql));

    // 从SQL文件中读取以分号结束的一行。
    if (File.FFGETS(strsql,1000,";")==false) break;

    // 删除掉SQL语句最后的分号。
    char *pp=strstr(strsql,";");
    if (pp==0) continue;
    pp[0]=0;

    logfile.Write("%s\n",strsql);

    int iret=conn.execute(strsql); // 执行SQL语句。

    // 把SQL语句执行结果写日志。
    if (iret==0) logfile.Write("exec ok(rpc=%d).\n",conn.m_cda.rpc);
    else logfile.Write("exec failed(%s).\n",conn.m_cda.message);

    PActive.UptATime();   // 进程的心跳。
  }

  logfile.Write("\n");

  return 0;
}


void EXIT(int sig)
{
  logfile.Write("程序退出，sig=%d\n\n",sig);

  conn.disconnect();

  exit(0);
}
