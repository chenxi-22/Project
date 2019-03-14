#include "OperaMySql.h"

char sql[MAX_LEN];

int InsertSql(Local* l, MYSQL* m_conn)
{
  mysql_init(m_conn);
  if(!mysql_real_connect(m_conn,"localhost","root","","Game",3306,NULL,0))
  {
    perror("mysql_real_connect");
    return -1;//链接失败
  }
  sprintf(sql,"insert into GameInfo values('%s','%s',%d);",l->id_buf,l->passwd_buf,1000);
  if(mysql_query(m_conn,sql))
  {
    perror("mysql_real_connect");
    return -1;//链接失败
  }
  //释放结果集
  mysql_close(m_conn);
  return 0;
}

int SelectSql(Local* l, MYSQL* m_conn)
{
  mysql_init(m_conn);
  if(!mysql_real_connect(m_conn,"localhost","root","","Game",3306,NULL,0))
  {
    perror("mysql_real_connect");
    return -1;//链接失败
  }
  sprintf(sql,"select * from GameInfo where ID='%s';",l->id_buf);
  if(mysql_query(m_conn,sql))
  {
    perror("mysql_query");
    return -1;//链接失���
  }
  
  //获取行数
  MYSQL_RES *result = mysql_store_result(m_conn);
  //获取列数                                                                                                                                    
  my_ulonglong rows = mysql_num_rows(result);
  MYSQL_ROW line;

  size_t i = 0;
  for(i = 0; i < rows ; ++i)
  {   
    line =  mysql_fetch_row(result);
    if(strcmp(line[0],l->id_buf)==0)
    {
      return 1;
    }
    printf("\n");
  }   

  mysql_close(m_conn);
  return 0;
}

int main()
{
  MYSQL m_conn;
  Local l;
  strcpy(l.id_buf, "liuchenxi");
  strcpy(l.passwd_buf, "hahahaha");
  InsertSql(&l, &m_conn);
  int ret = SelectSql(&l, &m_conn);
  printf("ret %d\n", ret);
}


