
#define PDU_PROD_NAME         "广东安居宝数码科技股份有限公司"
#define PDU_DESCRIPTION       "可以升级"
#define PDU_ANIMAWORD         "安居宝楼宇对讲系统"
#define PDU_HW_VER            "X3-A07840825"
#ifdef FRONT_DOOR
    #ifdef _FACE_DET
#define PDU_EQU_DESC          "安居宝AJB-ZJ15CCRZBIP主机"
#define PDU_EQU_NAME          "AJB-ZJ15CCRZ主机"
    #else
#define PDU_EQU_DESC          "安居宝AJB-ZJ15CCZBIP主机"
#define PDU_EQU_NAME          "AJB-ZJ15CCZ主机"        
    #endif
#define PDU_TYPE_NAME         "主机"
#define PDU_POSITION          "大门口"

#else
    #ifdef _FACE_DET    
#define PDU_EQU_DESC          "安居宝AJB-ZJ15CCRZIP主机"
#define PDU_EQU_NAME          "AJB-ZJ15CCRZ主机"
    #else
#define PDU_EQU_DESC          "安居宝AJB-ZJ15CCZIP主机"
#define PDU_EQU_NAME          "AJB-ZJ15CCZ主机"        
    #endif
#define PDU_TYPE_NAME         "主机"
#define PDU_POSITION          "门口"
#endif