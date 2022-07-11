#ifndef ____DATA_CONVERT_H____
#define ____DATA_CONVERT_H____

#include "string.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"

typedef union dTu8
{
	double f_val;
	uint8_t u8_val[8];
}D2U8;

typedef union fTu8
{
	float f_val;
	uint8_t u8_val[4];
}F2U8;

typedef union uTu8
{
	unsigned short f_val;
	uint8_t u8_val[2];
}U2U8;

typedef union sTu8
{
	signed short f_val;
	uint8_t u8_val[2];
}S2U8;

int ftoa(char* s, double d, int n);
void strReverse(char* str);
int strInStrCount(char *str, const char*delimiter);
int strSplit( char **arr, char *str, const char *delimiter);
char *strReplace(char *str,char *oldstr,char *newstr);
/**
 * @brief convert uint32_t to uint8_t buffer
 * 
 * @param data 
 * @param pBuf 
 * @return int 1 success, 0 fail
 */
int u32ToBuf(uint32_t* data, uint8_t* pBuf);
/**
 * @brief convert a uint8_t buffer to uint32_t
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int bufToU32(uint8_t* pBuf, uint32_t* data);
/**
 * @brief convert a 4 bytes uint8_t buffer to float
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int bufToFloat(uint8_t* pBuf, float* data);
/**
 * @brief convert float to uint8_t buffer
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int floatToBuf(uint8_t* pBuf,float * data);
/**
 * @brief convert double to string
 * 
 * @param df 
 * @param str 
 * @param nDights 小数点后位数
 * @return int 1 success, 0 fail
 */
int dtoc(double * df, uint8_t* str, uint8_t nDights);
/**
 * @brief convert integer to string
 * 
 * @param integer 
 * @param str 
 * @return int int 1 success, 0 fail
 */
int itoa(long int* integer, uint8_t* str);
/**
 * @brief convert integer to string
 * 
 * @param dest 
 * @param source 
 * @param nDigits 对齐位数
 */
void itoc(char* dest, int source, int nDigits);
/**
 * @brief convert to upper string
 * 
 * @param source 
 */
void strt2upper(char *source);
/**
 * @brief convert to lower string
 * 
 * @param source 
 */
void str2lower(char *source);
/**
 * @brief Get the Number object from string
 * 
 * @param source 
 * @param number 
 * @return uint8_t 0 fail, 1 success
 */
uint8_t GetNumber(char *source, int *number);
/**
 * @brief Get the Double object from string
 * 
 * @param source 
 * @param number 
 * @return uint8_t 0 fail, 1 success
 */
uint8_t GetDouble(char *source, double *number);
int data24BitToSigned(uint32_t Data);
int bufToSigned(uint8_t* pBuf, signed short* data);
int SignedToBuf(uint8_t* pBuf,signed short * data);
int bufToUnsigned(uint8_t* pBuf, unsigned short* data);
int UnsignedToBuf(uint8_t* pBuf,unsigned short * data);

void InvertUint8(uint8_t *DesBuf, uint8_t *SrcBuf);
void InvertUint16(uint16_t *DesBuf, uint16_t *SrcBuf);
void InvertUint32(uint32_t *DesBuf, uint32_t *SrcBuf);

#endif // !____DATA_CONVERT_H____
