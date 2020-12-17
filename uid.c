#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
mode_t vaild_acess(char *path,int uid,int gid)
{
  struct stat ret;
  stat(path,&ret);

  if(uid == ret.st_uid)   //
  {
      return 
      (ret.st_mode & S_IRUSR) |
      (ret.st_mode & S_IWUSR) |
      (ret.st_mode & S_IXUSR); 
  }
  else if(gid == ret.st_gid)
  {
      return
      (ret.st_mode & S_IRGRP) |
      (ret.st_mode & S_IWGRP) |
      (ret.st_mode & S_IXGRP); 
  }
  return 
  (ret.st_mode & S_IROTH) |
  (ret.st_mode & S_IWOTH) |
  (ret.st_mode & S_IXOTH);
}

int readAble(mode_t t)
{
  return (t & S_IRUSR) | (t & S_IRGRP) | (t & S_IROTH) ; 
}

int writeAble(mode_t t)
{
  return (t & S_IWUSR) | (t & S_IWGRP) | (t & S_IWOTH);
}

int main(int argc, char *argv[])
{
    char *path = "save.txt";
    int t = vaild_acess(path,getuid(),getgid());
    printf("%d",writeAble(t));
}

