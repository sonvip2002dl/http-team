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
 * FILE NAME      :  M2_L7_T1_password_checker.c      
 * DESCRIPTION    :  This program compares password 
 *                   using plain text.
 * *******************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char password[] = "Keysight U3810A";
	char* input;
	int result;
	
	printf("Enter Password >");
	input = inputString(stdin, 20);
	
	result = strcmp(password, input);

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
