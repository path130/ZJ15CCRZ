/*
 * speech_pro.c
 *
 *  Created on: 2017年7月4日
 *      Author: chli
 */
#include "speech_pro.h"
#include "ajb_net_speech.h"

static ajb_net_speech_handle_t*ajb_net_speech_handle=NULL;
int  speech_start(char *ip,int capture_volume,int paly_volume,int mode)
{
	ajb_net_speech_attr_t ajb_net_speech_attr;
	ajb_net_speech_attr.dest_ip=ip;
	ajb_net_speech_attr.dest_port=6668;
	ajb_net_speech_attr.local_port=6668;
	ajb_net_speech_attr.speech_trans_attr. samplerate=8000;
	ajb_net_speech_attr.speech_trans_attr. nchannel=1;
	ajb_net_speech_attr.speech_trans_attr. audio_capture_callback_fun=speech_trans_read_alaw_data_callback;
	ajb_net_speech_attr.speech_trans_attr.audio_capture_callback_data=NULL;
	ajb_net_speech_attr.speech_trans_attr.aec_enable=1;
		ajb_net_speech_attr.speech_trans_attr.encode_format=PCM_ALAW;

		switch(ajb_net_speech_attr.speech_trans_attr.encode_format)
		{
		case PCM_LINEAR:
		{
			ajb_net_speech_attr.speech_trans_attr. audio_capture_callback_fun=speech_trans_read_data_callback;
		}
		break;
		case PCM_ALAW:
		{
			ajb_net_speech_attr.speech_trans_attr. audio_capture_callback_fun=speech_trans_read_alaw_data_callback;
		}
		break;
		}
	ajb_net_speech_handle=new_ajb_net_speech_handle(&ajb_net_speech_attr);
	if(capture_volume>-1)
	{
	    //capture_volume_set(capture_volume);
	}


	start_ajb_net_speech(ajb_net_speech_handle);
	if(paly_volume>-1)
	{
	 	ajb_net_speech_set_play_volume(ajb_net_speech_handle,  paly_volume);
	}
	return 0;

}

int  cloud_speech_start(char *ip,int capture_volume,int paly_volume,int mode)
{
ajb_net_speech_attr_t ajb_net_speech_attr;
		ajb_net_speech_attr.dest_ip=ip;
		ajb_net_speech_attr.dest_port=6668;
		ajb_net_speech_attr.local_port=6668;
		ajb_net_speech_attr.speech_trans_attr. samplerate=8000;
		ajb_net_speech_attr.speech_trans_attr. nchannel=1;
		ajb_net_speech_attr.speech_trans_attr. audio_capture_callback_fun=speech_trans_read_alaw_data_callback;
		ajb_net_speech_attr.speech_trans_attr.audio_capture_callback_data=NULL;
		ajb_net_speech_attr.speech_trans_attr.aec_enable=1;
		ajb_net_speech_attr.speech_trans_attr.encode_format=PCM_LINEAR;

		switch(ajb_net_speech_attr.speech_trans_attr.encode_format)
		{
		case PCM_LINEAR:
		{
			ajb_net_speech_attr.speech_trans_attr. audio_capture_callback_fun=speech_trans_read_data_callback;
		}
		break;
		case PCM_ALAW:
		{
			ajb_net_speech_attr.speech_trans_attr. audio_capture_callback_fun=speech_trans_read_alaw_data_callback;
		}
		break;
		}
	ajb_net_speech_handle=new_ajb_net_speech_handle(&ajb_net_speech_attr);
	if(capture_volume>-1)
	{
	    //capture_volume_set(capture_volume);
	}


	start_ajb_net_speech(ajb_net_speech_handle);
	if(paly_volume>-1)
	{
	 	ajb_net_speech_set_play_volume(ajb_net_speech_handle,  paly_volume);
	}
	return 0;
		
}
int  speech_stop(void)
{
	if(ajb_net_speech_handle!=NULL)
	{
		stop_ajb_net_speech(ajb_net_speech_handle);
		delete_ajb_net_speech_handle(ajb_net_speech_handle);
		ajb_net_speech_handle=NULL;
	}
return 0;
}

int speech_volume_set(int volume)
{
	ajb_net_speech_set_play_volume(ajb_net_speech_handle,  volume);
	return 0;
}

