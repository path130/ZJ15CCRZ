
#ifndef HEADER_AES_H
#define HEADER_AES_H


#ifdef  __cplusplus
extern "C" {
#endif


#define AES_MODE_ECB    0
#define AES_MODE_CBC    1
#define AES_MODE_CFB    2
#define AES_MODE_OFB    3
#define AES_MODE_CTR    4
#define AES_MODE_SIC    5


#define AES_MAXNR        14
#define AES_BLOCK_SIZE   16


struct aes_key_st
{
    unsigned int rd_key[4 *(AES_MAXNR + 1)];
    int rounds;
};
typedef struct aes_key_st AES_KEY;


struct aes_st
{
	int  mode;
	AES_KEY  enc_key;
	AES_KEY  dec_key;
	unsigned char  enc_iv[AES_BLOCK_SIZE];
	unsigned char  dec_iv[AES_BLOCK_SIZE];
	unsigned char  enc_cnt[AES_BLOCK_SIZE];
	unsigned char  dec_cnt[AES_BLOCK_SIZE];
	unsigned int   enc_num;
	unsigned int   dec_num;
};
typedef struct aes_st AES;



// 创建AES对象
//   注意   创建后密钥为空，必需设置密钥
//   返回   AES对象指针，如果为NULL表明创建失败
//   参数
AES *AES_New(void);


// 释放AES对象
//   返回
//   参数   aes -- AES对象指针
void AES_Free(AES *aes);


// 设置工作模式
//   注意   创建后默认工作模式为CTR模式
//          如果使用ECB或CBC模式，加密和解密时所有数据长度必须是AES_BLOCK_SIZE的整数倍 !!!
//   返回   <0 失败； =0 成功
//   参数   aes  -- AES对象指针
//          mode -- AES工作模式
int  AES_SetMode(AES *aes, const int mode);


// 设置密钥
//   返回   <0 失败； =0 成功
//   参数   aes     -- AES对象指针
//          userKey -- AES密钥指针
//          bits    -- AES密钥位数，必须是128、192、或256位
int  AES_SetKey(AES *aes, const unsigned char *userKey, const int bits);


// 设置加密初始向量
//   返回   
//   参数   aes     -- AES对象指针
//          initvec -- AES初始向量
void AES_SetEncInitVec(AES *aes, const unsigned char *initvec);


// 设置解密初始向量
//   返回   
//   参数   aes     -- AES对象指针
//          initvec -- AES初始向量
void AES_SetDecInitVec(AES *aes, const unsigned char *initvec);


// 数据加密
//   注意   加密后的数据与原始数据长度相同，因此输出的加密数据缓冲区长度必须保证 >= length
//   返回   <0 失败； =0 成功
//   参数   aes    -- AES对象指针
//          inbuf  -- 输入的原始数据缓冲区指针
//          outbuf -- 输出的加密数据缓冲区指针，可以与inbuf相同
//          length -- 输入的原始数据长度（字节）
int  AES_DataEncrypt(AES *aes, const unsigned char *inbuf, unsigned char *outbuf, int length);


// 数据解密
//   注意   解密后的数据与加密数据长度相同，因此输出的解密数据缓冲区长度必须保证 >= length
//   返回   <0 失败； =0 成功
//   参数   aes    -- AES对象指针
//          inbuf  -- 输入的加密数据缓冲区指针
//          outbuf -- 输出的解密数据缓冲区指针，可以与inbuf相同
//          length -- 输入的加密数据长度（字节）
int  AES_DataDecrypt(AES *aes, const unsigned char *inbuf, unsigned char *outbuf, int length);


#ifdef  __cplusplus
}
#endif

#endif//HEADER_AES_H
