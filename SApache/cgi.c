#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <mysql/mysql.h>

void Show_Event(char *);
void Show_Tables(MYSQL *,MYSQL_RES*,char *);
void Delete_Row(char *,int);
void Reset_Table_Student(char *);
void Reset_Table_Score(char *);
void Reset_Table_Course(char *);
void Add_Row(char *,char *);
void Drop_And_Create(char *);
void Match_Password(char *,char *);

#define OBJ_LEN 50
#define ID_LEN 12
#define NAME_SIZE 20
#define STUID_LEN 11
#define STUNAME_LEN 11
#define SEX_LEN 11
#define AGE_LEN 12
#define PHONENUM_LEN 12
#define SUBID_LEN 11
#define SUBNAME_LEN 21
#define SQL_LEN 128
#define POST_LEN 2048
#define ACCOUNT_LEN 16
#define PASSWORD_LEN 32


void Show_Tables(MYSQL *mycon,MYSQL_RES* resptr,char *object)//动态打印数据库中的表格
{
	MYSQL_ROW sqlrow ;
	MYSQL_FIELD *field_ptr ;
	char delete_buff[NAME_SIZE] = {0};
	char add_buff[NAME_SIZE] = {0};
	char sub_buff[NAME_SIZE]= {0};
	char maxlength[NAME_SIZE] = {0};
	unsigned int field_count = 0 ;
	printf("<html>\
			<head>\
			<title>%s Table</title>\
			</head>\
			\
			<body>\
			<h1 align=\"center\" style=\"font-family:times;\">%s Table</h1>\
			<h4 align=\"left\" style=\"font-family:times;\">Please click \"submit\" after editing</h1>\
			<br>\
			<center>\
			<form action=\"show\" method=\"post\">\
			<table border=3 bordercolor=\"#006803\" cellpadding=\"8\" rules=\"all\" style=\"font-family:times;\" frame=\"hsides\">\
			<tr align=\"center\">",object,object);
	while((field_ptr=mysql_fetch_field(resptr))!=NULL)
		printf("<th>%s</th>",field_ptr->name);
	printf("<th>delete</th></tr>");
	mysql_field_seek(resptr,0);
	while( ( sqlrow=mysql_fetch_row(resptr) ) )
	{
		printf("<tr align=\"center\">");
		char id_buff[ID_LEN] = {0};
		char name_buff[NAME_SIZE] = {0};
		while(field_count < mysql_field_count(mycon) &&
				(field_ptr = mysql_fetch_field(resptr))!=NULL)
		{
			if(strncmp(field_ptr->name,"ID",2)==0)
				strcpy(id_buff,sqlrow[field_count]);
			strcpy(name_buff,field_ptr->name);
			strcat(name_buff,id_buff);
			sprintf(maxlength,"%ld",field_ptr->length);
			printf("<td><input type=\"input\" value=%s name=%s maxlength=%s></td>",sqlrow[field_count],name_buff,maxlength);
			field_count++;
			memset(name_buff,0,NAME_SIZE);
			memset(maxlength,0,NAME_SIZE);
		}
		strcpy(delete_buff,object);
		strcat(delete_buff,"Del");
		strcat(delete_buff,id_buff);
		printf("<td><input type=\"checkbox\" value=\"delete\" name=%s></td>",delete_buff);
		printf("</tr>");
		mysql_field_seek(resptr,0);
		field_count=0;
	}
	printf("</table>\
			<br>");
	strcpy(add_buff,object);
	strcat(add_buff,"Add");
	strcpy(sub_buff,object);
	strcat(sub_buff,"Submit");
	printf("<input type=\"submit\" value=\"insert\" name=%s>\
			<input type=\"submit\" value=\"submit\" name=%s>\
			</form>\
			</center>\
			\
			</body>\
			</html>\
			",add_buff,sub_buff);
}

void Show_Event(char *object)
{
	MYSQL my_con;
	int res;
	mysql_init(&my_con);
	if(mysql_real_connect(&my_con,"localhost","root","123456","StudentManagement",0,NULL,0))
	{
		char Cur_SQL[SQL_LEN] = {0};
		sprintf(Cur_SQL,"select *from %s",object);
		res = mysql_query(&my_con,Cur_SQL);
		if(res)
			printf("select error :%s\n",mysql_error(&my_con));
		else
		{
			MYSQL_RES *res_ptr = mysql_store_result(&my_con);
			if(res_ptr)
				Show_Tables(&my_con,res_ptr,object);
			mysql_free_result(res_ptr);
		}
		mysql_close(&my_con);
	}
	else
	{
		fprintf(stderr,"Connection Failed\n");
		if(mysql_errno(&my_con))
		{
			fprintf(stderr,"Connection Error %d:%s\n",mysql_errno(&my_con),
					mysql_error(&my_con));
		}
	}
}

void Drop_And_Create(char *object)//每次submit删表重建
{
	MYSQL my_con;
	int res;
	mysql_init(&my_con);
	if(mysql_real_connect(&my_con,"localhost","root","123456","StudentManagement",0,NULL,0))
	{
		char Cur_SQL[SQL_LEN] = {0};
		sprintf(Cur_SQL,"drop table %s",object);
		res = mysql_query(&my_con,Cur_SQL);
		if(res)
			printf("drop error :%s\n",mysql_error(&my_con));
		memset(Cur_SQL,0,SQL_LEN);
		if(strcmp(object,"Student")==0)
			res = mysql_query(&my_con,"create table Student(\
				  ID INT primary key auto_increment,\
					StuID varchar(10),StuName varchar(10),\
					Sex varchar(10),\
					Age int,\
					PhoneNum varchar(11)\
					)");
		else if(strcmp(object,"Score")==0)
			res = mysql_query(&my_con,"create table Score(\
				  ID INT primary key auto_increment,StuID varchar(10),\
					StuName varchar(10),\
					SubID varchar(10),\
					SubName varchar(20),\
					Degree float\
					)");
		else if(strcmp(object,"Course")==0)
			res = mysql_query(&my_con,"create table Course(\
				  ID INT primary key auto_increment,\
					SubID varchar(10),\
					SubName varchar(10)\
					);");
		if(res)
			printf("create error :%s\n",mysql_error(&my_con));
		mysql_close(&my_con);
	}
	else
	{
		fprintf(stderr,"Connection Failed\n");
		if(mysql_errno(&my_con))
		{
			fprintf(stderr,"Connection Error %d:%s\n",mysql_errno(&my_con),
					mysql_error(&my_con));
		}
	}
}

void *Move_Equal(char **des,char *obj)//提取每字段数据
{
	while(**des != '=')
		++(*des);
	++(*des);
	if(*des==NULL || strncmp(*des,"%28",3)==0) 
		strcpy(obj,"NULL") ;
	else
		strcpy(obj,*des);
}

void Reset_Table_Student(char *html)//每submit一次，表格的数据重置
{
	if(html==NULL)
		return ;
	char *del = NULL ;
	char stuid[STUID_LEN] = {0};
	char stuname[STUNAME_LEN] = {0};
	char sex[SEX_LEN] = {0};
	char age[AGE_LEN] = {0};
	char phonenum[PHONENUM_LEN] = {0};
	char sql[SQL_LEN] = {0};
	char pml_arr[POST_LEN] = {0} ;
	strcpy(pml_arr,html);
	char *pml = pml_arr;
	char *ptr = NULL ;
	char *tmp = NULL ;
	Drop_And_Create("Student");
	int sign = 1 ;
	ptr = strtok_r(pml,"&",&tmp);
	if(strncmp(ptr,"StudentSubmit",13)==0)
		return ;
	while(sign)
	{
		int co = 0 ;
		while(co != 5 && sign==1)
		{
			ptr = strtok_r(NULL,"&",&tmp);
			if((strncmp(ptr,"StudentSubmit",13)==0)|| ptr == NULL
			  || (strncmp(ptr,"StudentAdd",10)==0))
			{
				sign = 0 ;
				break ;
			}
			if(strncmp(ptr,"ID",2)==0)
				continue ;
			else if(strncmp(ptr,"StuID",5)==0)
			{
				co++;
				Move_Equal(&ptr,stuid);
				continue ;
			}
			else if(strncmp(ptr,"StuName",7)==0)
			{
				co++;
				Move_Equal(&ptr,stuname);
				continue ;
			}
			else if(strncmp(ptr,"Sex",3)==0)
			{
				co++;
				Move_Equal(&ptr,sex);
				continue ;
			}
			else if(strncmp(ptr,"Age",3)==0)
			{
				co++;
				Move_Equal(&ptr,age);
				continue ;
			}
			else if(strncmp(ptr,"PhoneNum",8)==0)
			{
				co++;
				Move_Equal(&ptr,phonenum);
				continue ;
			}
		}
		if(sign){
			int age_num = atoi(age);
			sprintf(sql,"insert into Student(StuID,StuName,Sex,Age,PhoneNum) values('%s','%s','%s',%d,'%s')",stuid,stuname,sex,age_num,phonenum);
			Add_Row("Student",sql);
			memset(stuid,0,STUID_LEN);
			memset(stuname,0,STUNAME_LEN);
			memset(sex,0,SEX_LEN);
			memset(age,0,SEX_LEN);
			memset(phonenum,0,PHONENUM_LEN);
			memset(sql,0,SQL_LEN);
		}
	}
	while( ( del = strstr(html,"StudentDel") )!= NULL)
	{
		del += 10 ;
		int num = 0 ;
		while( *del != '=' )
		{
			num = num*10 + *del - '0';
			++del;
		}
		Delete_Row("Student",num);
		html = del+7;
	}
}

void Reset_Table_Score(char *html)
{
	if(html==NULL)
		return ;
	char *del = NULL ;
	char stuid[STUID_LEN] = {0};
	char stuname[STUNAME_LEN] = {0};
	char subid[SUBID_LEN] = {0};
	char subname[SUBNAME_LEN] = {0};
	char degree[AGE_LEN] = {0};
	char sql[SQL_LEN] = {0};
	char pml_arr[POST_LEN] = {0} ;
	strcpy(pml_arr,html);
	char *pml = pml_arr;
	char *ptr = NULL ;
	char *tmp = NULL ;
	Drop_And_Create("Score");
	int sign = 1 ;
	ptr = strtok_r(pml,"&",&tmp);
	if(strncmp(ptr,"ScoreSubmit",11)==0)
		return ;
	while(sign)
	{
		int co = 0 ;
		while(co != 5 && sign==1)
		{
			ptr = strtok_r(NULL,"&",&tmp);
			if((strncmp(ptr,"ScoreSubmit",11)==0) || ptr == NULL
					|| (strncmp(ptr,"ScoreAdd",8)==0) )
			{
				sign = 0 ;
				break ;
			}
			if(strncmp(ptr,"ID",2)==0)
				continue ;
			else if(strncmp(ptr,"StuID",5)==0)
			{
				co++;
				Move_Equal(&ptr,stuid);
				continue ;
			}
			else if(strncmp(ptr,"StuName",7)==0)
			{
				co++;
				Move_Equal(&ptr,stuname);
				continue ;
			}
			else if(strncmp(ptr,"SubID",5)==0)
			{
				co++;
				Move_Equal(&ptr,subid);
				continue ;
			}
			else if(strncmp(ptr,"SubName",7)==0)
			{
				co++;
				Move_Equal(&ptr,subname);
				continue ;
			}
			else if(strncmp(ptr,"Degree",6)==0)
			{
				co++;
				Move_Equal(&ptr,degree);
				continue ;
			}
		}
		if(sign){
			float degree_f = atof(degree) ;
			sprintf(sql,"insert into Score(StuID,StuName,SubID,SubName,Degree) values('%s','%s','%s','%s',%f)",stuid,stuname,subid,subname,degree_f);
			Add_Row("Score",sql);
			memset(stuid,0,STUID_LEN);
			memset(stuname,0,STUNAME_LEN);
			memset(subid,0,SUBID_LEN);
			memset(subname,0,SUBNAME_LEN);
			memset(degree,0,AGE_LEN);
			memset(sql,0,SQL_LEN);
		}
	}
	while( ( del = strstr(html,"ScoreDel") )!= NULL)
	{
		del += 8 ;
		int num = 0 ;
		while( *del != '=' )
		{
			num = num*10 + *del - '0';
			++del;
		}
		Delete_Row("Score",num);
		html = del+7;
	}
}

void Reset_Table_Course(char *html)
{
	if(html==NULL)
		return ;
	char *del = NULL ;
	char subid[SUBID_LEN] = {0};
	char subname[SUBNAME_LEN] = {0};
	char sql[SQL_LEN] = {0};
	char pml_arr[POST_LEN] = {0} ;
	strcpy(pml_arr,html);
	char *pml = pml_arr;
	char *ptr = NULL ;
	char *tmp = NULL ;
	Drop_And_Create("Course");
	int sign = 1 ;
	ptr = strtok_r(pml,"&",&tmp);
	if(strncmp(ptr,"CourseSubmit",12)==0)
		return ;
	while(sign)
	{
		int co = 0 ;
		while(co != 2 && sign==1)
		{
			ptr = strtok_r(NULL,"&",&tmp);
			if(strncmp(ptr,"CourseSubmit",12)==0 || ptr==NULL 
					|| strncmp(ptr,"CourseAdd",9)==0)
			{
				sign = 0 ;
				break ;
			}
			if(strncmp(ptr,"ID",2)==0)
				continue ;
			else if(strncmp(ptr,"SubID",5)==0)
			{
				co++;
				Move_Equal(&ptr,subid);
				continue ;
			}
			else if(strncmp(ptr,"SubName",7)==0)
			{
				co++;
				Move_Equal(&ptr,subname);
				continue ;
			}
		}
		if(sign){
			sprintf(sql,"insert into Course(SubID,SubName) values('%s','%s')",subid,subname);
			Add_Row("Course",sql);
			memset(subid,0,SUBID_LEN);
			memset(subname,0,SUBNAME_LEN);
			memset(sql,0,SQL_LEN);
		}
	}
	while( ( del = strstr(html,"CourseDel") )!= NULL)
	{
		del += 9 ;
		int num = 0 ;
		while( *del != '=' )
		{
			num = num*10 + *del - '0';
			++del;
		}
		Delete_Row("Course",num);
		html = del+7;
	}
}

void Add_Row(char *table,char *sql)
{
	MYSQL my_con;
	int res;
	mysql_init(&my_con);
	if(mysql_real_connect(&my_con,"localhost","root","123456","StudentManagement",0,NULL,0))
	{
		res = mysql_query(&my_con,sql);
		if(res)
			printf("insert error :%s\n",mysql_error(&my_con));
		mysql_close(&my_con);
	}
	else
	{
		fprintf(stderr,"Connection Failed\n");
		if(mysql_errno(&my_con))
		{
			fprintf(stderr,"Connection Error %d:%s\n",mysql_errno(&my_con),
					mysql_error(&my_con));
		}
	}
}

void Delete_Row(char *table,int id)
{
	MYSQL my_con;
	int res;
	mysql_init(&my_con);
	if(mysql_real_connect(&my_con,"localhost","root","123456","StudentManagement",0,NULL,0))
	{
		char Cur_SQL[SQL_LEN] = {0};
		sprintf(Cur_SQL,"delete from %s where ID = %d",table,id);
		res = mysql_query(&my_con,Cur_SQL);
		if(res)
			printf("delete error :%s\n",mysql_error(&my_con));
		else
			mysql_close(&my_con);
	}
	else
	{
		fprintf(stderr,"Connection Failed\n");
		if(mysql_errno(&my_con))
		{
			fprintf(stderr,"Connection Error %d:%s\n",mysql_errno(&my_con),
					mysql_error(&my_con));
		}
	}
}

void Get_Ptr(char **p)//如果有数据，识别制作p跳到最后一个&后面
{
	if( p == NULL || *p == NULL)
		return ;
	if(strncmp(*p,"Student",7) == 0 || 
			strncmp(*p,"Score",5) == 0 ||
			strncmp(*p,"Course",6) == 0)
		return ;
	else
		*p = strrchr(*p,'&') + 1;
}

void Match_Password(char *account,char *password)
{
	MYSQL_ROW sqlrow ;
	MYSQL_FIELD *field_ptr ;
	MYSQL my_con;
	int res;
	mysql_init(&my_con);
	unsigned int field_count = 0 ;
	char account_buff[ID_LEN] = {0};
	char password_buff[NAME_SIZE] = {0};
	if(mysql_real_connect(&my_con,"localhost","root","123456","StudentManagement",0,NULL,0))
	{
		res = mysql_query(&my_con,"select *from Password");
		if(res)
			printf("select error :%s\n",mysql_error(&my_con));
		else
		{
			MYSQL_RES *res_ptr = mysql_store_result(&my_con);
			if(res_ptr)
			{
				while( ( sqlrow=mysql_fetch_row(res_ptr) ) )
				{
					while(field_count < mysql_field_count(&my_con) &&
							(field_ptr = mysql_fetch_field(res_ptr))!=NULL)
					{
						if(strncmp(field_ptr->name,"Account",7)==0)
							strcpy(account_buff,sqlrow[field_count]);
						else if(strncmp(field_ptr->name,"Password",8)==0)
							strcpy(password_buff,sqlrow[field_count]);
						field_count++;
					}
				}

			}
			mysql_free_result(res_ptr);
		}
		mysql_close(&my_con);
	}
	else
	{
		fprintf(stderr,"Connection Failed\n");
		if(mysql_errno(&my_con))
		{
			fprintf(stderr,"Connection Error %d:%s\n",mysql_errno(&my_con),
					mysql_error(&my_con));
		}
	}
	if( (strcmp(account,account_buff)==0) && 
			(strcmp(password,password_buff)==0) )
	{
		printf("<html>\
				<head>\
				<title>Student Management</title>\
				</head>\
				\
				<body>\
				<br>\
				<br>\
				<br>\
				<br>\
				<h1 align=\"center\" style=\"font-family:courier;\">Student Management</h1>\
				<br>\
				<form action=\"show\" method=\"post\">\
				<div align=\"center\">\
				<input type=\"submit\" value=\"Student Table\" name=\"Student\" style=\"width:15%;height:6%;\">\
				\
				</div>\
				<br>\
				<div align=\"center\">\
				<input type=\"submit\" value=\"Course Table\" name=\"Course\" style=\"width:15%;height:6%;\">\
				</div>\
				<br>\
				<div align=\"center\">\
				<input type=\"submit\" value=\"Score Table\" name=\"Score\" style=\"width:15%;height:6%;\">\
				</div>\
				</form>\
				\
				 </body>\
				  </html>\
				  ");
	}
	else
		printf("Account or Password is wrong\n");
}

void Get_AcPa(char *argv,char *account,char *password)
{
	char *tmp = NULL;
	char *a = strtok_r(argv,"&",&tmp);
	if(a != NULL)
		strcpy(account,a+8);
	char *p = strtok_r(NULL,"&",&tmp);
	if(p != NULL)
		strcpy(password,p+9);
}

int main(int argc,char *argv[])
{
	if(argc != 2)
	{
		printf("error:argc\n");
		exit(0);
	}
	if(strncmp(argv[1],"Account",7)==0)
	{
		char account[ACCOUNT_LEN] = {0};
		char password[PASSWORD_LEN] = {0};
		Get_AcPa(argv[1],account,password);
		if( (strlen(account)==0) || (strlen(password)==0) )
			printf("Account or Password cannot be NULL\n");
		Match_Password(account,password);
	}
	else if(strncmp(argv[1],"Student=",8)==0)
		Show_Event("Student");
	else if(strncmp(argv[1],"Course=",7)==0)
		Show_Event("Course");
	else if(strncmp(argv[1],"Score=",6)==0)
		Show_Event("Score");
	else
	{
		char *q = argv[1];
		Get_Ptr(&q);
		char p[POST_LEN] = {0};
		strcpy(p,q);
		if(p != NULL)
		{
			if(strncmp(p,"Student",7)==0)
			{
				Reset_Table_Student(argv[1]);
				if(strncmp(p,"StudentAdd",10)==0)
					Add_Row("Student","insert into Student(StuID) values(NULL)");
				Show_Event("Student");
			}
			else if(strncmp(p,"Course",6)==0)
			{
				Reset_Table_Course(argv[1]);
				if(strncmp(p,"CourseAdd",9)==0)
					Add_Row("Course","insert into Course(SubID) values(NULL)");
				Show_Event("Course");
			}
			else if(strncmp(p,"Score",5)==0)
			{
				Reset_Table_Score(argv[1]);
				if(strncmp(p,"ScoreAdd",8)==0)
					Add_Row("Score","insert into Score(StuID) values(NULL)");
				Show_Event("Score");
			}
		}
	}
	exit(0);
}
