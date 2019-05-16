#include <string.h>
#include <stdlib.h>
#include "udp_pc_task.h"
#include "net_com.h"
#include "public.h"
#include "dev_info.h"
#include "dev_pro.h"
#include "dev_config.h"
#include "global_data.h"

static pdu_head opdu={};

static void SetExOpdu(pdu_head *pHeader)
{
	if ( (!IsMyIp(pHeader->ip_addr)||is_my_subnet_mask(pHeader->subnet_mask)
				||is_my_gateway_addr(pHeader->gateway_addr))
				&& IsMyMac(pHeader->mac_addr))
	{
		eth_para_t para;
		memcpy(&para.dhcp_ebl,&pHeader->dhcp_ebl,sizeof(eth_para_t));

        	memcpy(opdu.dns1,pHeader->dns1,4);
		memcpy(opdu.dns2,pHeader->dns2,4);

		memcpy(dev_cfg.dns1,pHeader->dns1,4);
		memcpy(dev_cfg.dns2,pHeader->dns2,4);
		dev_config_save();
        
		if (save_eth_para(&para) < 0) return;
		save_dev_eth_para(&para);
		usleep(200000);
		printf("save OK\n\n");
		//system("reboot");
           exit(0);
	}
}



void InitOpdu(pdu_head * pHeader)
{
	opdu.dhcp_ebl = pHeader->dhcp_ebl;
	memcpy(opdu.mac_addr,pHeader->mac_addr,8);
	memcpy(opdu.ip_addr,pHeader->ip_addr,4);
	memcpy(opdu.subnet_mask,pHeader->subnet_mask,4);
	memcpy(opdu.gateway_addr,pHeader->gateway_addr,4);
	memcpy(opdu.dns1,pHeader->dns1,4);
	memcpy(opdu.dns2,pHeader->dns2,4);
	memcpy(opdu.http_port,pHeader->http_port,4);

	memcpy(opdu.equ_description,pHeader->equ_description,64);
	memcpy(opdu.equ_name,pHeader->equ_name,32);
	opdu.equ_type = pHeader->equ_type;
	memcpy(opdu.prod_name,pHeader->prod_name,64);
	memcpy(opdu.type_name,pHeader->type_name,32);

	memcpy(opdu.hardware_version,pHeader->hardware_version,16);
	memcpy(opdu.software_version,pHeader->software_version,16);
	memcpy(opdu.position,pHeader->position,32);
	memcpy(opdu.description,pHeader->description,64);
}


void udp_pc_process_task(int socketfd)
{
    app_debug(DBG_INFO,"udp_process_task starting!\n");
    int recv_len;
    pdu_head ipdu;
    int arp_rec_run=1;
    char i;
   // InitUdpEcho();
    while(arp_rec_run)
    {
    	struct sockaddr_in client;
        recv_len=recv_data_from(socketfd,&ipdu,sizeof(pdu_head),(struct sockaddr *)(&client));
        if(recv_len<0)
        	continue;
        else if(ipdu.op_code == 2)//来自服务器的普适查询信息
        {

        	opdu.op_code = 5;
        	client.sin_port= htons(6667);
        	send_data_to(socketfd,(void *)&opdu,sizeof(pdu_head),(struct sockaddr *)(&client));
        }
        else if( ipdu.op_code == 4)//来自服务器的设置信息
        {
        	app_debug(DBG_INFO,"SetExOpdu!!!!!!!!!! \n");
        	SetExOpdu(&ipdu);
        }
    }
    app_debug(DBG_INFO,"udp_process_task closed!\n");
}
