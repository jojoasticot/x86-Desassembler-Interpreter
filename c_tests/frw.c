#include <stdio.h>
FILE *fp;
int inbuf[10];
main(argc, argv)
char **argv;
{
    int outbf[10]; 
    int i;
    for (i = 0; i < 10; ++i) {
        outbf[i] = i;
    }
    if((fp = fopen("output", "w+")) == NULL ) {
        printf("Error\n");
        exit(1);
    }
    fwrite(outbf, sizeof(int), 10, fp);
    fseek(fp, 0L, 0);
    fread(inbuf, sizeof(int), 10, fp);
    fclose(fp);
    return 0;
}
