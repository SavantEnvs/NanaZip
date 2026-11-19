#include "lfs.h"
#include <stdio.h>
#include <string.h>
#define BS 512
#define BC 32
static unsigned char img[BS*BC];
static int rd(const struct lfs_config*c,lfs_block_t b,lfs_off_t o,void*buf,lfs_size_t s){memcpy(buf,img+b*BS+o,s);return 0;}
static int pg(const struct lfs_config*c,lfs_block_t b,lfs_off_t o,const void*buf,lfs_size_t s){memcpy(img+b*BS+o,buf,s);return 0;}
static int er(const struct lfs_config*c,lfs_block_t b){memset(img+b*BS,0xff,BS);return 0;}
static int sy(const struct lfs_config*c){return 0;}
int main(int argc,char**argv){
  struct lfs_config cfg={0}; cfg.read=rd;cfg.prog=pg;cfg.erase=er;cfg.sync=sy;cfg.read_size=16;cfg.prog_size=16;cfg.block_size=BS;cfg.block_count=BC;cfg.cache_size=64;cfg.lookahead_size=16;cfg.block_cycles=500;
  lfs_t l; memset(img,0xff,sizeof img); if(lfs_format(&l,&cfg)||lfs_mount(&l,&cfg)) return 1;
  lfs_mkdir(&l,"dir"); lfs_file_t f;
  lfs_file_open(&l,&f,"hello.txt",LFS_O_WRONLY|LFS_O_CREAT); lfs_file_write(&l,&f,"Hello, littlefs!\n",17); lfs_file_close(&l,&f);
  lfs_file_open(&l,&f,"dir/big.bin",LFS_O_WRONLY|LFS_O_CREAT); for(int i=0;i<1500;i++){unsigned char c=(unsigned char)(i*7);lfs_file_write(&l,&f,&c,1);} lfs_file_close(&l,&f);
  lfs_unmount(&l); FILE*o=fopen(argv[1],"wb"); fwrite(img,1,sizeof img,o); fclose(o); return 0;}
