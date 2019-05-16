/*
 * json_data_pro.h
 *
 *  Created on: 2015-3-4 下午1:43:49
 *  
 */

#ifndef JSON_DATA_PRO_H_
#define JSON_DATA_PRO_H_

#include "json.h"

extern int setup_json_face_threhold;
extern char setup_json_server_ip[20];
extern int setup_json_face_size;
extern int setup_json_server_port;
extern int setup_json_stop_period;
extern int setup_json_cam_algo;
extern int setup_json_algo_sw_wait;

char *get_name_by_id(char *id);
int init_id_match_json_data(char*filename);
int get_threshold_json_data(char*filename, char *key);
char *get_string_json_data(char*filename, char *key);

#endif /* JSON_DATA_PRO_H_ */
