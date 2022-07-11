#include "data_convert.h"
#include "string.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"


int ftoa(char* s, double d, int n)
{
	int     sumI;
	float   sumF;
	int     sign = 0;
	int     temp;
	int     count = 0;
	char *p;
	char *pp;
	if(s == NULL) return -1;
	p = s;
	/*Is less than 0*/
	if(d < 0)
	{
		sign = 1;
		d = 0 - d;
	}
	sumI = (int)d;    //sumI is the part of int
	sumF = d - sumI;  //sumF is the part of float
	/*Int ===> String*/
	do
	{
		temp = sumI % 10;
		*(s++) = temp + '0';
	}while((sumI = sumI /10) != 0);
	/*******End*******/
	if(sign == 1)
	{
		*(s++) = '-';
	}
	pp = s;
	pp--;
	while(p < pp)
	{
		*p = *p + *pp;
		*pp = *p - *pp;
		*p = *p -*pp;
		p++;
		pp--;
	}
	*(s++) = '.';     //point
	/*Float ===> String*/
	do
	{
		temp = (int)(sumF*10);
		*(s++) = temp + '0';
		if((++count) == n)
			break;
		sumF = sumF*10 - temp;
	}while(!(sumF > -0.000001f && sumF < 0.000001f));
	*s = '\0';
	return 0;
}

int dec2HexStr(int dec, char* hex)
{
	sprintf(hex,"%d",dec);
	return 1;
}

void strReverse(char* str)
{
	int i,j;
	j=strlen(str);
	for(i=0,j--;i<j;i++,j--)
	{
		str[i] = str[i] ^ str[j];
		str[j] = str[j] ^ str[i];
		str[i] = str[j] ^ str[i];
	}
}
/*
*	目标字符串在源字符串存在的个数
*/
int strInStrCount(char *str, const char*delimiter) 
{
	char *first = NULL;
	char *second = NULL;
	int num = 0;

	first = strstr(str,delimiter);
	while(first != NULL)
	{
		second = first+1;
		num++;
		first = strstr(second,delimiter);
	}
	return num;
}

/***
 * 	以目标字符串分割源字符串，并将分割后的字符串存储
 * */
int strSplit( char **arr, char *str, const char *delimiter)
{
	char *s =NULL;
	if((arr==NULL)||(str == NULL)||(delimiter == NULL))
		return 0;
	s=strtok(str,delimiter);
	if(s==NULL)
		return 0;
	while(s != NULL)
	{
		*arr++ = s;
		s = strtok(NULL,delimiter);
	}
	return 1;
}

/* 功  能：将str字符串中的oldstr字符串替换为newstr字符串
 * 参  数：str：操作目标 oldstr：被替换者 newstr：替换者
 * 返回值：返回替换之后的字符串
 * 版  本： V0.2
 */
char *strReplace(char *str,char *oldstr,char *newstr)
{
	int i;
	char *bstr = (char*)malloc(strlen(str));//转换缓冲区
	memset(bstr,0,sizeof(strlen(str)));
	for(i = 0;i < strlen(str);i++){
		if(!strncmp(str+i,oldstr,strlen(oldstr))){//查找目标字符串
			strcat(bstr,newstr);
			i += strlen(oldstr) - 1;
		}else{
			strncat(bstr,str + i,1);//保存一字节进缓冲区
		}
	}
	strcpy(str,bstr);
	free(bstr);
	return str;
} 

void float2Hex(float value,uint8_t *disBuf,uint16_t pos)
{
	uint8_t i;
	F2U8 f;
	f.f_val=value;
	for( i=0;i<4;i++)
	{
		disBuf[pos+i]=f.u8_val[i];
	}
}

void double2Hex(double value,uint8_t *disBuf,uint16_t pos)
{
	uint8_t i;
	D2U8 d;
	d.f_val=value;
	for( i=0;i<8;i++)
	{
		disBuf[pos+i]=d.u8_val[i];
	}
}

double hex2Double(uint8_t * buf)
{
	D2U8 d;
	memcpy(d.u8_val,buf,8);
	return d.f_val;
}

float hex2Float(uint8_t * buf)
{
	F2U8 f;
	memcpy(f.u8_val,buf,4);
	return f.f_val;
}

/**
 * @brief convert uint32_t to uint8_t buffer
 * 
 * @param data 
 * @param pBuf 
 * @return int 1 success, 0 fail
 */
int u32ToBuf(uint32_t* data, uint8_t* pBuf)
{
	if((pBuf == NULL)||(data == NULL))
		return 0;
	*pBuf = (uint8_t)(*data>>24);
	pBuf++;
	*pBuf = (uint8_t)(*data>>16);
	pBuf++;
	*pBuf = (uint8_t)(*data>>8);
	pBuf++;
	*pBuf = (uint8_t)(*data);
	return 1;
}
/**
 * @brief convert a uint8_t buffer to uint32_t
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int bufToU32(uint8_t* pBuf, uint32_t* data)
{
	if((pBuf == NULL)||(data == NULL))
		return 0;
	*data = *pBuf;
	*data = *data <<8;
	pBuf ++;
	*data += *pBuf;
	*data = *data <<8;
	pBuf ++;
	*data += *pBuf;
	*data = *data <<8;
	pBuf ++;
	*data += *pBuf;
	return 1;
}
/**
 * @brief convert a 4 bytes uint8_t buffer to float
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int bufToFloat(uint8_t* pBuf, float* data)
{
	uint8_t i;
	F2U8 f;
	if((pBuf == NULL)||(data == NULL))
		return 0;
	for(i=0;i<4;i++)
	{
		f.u8_val[i]=pBuf[i];
	}
	*data = f.f_val;
	return 1;
}
/**
 * @brief convert float to uint8_t buffer
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int floatToBuf(uint8_t* pBuf,float * data)
{
	uint8_t i;
	F2U8 f;
	if((pBuf == NULL)||(data == NULL))
		return 0;
	f.f_val=*data;
	for(i=0;i<4;i++)
	{
		pBuf[i]=f.u8_val[i];
	}
	return 1;
}
/**
 * @brief convert double to string
 * 
 * @param df 
 * @param str 
 * @param nDights 小数点后位数
 * @return int 1 success, 0 fail
 */
int dtoc(double * df, uint8_t* str, uint8_t nDights)
{
	char ch[200] = {0};
	char format[8] = {'%','.'};
	if((df == NULL)||(str==NULL))
		return 0;
	sprintf(&format[2],"%df",nDights);
	sprintf(ch,format,*df);
	memcpy(str,ch,strlen(ch));
	str[strlen(ch)] = 0;
	return 1;
}
/**
 * @brief convert integer to string
 * 
 * @param integer 
 * @param str 
 * @return int int 1 success, 0 fail
 */
int itoa(long int* integer, uint8_t* str)
{
	char tStr[100] = {0};
	if((integer == NULL)||(str==NULL))
		return 0;
	sprintf(tStr,"%ld",*integer);
	memcpy(str,tStr,strlen(tStr));
	str[strlen(tStr)] = 0;
	return 1;
}
/**
 * @brief convert integer to string
 * 
 * @param dest 
 * @param source 
 * @param nDigits 对齐位数
 */
void itoc(char* dest, int source, int nDigits)
{
	char temp[60] = {0};
	char format[20] = {0};
	int nLength;
	if(dest == NULL || nDigits >= 60)
		return;
	format[0] = '%';
	format[1] = '0';
	sprintf(&format[2], "%dd", nDigits);
	sprintf(temp, format, source);
	nLength = strlen(temp);
	memcpy(dest, temp, nLength);	
	dest[nLength] = 0x00;
}
/**
 * @brief convert to upper string
 * 
 * @param source 
 */
void strt2upper(char *source)  
{ 
	char *c;
	int i = 0;
	while(1)
	{
		c = &source[i++];
		if(*c == 0 || i > 20)
			break;
		if(*c >= 'a' && *c <= 'z')  
			*c += 'A' - 'a';   
	}
} 
/**
 * @brief convert to lower string
 * 
 * @param source 
 */
void str2lower(char *source)
{
	char *c;
	int i = 0;
	while(1)
	{
		c = &source[i++];
		if(*c == 0 || i > 20)
			break;
		if(*c >= 'a' && *c <= 'z')  
			*c -= 'A' - 'a'; 
	}	
}
/**
 * @brief Get the Number object from string
 * 
 * @param source 
 * @param number 
 * @return uint8_t 0 fail, 1 success
 */
uint8_t GetNumber(char *source, int *number)
{
	int8_t i;
	int8_t nFind = -1;
	if(number == NULL || source == NULL)
		return 0;
	for(i = 0; i < 20; i++)
	{
		if((source[i] >= '0' && source[i] <= '9') || (source[i] == '-' && (source[i+1] >= '0' && source[i+1] <= '9')))
		{
			nFind = i;
			break;
		}
		else if(source[i] == 0x00)
			break;
	}
	if(nFind >= 0)
	{
		*number = atoi(&source[nFind]);
		return 1;
	}
	return 0;
}
/**
 * @brief Get the Double object from string
 * 
 * @param source 
 * @param number 
 * @return uint8_t 0 fail, 1 success
 */
uint8_t GetDouble(char *source, double *number)
{
	int8_t i;
	int8_t nFind = -1;
	if(number == NULL || source == NULL)
		return 0;
	for(i = 0; i < 20; i++)
	{
		if((source[i] >= '0' && source[i] <= '9') || (source[i] == '-' && (source[i+1] >= '0' && source[i+1] <= '9')))
		{
			nFind = i;
			break;
		}
		else if(source[i] == 0x00)
			break;
	}
	if(nFind >= 0)
	{
		*number = atof(&source[nFind]);
		return 1;
	}
	return 0;
}


/**
 * @brief convert a 4 bytes uint8_t buffer to float
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int bufToSigned(uint8_t* pBuf, signed short* data)
{
	uint8_t i;
	S2U8 s;
	if((pBuf == NULL)||(data == NULL))
		return 0;
	
	for( i=0;i<2;i++)
	{
		s.u8_val[i]=pBuf[i];
	}
	*data = s.f_val;
	return 1;
}
/**
 * @brief convert float to uint8_t buffer
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int SignedToBuf(uint8_t* pBuf,signed short * data)
{
	uint8_t i;
	S2U8 s;
	if((pBuf == NULL)||(data == NULL))
		return 0;
	s.f_val=*data;
	for( i=0;i<2;i++)
	{
		pBuf[i]=s.u8_val[i];
	}
	return 1;
}

/**
 * @brief convert a 4 bytes uint8_t buffer to float
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int bufToUnsigned(uint8_t* pBuf, unsigned short* data)
{
	uint8_t i;
	U2U8 s;
	if((pBuf == NULL)||(data == NULL))
		return 0;
	
	for( i=0;i<2;i++)
	{
		s.u8_val[i]=pBuf[i];
	}
	*data = s.f_val;
	return 1;
}
/**
 * @brief convert float to uint8_t buffer
 * 
 * @param pBuf 
 * @param data 
 * @return int 1 success, 0 fail
 */
int UnsignedToBuf(uint8_t* pBuf,unsigned short * data)
{
	uint8_t i;
	U2U8 s;
	if((pBuf == NULL)||(data == NULL))
		return 0;
	
	s.f_val=*data;
	for( i=0;i<2;i++)
	{
		pBuf[i]=s.u8_val[i];
	}
	return 1;
}

int data24BitToSigned(uint32_t Data)
{
	int temp;
	if((Data&0x800000)>0)
	{
		Data |= 0xff000000; //高位补1
	}
	temp = (int)Data;
	return temp;
}



/**
**************************************************************************************************
* @Brief    Single byte data inversion        
* @Param    
*            @DesBuf: destination buffer
*            @SrcBuf: source buffer
* @RetVal    None
* @Note      (MSB)0101_0101 ---> 1010_1010(LSB)
**************************************************************************************************
*/
void InvertUint8(uint8_t *DesBuf, uint8_t *SrcBuf)
{
	int i;
	uint8_t temp = 0;

	for(i = 0; i < 8; i++)
	{
		if(SrcBuf[0] & (1 << i))
		{
			temp |= 1<<(7-i);
		}
	}
	DesBuf[0] = temp;
}
 
/**
**************************************************************************************************
* @Brief    double byte data inversion        
* @Param    
*            @DesBuf: destination buffer
*            @SrcBuf: source buffer
* @RetVal    None
* @Note      (MSB)0101_0101_1010_1010 ---> 0101_0101_1010_1010(LSB)
**************************************************************************************************
*/
void InvertUint16(uint16_t *DesBuf, uint16_t *SrcBuf)  
{  
	int i;  
	uint16_t temp = 0;    

	for(i = 0; i < 16; i++)  
	{  
		if(SrcBuf[0] & (1 << i))
		{          
			temp |= 1<<(15 - i);  
		}
	}  
	DesBuf[0] = temp;  
}

void InvertUint32(uint32_t *DesBuf, uint32_t *SrcBuf)  
{  
	int i;  
	uint16_t temp = 0;    

	for(i = 0; i < 32; i++)  
	{  
		if(SrcBuf[0] & (1 << i))
		{          
			temp |= 1<<(32 - i);  
		}
	}  
	DesBuf[0] = temp;  
}


