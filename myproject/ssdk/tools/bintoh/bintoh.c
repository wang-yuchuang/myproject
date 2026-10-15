
/*
* convert binary to .h
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
typedef unsigned char u8;
typedef unsigned int  u32;
void read_bin(char *path, u8 *buf, u32 size)
{
    FILE *infile;

    if ((infile = fopen(path, "rb")) == NULL) {
        printf("\nCan not open the path: %s \n", path);
        exit(-1);
    }

    fread(buf, sizeof(u8), size, infile);
    fclose(infile);
}
u32 GetBinSize(char *filename)
{
    u32  siz = 0;
    FILE  *fp = fopen(filename, "rb");

    if (fp) {
        fseek(fp, 0, SEEK_END);
        siz = ftell(fp);
        fclose(fp);
    }

    return siz;
}
void OutPut_handle(char *outpath, u8 *buf, u32 size)
{
    FILE *infile;
    int i, j, k, n;
    int fd ;
    char pbuf[10] = {0};
    char *array = "static const unsigned char ddr_lowpower_image[] = {\n";
    char *Handle = "#ifndef DDR_LOW_POWER_H\n";
    char *Handle1 = "#define DDR_LOW_POWER_H\n";
    char *line_T       = "\n";
    char *EndIF        = "\n#endif \n";

    if ((infile = fopen(outpath, "wa+")) == NULL) {
        printf("\nCan not open the path: %s \n", outpath);
        exit(-1);
    }

    k = 0;
    fwrite(Handle, strlen(Handle), 1, infile);
    fwrite(Handle1, strlen(Handle1), 1, infile);
    fwrite(array, strlen(array), 1, infile);
    printf("size=%d\n", size);

    for (i = 0; i < size; i++) {
        k++;
        sprintf(pbuf, "0x%02x", buf[i]);
        fwrite(pbuf, strlen(pbuf), 1, infile);

        if (k != 16)
            fwrite(", ", strlen(", "), 1, infile);
        else
            fwrite(",", strlen(","), 1, infile);

        if (k == 16) {
            k = 0;
            fwrite("\n", strlen("\n"), 1, infile);
        }
    }

    fseek(infile, 0, SEEK_END);

    if (k == 0) {
        fwrite("};", strlen("};"), 1, infile);
    }
    else
        fwrite("\n};", strlen("\n};"), 1, infile);

    //在infile文件中和换行
    fwrite(line_T, strlen(line_T), 1, infile);
    fwrite(EndIF, strlen(EndIF), 1, infile);
    fclose(infile);
    return;
}

int main(int argc, void **argv)
{
    u8 *buf = NULL;
    u32 size;
    char srcbin[100] = {0};
    char dstfile[100] = {0};
    //读取目标.bin文件
    //printf("please input src file path\n");
    //scanf("%s",srcbin);
    strcpy(srcbin, argv[1]);
    //创建一个.h头文件用于保存bin转C数组的文件
    //printf("please input output path\n");
    //scanf("%s",dstfile);
    strcpy(dstfile, argv[2]);
    //获取文件的大小
    size = GetBinSize(srcbin);
    //申请用于存放该文件的数组
    buf = (unsigned char *)malloc(sizeof(unsigned char) * size);
    //读取文件
    read_bin(srcbin, buf, size);
    //制作头文件，该头文件下含有两个数组，一个是有数据的，另外一个是全0数组
    //全0主要备用，以后要清空可以调用这个数组
    OutPut_handle(dstfile, buf, size);
    return 0;
}
