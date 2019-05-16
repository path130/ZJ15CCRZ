#ifndef _SENSOR_H_
#define _SENSOR_H_

int SensorStart(char is_night);
int SensorStop();
int SensorRead(char *buf);
extern int ae_rule_algo_init(char is_night,char call_kzq,char is_call);
extern int ae_rule_algo_auto_sw(void);

#endif