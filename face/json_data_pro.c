/*
 * json_data_pro.c
 *
 *  Created on: 2015-3-4 下午1:43:38
 *  
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "json_data_pro.h"

int setup_json_face_threhold=70;
char setup_json_server_ip[20];
int setup_json_face_size=40;
int setup_json_server_port=10888,setup_json_stop_period=2,setup_json_cam_algo=1,setup_json_algo_sw_wait=6;//by hgj



void str_to_hex(char*data,int data_len,char*hex_out,int *hex_len)
{
	char *buf=alloca(data_len+1);
	char tmp;
	int i=0,j=0;;
	for(i=0;i<data_len;i++)
	{
		if((*(data+i)>='0')&&(*(data+i)<='9'))
		{
			tmp=*(data+i)-'0';
		}
		else if((*(data+i)>='a')&&(*(data+i)<='f'))
		{
			tmp=*(data+i)-'a'+10;
		}
		else if((*(data+i)>='A')&&(*(data+i)<='F'))
		{
			tmp=*(data+i)-'A'+10;
		}
		buf[i]=tmp;
		//printf("data=%02x\n",buf[i]);
	}
	for(j=0,i=0;j<data_len;i++)
	{
		hex_out[i]=(buf[j]<<4)+buf[j+1];
		j+=2;
		//printf("##hex_out=%02x\n",hex_out[i]);
	}
	if(hex_len!=NULL)
	{
		*hex_len=i;
		//printf("##hex_len=%d\n",*hex_len);
	}
}

void hex_to_str(char*hex_in,int hex_len,char*out,int *len)
{
	char *buf=alloca(hex_len+1);
	char tmp;
	int i=0;
	memset(out,'\0',hex_len);
	for(i=0;i<hex_len;i++)
	{
		//printf("%02x ",hex_in[i]);
		sprintf(out,"%s%02x",out,hex_in[i]);
	}
	//printf("##out=%s\n",out);
	if(len!=NULL)
	{
		*len=i;
		//printf("##hex_len=%d\n",*len);
	}
}



char* build_json_ret_data(char * terminal_code,char*equipment_code,char *state)
{

	json_object *my_object;
	my_object = json_object_new_object();
	json_object_object_add(my_object, "user_name", json_object_new_string("a210f1f2f390"));
	json_object_object_add(my_object, "terminal_code", json_object_new_string("a210f1f2f390"));
	json_object_object_add(my_object, "type", json_object_new_string("1"));
	json_object_object_add(my_object, "equipmentCode", json_object_new_string(equipment_code));
	json_object_object_add(my_object, "state", json_object_new_string(state));
	char *ptr=json_object_to_json_string(my_object);
	//printf("build_json_data:%s\n", json_object_to_json_string(my_object));
	char *string=malloc(strlen(ptr)+1);
	if(string!=NULL)
	{
		memset(string,'\0',strlen(ptr+1));
		memcpy(string,ptr,strlen(ptr));
		//printf("strlen(ptr):%d\n",strlen(string));
	}
	json_object_put(my_object);
	return string;
}

void free_json_ret_data(void*ptr)
{
	free(ptr);
}


static int count=0;
int cloud_json_data_process(void *data,int len)
{

	uint32_t cmd=0;
	if(len>=10)
	{
		memcpy(&cmd,data+2,4);
		printf("###cmd=%04d\n",cmd);
	}
	json_object *my_string, *my_object,*arrary_object,*arrary_item,*arrary_item_data;
	int arrary_length=0,i=0;
	char *string=NULL;
	char code_id[12];
	int code_id_len;
	char send_buf[256] ={0x98 ,0x5a ,0x11 ,0x01   ,0x8c ,0x01 ,0x00 ,0x00 ,0x01 ,0xff ,   0xf2 ,0x00 ,0x00 ,0x00 ,0x01 ,0x1e   ,0x03 ,0x00 ,0x15 ,0x9b ,0xfe };
	char send_buf2[256]={0x98 ,0x5a ,0x11 ,0x02   ,0x8c ,0x01 ,0x00 ,0x00 ,0x01 ,0xff ,   0xf2 ,0x00 ,0x00 ,0x00 ,0x01 ,0x1e   ,0x03 ,0x00 ,0x15 ,0x9b ,0xfe };



	//printf("$$$$$$=%s\n",data+10);

	my_string = json_tokener_parse(data+10);
	printf("my_string=%s\n", json_object_get_string(my_string));

	switch(cmd)
	{
	case 2584:
	{
		my_object=json_object_object_get(my_string,"resultType");
		string=json_object_get_string(my_object);
		if(string!=NULL)
		{
			printf("##resultType=%s\n", string);
		}
		else
		{
			json_object_put(my_object);
			break;
		}
		json_object_put(my_object);

		my_object=json_object_object_get(my_string,"equipmentCode");
		string=json_object_get_string(my_object);
		if(string!=NULL)
		{
			printf("##equipmentCode=%s\n", string);
			str_to_hex(string,strlen(string),code_id,code_id_len);
			memcpy(send_buf+4,code_id,6);
		}
		json_object_put(my_object);

		my_object=json_object_object_get(my_string,"terminal_code");
		string=json_object_get_string(my_object);
		if(string!=NULL)
		{
			printf("##terminal_code=%s\n", string);
			str_to_hex(string,strlen(string),code_id,code_id_len);
			memcpy(send_buf+10,code_id,6);
		}
		json_object_put(my_object);



		my_object=json_object_object_get(my_string,"resultCode");
		string= json_object_get_string(my_object);
		if(string!=NULL){
			printf("##resultCode=%s\n",string);
		}
		json_object_put(my_object);

		my_object=json_object_object_get(my_string,"state");
		char *stat=json_object_get_string(my_object);
		if(stat!=NULL)
		{
			printf("##  state=%s\n", stat);
		}
		else
		{
			json_object_put(my_object);
			break;
		}

		count++;
		send_buf[18]=count;
		if(*stat==0x31)
		{
			printf("###open\n");
			send_buf[3]=1;
		//	uart_send_data(UART1_DATA, send_buf,21);
		}
		else
		{
			send_buf[3]=2;
			//uart_send_data(UART1_DATA, send_buf,21);
		}
		json_object_put(my_object);
	}
	break;
	case 2585:
	{
		my_object=json_object_object_get(my_string,"terminal_code");
		string=json_object_get_string(my_object);
		if(string!=NULL)
		{
			printf("##terminal_code=%s\n", string);
			str_to_hex(string,strlen(string),code_id,code_id_len);
			memcpy(send_buf+10,code_id,6);
		}
		else
		{
			json_object_put(my_object);
			break;
		}
		json_object_put(my_object);

		my_object=json_object_object_get(my_string,"info");
		if(my_object!=NULL)
		{
			//printf("##info=%s\n", json_object_get_string(my_object));
			arrary_object=json_object_object_get(my_object,"EQUIPMENTCONTROL");
			arrary_length=json_object_array_length(arrary_object);
			printf("arrary_length=%d\n", arrary_length);


			for(i=0;i<arrary_length;i++)
			{
				printf("~~~~~~~~~~~~~~~~~~~~~~~i=%d~~~~~~~~~~~~~~~~~~~~~~~~~~\n",i);
				arrary_item=json_object_array_get_idx(arrary_object,i);
				arrary_item_data=json_object_object_get(arrary_item,"equipment_code");

				string=json_object_get_string(arrary_item_data);
				if(string!=NULL)
				{
					printf("equipment_code=%s\n", string);
					str_to_hex(string,strlen(string),code_id,code_id_len);
					memcpy(send_buf+4,code_id,6);
				}
				else
				{
					json_object_put(arrary_item_data);
					continue;
				}
				json_object_put(arrary_item_data);

				arrary_item_data=json_object_object_get(arrary_item,"show_name");
				string= json_object_get_string(arrary_item_data);
				printf("show_name=%s\n",string);
				json_object_put(arrary_item_data);


				arrary_item_data=json_object_object_get(arrary_item,"type");
				string=json_object_get_string(arrary_item_data);
				printf("type=%s\n", string);
				json_object_put(arrary_item_data);

				arrary_item_data=json_object_object_get(arrary_item,"status");
				char *stat=json_object_get_string(arrary_item_data);
				if(stat!=NULL)
				{
					printf("##  state=%s\n", stat);
				}
				else
				{
					continue;
				}

				count++;
				send_buf[18]=count;
				if(*stat==0x31)
				{
					printf("###open\n");
					send_buf[3]=1;
					//uart_send_data(UART1_DATA, send_buf,21);
				}
				else
				{
					send_buf[3]=2;
					//uart_send_data(UART1_DATA, send_buf,21);
				}
				json_object_put(arrary_item_data);

			}

		}

		json_object_put(my_object);
	}
	break;
	}

	//printf("my_string.to_string()=%s\n", json_object_get_string(my_string));
}
#define ID_LEN 32
#define NAME_LEN 32
typedef struct
{
	char id[ID_LEN];
	char name[NAME_LEN];
}id_name_match_t;

typedef struct{
	uint32_t amount;
	uint32_t max_amount;
	uint32_t last_index;
	id_name_match_t *id_name_match;
}id_name_match_list_t;


static id_name_match_list_t*id_name_match_list;

id_name_match_list_t* create_id_name_match_list(uint32_t max_amount)
{
	id_name_match_list_t*id_name_match_list=(id_name_match_list_t*)malloc(sizeof(id_name_match_list_t));
	if(id_name_match_list==NULL)
	{
		printf("malloc for id_name_match_list failured! ");
		return NULL;
	}

	id_name_match_list->id_name_match=(id_name_match_t*)malloc(sizeof(id_name_match_t)*max_amount);
	if(id_name_match_list->id_name_match==NULL)
	{
		printf("malloc for id_name_match_list->id_name_match failured! ");
		free(id_name_match_list);
		return NULL;
	}

	id_name_match_list->amount=0;
	id_name_match_list->max_amount=max_amount;
	id_name_match_list->last_index=0;
	int i=0;
	for(i=0;i<max_amount;i++)
	{
		memset(id_name_match_list->id_name_match[i].id,'\0',ID_LEN);
		memset(id_name_match_list->id_name_match[i].name,'\0',NAME_LEN);
	}

	return id_name_match_list;
}


int add_id_name_match_to_id_name_match_list(id_name_match_list_t*id_name_match_list,id_name_match_t*id_name_match)
{
	memcpy(&id_name_match_list->id_name_match[id_name_match_list->last_index],id_name_match,sizeof(id_name_match_t));
	id_name_match_list->last_index++;

	if(id_name_match_list->last_index>=id_name_match_list->max_amount)
	{
		printf("add failured,too many id_name_match ! ");
	}
	else
	{
		id_name_match_list->amount++;
	}
	return 0;
}

char* get_name_from_id_name_match_list_by_id(const id_name_match_list_t*id_name_match_list,char *id)
{
	//printf("###########id=%d\n",id);
	int i=0;
	for(i=0;i<id_name_match_list->amount;i++)
	{
		if(0 == strcmp(id_name_match_list->id_name_match[i].id, id))
		{
			return id_name_match_list->id_name_match[i].name;
		}
	}

	return NULL;
}








char *get_name_by_id(char *id)
{
	//printf("#######11####id=%d\n",id);
	if(id_name_match_list==NULL || id == NULL)
	{
		return NULL;
	}
	return get_name_from_id_name_match_list_by_id(id_name_match_list, id);
}

#define ID_LIST_MAXIMUM 10000
int init_id_match_json_data(char*filename)
{

	id_name_match_list=create_id_name_match_list(ID_LIST_MAXIMUM);

	int fd=open(filename,O_RDONLY);
	if(fd<0)
	{
		printf("open %s error!\n",filename);
		return -1;
	}
	char *data=malloc(600*1024);
	memset(data,'\0',600*1024);
	int readlen=read(fd,data,600*1024);
	if(readlen>0)
	{
		json_object *my_string, *my_object,*arrary_object,*arrary_item;
		my_string = json_tokener_parse(data);
	//	printf("json:%s\n",data);
		arrary_object=json_object_object_get(my_string,"listdata");

		/*		arrary_item_data=json_object_object_get(arrary_item,"name");*/
		//char *string=json_object_get_string(arrary_object);
		//printf("arrary_object=%s\n",string);
		//return 0;

		if(arrary_object!=NULL)
		{
			int arrary_length=json_object_array_length(arrary_object);
			printf("arrary_length=%d\n", arrary_length);
			if(arrary_length > ID_LIST_MAXIMUM){
                arrary_length = ID_LIST_MAXIMUM;
                printf("to avoid overflow,change the arrary_length to %d",ID_LIST_MAXIMUM);
			}
			int i=0;
			for(i=0;i<arrary_length;i++)
			{
				arrary_item=json_object_array_get_idx(arrary_object,i);
				json_object *arrary_item_name=json_object_object_get(arrary_item,"name");
				json_object *arrary_item_id=json_object_object_get(arrary_item,"id");
				char *name=json_object_get_string(arrary_item_name);
				char *id=json_object_get_string(arrary_item_id);
				if(name!=NULL)
				{
					id_name_match_t id_name_match={
							.id={'\0'},
							.name={'\0'},
					};
					memcpy(id_name_match.name,name,strlen(name));//TODO. in case strlen > NAME_LEN
					memcpy(id_name_match.id,id,strlen(id));
					add_id_name_match_to_id_name_match_list(id_name_match_list,&id_name_match);
					//printf("id=%d     name=%s  \n",id,name);
				}

				//printf("get_name:%s \n",get_name_from_id_name_match_list_by_id(id_name_match_list,id));
				//
				json_object_put(arrary_item_name);
				json_object_put(arrary_item_id);

			}
		}
	}
	if(data != NULL){
	    free(data);
	    data=NULL;
	}
	close(fd);

	return 0;

}

int get_threshold_json_data(char*filename, char *key)
{
	int threshold=-1;
	int fd=open(filename,O_RDONLY);
	if(fd<0)
	{
		printf("open %s error!\n",filename);
		return -1;
	}
	char *data=alloca(1024);
	memset(data,'\0',1024);
	int readlen=read(fd,data,1024);
	if(readlen>0)
	{
		json_object *my_string=NULL, *my_object=NULL;//TODO memory leak problem, only run once
		my_string = json_tokener_parse(data);
		//printf("json:%s\n",data);
		my_object=json_object_object_get(my_string,key);
		if(my_object!=NULL)
		{
			threshold=json_object_get_int(my_object);
			json_object_put(my_object);
			printf("threshold=%d\n",threshold);
		}

	}
	close(fd);

	return threshold;

}

char *get_string_json_data(char*filename, char *key)
{
	char *string = NULL;
	int fd=open(filename,O_RDONLY);
	if(fd<0)
	{
		printf("open %s error!\n",filename);
		return -1;
	}
	char *data=alloca(1024);
	memset(data,'\0',1024);
	int readlen=read(fd,data,1024);
	if(readlen>0)
	{
		json_object *my_string=NULL, *my_object=NULL;//TODO memory leak problem, only run once
		my_string = json_tokener_parse(data);
		//printf("json:%s\n",data);
		my_object=json_object_object_get(my_string,key);
		if(my_object!=NULL)
		{
			string=json_object_get_string(my_object);
// 			json_object_put(my_object);
			printf("string=%s\n",string);
		}

	}
	close(fd);

	return string;

}
