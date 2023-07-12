#include <stdio.h>

int main(int argc, char** argv)
{

FILE *fp;

fp = fopen("/mnt/weatherinfo.txt", "a+");
fprintf(fp, "%s\n", argv[1]);

fclose(fp);

}