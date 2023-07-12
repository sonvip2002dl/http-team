/*******************************************************
 *         © Keysight Technologies 2019 
 *
 * PROPRIETARY RIGHTS of Keysight Technologies are 
 * involved in the subject matter of this software. 
 * All manufacturing, reproduction, use, and sales 
 * rights pertaining to this software are governed 
 * by the license agreement. The recipient of this 
 * code implicitly accepts the terms of the license. 
 *
 *********************************************************
 *
 * FILE NAME      :  M2_L7_T1_password_checker_md5.c      
 * DESCRIPTION    :  This program compares the password
 *                :  entered as plain text against the
 *				  :  md5 hashed phrase.
 * *******************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/md5.h>

char *inputString(FILE* fp, size_t size)
{
    //The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = (char*)realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)
        return str;
    
    while(EOF!=(ch=fgetc(fp)) && ch != '\n')
    {
        str[len++]=ch;
        if(len==size)
        {
            str = (char*)realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return (char*)realloc(str, sizeof(char)*len);
}

int main(void)
{
	char password[] = "03e3524785047613ba7cbee21f616910";
	char* input;
	unsigned char digest[MD5_DIGEST_LENGTH];
	int result;
    char mdString[33];

	printf("Enter Password >");
	input = inputString(stdin, 20);

	MD5((unsigned char*)input, strlen(input), (unsigned char*)&digest);
 
    	for(int i = 0; i < 16; i++)
		sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

	result = strcmp(password, mdString);

	if (result == 0)
	{
		printf("Password is correct.\n");
	}
	else
	{
		printf("Password is incorrect.\n");
	}

	free(input);
	return 0;
}
