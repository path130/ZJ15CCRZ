
#define PDU_PROD_NAME         "�㶫���ӱ�����Ƽ��ɷ����޹�˾"
#define PDU_DESCRIPTION       "��������"
#define PDU_ANIMAWORD         "���ӱ�¥��Խ�ϵͳ"
#define PDU_HW_VER            "X3-A07840825"
#ifdef FRONT_DOOR
    #ifdef _FACE_DET
#define PDU_EQU_DESC          "���ӱ�AJB-ZJ15CCRZBIP����"
#define PDU_EQU_NAME          "AJB-ZJ15CCRZ����"
    #else
#define PDU_EQU_DESC          "���ӱ�AJB-ZJ15CCZBIP����"
#define PDU_EQU_NAME          "AJB-ZJ15CCZ����"        
    #endif
#define PDU_TYPE_NAME         "����"
#define PDU_POSITION          "���ſ�"

#else
    #ifdef _FACE_DET    
#define PDU_EQU_DESC          "���ӱ�AJB-ZJ15CCRZIP����"
#define PDU_EQU_NAME          "AJB-ZJ15CCRZ����"
    #else
#define PDU_EQU_DESC          "���ӱ�AJB-ZJ15CCZIP����"
#define PDU_EQU_NAME          "AJB-ZJ15CCZ����"        
    #endif
#define PDU_TYPE_NAME         "����"
#define PDU_POSITION          "�ſ�"
#endif