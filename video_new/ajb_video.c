/*
 * ajb_video.c
 *
 *  Created on: 2016年10月17日
 *      Author: chli
 */
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "ajb_video.h"
#include "ajb_rtpc.h"
//#include "rtpc.h"
#include "public.h"
#include <errno.h>

#ifndef DEBUG_CONNECTION_MOD


ajb_video_service_create_t ajb_video_service_create_ptr;
ajb_create_ajb_video_handle_t ajb_create_ajb_video_handle_ptr;
ajb_create_ajb_video_handle_t ajb_create_ajb_video_handle_ptr;
ajb_video_playback_t ajb_video_playback_ptr;



ajb_dl_video_handle_t*new_ajb_dl_video_handle(char*dl_name)
{
	char *error_msg=NULL;
	ajb_dl_video_handle_t*ajb_dl_video_handle=(ajb_dl_video_handle_t*)calloc(1,sizeof(ajb_dl_video_handle_t));
	if(ajb_dl_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"calloc for  ajb_dl_video_handle failed,: %s!\n",strerror(errno));
		return NULL;
	}

	ajb_dl_video_handle->dl_handle = dlopen (dl_name, RTLD_LAZY);
	if (ajb_dl_video_handle->dl_handle==NULL)
	{
		app_debug(DBG_FATAL,"dlopen %s failed,error: %s!\n",dl_name,dlerror());
		return NULL;
	}

	ajb_dl_video_handle->ajb_video_service_create_ptr= dlsym(ajb_dl_video_handle->dl_handle, "ajb_video_service_create");
	if ((error_msg = dlerror()) != NULL)
	{
		app_debug(DBG_FATAL,"dlsym for ajb_video_service_create  failed,error:%s !\n",error_msg);
		ajb_dl_video_handle->ajb_video_service_create_ptr=NULL;
	}

	ajb_dl_video_handle->ajb_create_ajb_video_handle_ptr= dlsym(ajb_dl_video_handle->dl_handle , "ajb_create_ajb_video_handle");
	if ((error_msg = dlerror()) != NULL)
	{
		app_debug(DBG_FATAL,"dlsym for ajb_create_ajb_video_handle  failed,error: %s!\n",error_msg);
		ajb_dl_video_handle->ajb_create_ajb_video_handle_ptr=NULL;
	}

	ajb_dl_video_handle->ajb_delete_ajb_video_handle_ptr= dlsym(ajb_dl_video_handle->dl_handle , "ajb_delete_ajb_video_handle");
	if ((error_msg = dlerror()) != NULL)
	{
		app_debug(DBG_FATAL,"dlsym for ajb_delete_ajb_video_handle  failed,error:%s !\n",error_msg);
		ajb_dl_video_handle->ajb_delete_ajb_video_handle_ptr=NULL;
	}

	ajb_dl_video_handle->ajb_video_capture_ptr= dlsym(ajb_dl_video_handle->dl_handle , "ajb_video_capture");
	if ((error_msg = dlerror()) != NULL)
	{
		app_debug(DBG_FATAL,"dlsym for ajb_video_capture  failed,error:%s !\n",error_msg);
		ajb_dl_video_handle->ajb_video_capture_ptr=NULL;
	}

	ajb_dl_video_handle->ajb_video_playback_ptr= dlsym(ajb_dl_video_handle->dl_handle , "ajb_video_playback");
	if ((error_msg = dlerror()) != NULL)
	{
		app_debug(DBG_FATAL,"dlsym for ajb_video_playback  failed,error: %s!\n",error_msg);
		ajb_dl_video_handle->ajb_video_playback_ptr=NULL;
	}

	ajb_dl_video_handle->ajb_video_take_photo_ptr= dlsym(ajb_dl_video_handle->dl_handle , "ajb_video_take_photo");
	if ((error_msg = dlerror()) != NULL)
	{
		app_debug(DBG_FATAL,"dlsym for ajb_video_take_photo  failed,error:%s !\n",error_msg);
		ajb_dl_video_handle->ajb_video_take_photo_ptr=NULL;
	}

	return ajb_dl_video_handle;
}

void delete_ajb_dl_video_handle(ajb_dl_video_handle_t*ajb_dl_video_handle)
{
	if(ajb_dl_video_handle!=NULL)
	{
		if(ajb_dl_video_handle->dl_handle!=NULL)
		{
			dlclose(ajb_dl_video_handle->dl_handle);
			ajb_dl_video_handle->dl_handle=NULL;
		}
		free(ajb_dl_video_handle);
		ajb_dl_video_handle=NULL;
	}
}


 int init_ajb_video_dl(char*dl_name)
 {
		void *handle;
		char *error;

		handle = dlopen (dl_name, RTLD_LAZY);
		if (!handle) {
			fprintf (stderr, "%s\n", dlerror());
			exit(1);
		}

/*		int i=0;
		for(i=0;i<sizeof(video_handle_dl_array)/sizeof(dl_array_t);i++)
		{
			video_handle_dl_array[i] .fun= dlsym(handle, video_handle_dl_array[i].fun_name);
			if ((error = dlerror()) != NULL)
			{
				fprintf (stderr, "%s\n", error);
				exit(1);
			}
		}*/

		ajb_video_playback_ptr= dlsym(handle, "ajb_video_playback");
				if ((error = dlerror()) != NULL)
				{
					fprintf (stderr, "%s\n", error);
					exit(1);
				}


				ajb_video_playback_ptr(NULL,NULL,0);
		//printf ("%f\n",);
		dlclose(handle);
		return 0;


 }











/*




int ajb_video_service_create(const ajb_video_service_attr_t*attr)
{
	return 0;
}


ajb_video_handle_t*ajb_create_ajb_video_handle(ajb_video_attr_t*attr)
{
	ajb_video_handle_t*handle=NULL;
	return handle;
}

void ajb_delete_ajb_video_handle(ajb_video_handle_t*handle)
{

}

int ajb_video_capture(ajb_video_handle_t* handle,void*buf,int length)
{
	return 0;
}

int ajb_video_playback(ajb_video_handle_t* handle,void*buf,int length)
{
	return 0;
}

int ajb_video_take_photo(ajb_video_handle_t* handle,void*buf,int length)
{
	return 0;
}
*/

#endif
