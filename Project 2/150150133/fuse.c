/*
 * Bengisu Guresti	150150105
 * Ufuk Demir		150170710
 * Abdullah Akgul	150150133
 * gcc fuse.c -o fuse -Wall -ansi -W -std=c99 -g -ggdb -D_GNU_SCORE -D_FILE_OFFSET_BITS=64 -lfuse -lmagic -lgd -Wextra -pedantic -lansilove -D_BSD_SOURCE
 */

#define FUSE_USE_VERSION 26
#define _POSIX_C_SOURCE 199309

#include <png.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ansilove.h>
#include <magic.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/xattr.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gd.h>

char *rw_path;
int global_len;

static char* translate_path(const char* path)
{
    char *rPath= malloc(sizeof(char)*(strlen(path)+strlen(rw_path) + 1));
    strcpy(rPath,rw_path);
    if (rPath[strlen(rPath)-1]=='/') {
        rPath[strlen(rPath)-1]='\0';
    }
    strcat(rPath,path);
	return rPath;
}

static char* convert_text_to_PNG(char *path){
	struct ansilove_ctx ctx;
	struct ansilove_options options;
    ansilove_init(&ctx, &options);
	ansilove_loadfile(&ctx, path);
	ansilove_ansi(&ctx, &options);
	int len = ctx.png.length; 
	char *to_return = malloc(len);
	memset(to_return, 0, len);
	memcpy(to_return, ctx.png.buffer, ctx.png.length);
	ansilove_clean(&ctx);
	global_len = len;
	return to_return;
}

static int file_type_checker(const char *file_name){
	const char *magic_full;
    magic_t magic_cookie;
    magic_cookie = magic_open(MAGIC_MIME_TYPE);
    if (magic_cookie == NULL) {
        printf("unable to initialize magic library\n");
        return 1;
    }
    if (magic_load(magic_cookie, NULL) != 0) {
        printf("cannot load magic database - %s\n", magic_error(magic_cookie));
        magic_close(magic_cookie);
        return 1;
    }
    magic_full = magic_file(magic_cookie, file_name);
    if(magic_full == NULL){
		magic_close(magic_cookie);
		return 0;
	}
	char deneme[strlen(magic_full)];
	strcpy(deneme, magic_full);
    char* token = strtok(deneme, "/"); 
    
	char* left = token;
  	token = strtok(NULL, "/"); 
	char* right = token;
    if (strcmp(left, "text") == 0){
		magic_close(magic_cookie);
		return 1;
	}
	else if(strcmp(left, "application") == 0){
		if (strcmp(right, "octet-stream") == 0){
			magic_close(magic_cookie);
			return 1;
		}
		else {
			magic_close(magic_cookie);
			return 0;
		}
	}
	else{
		magic_close(magic_cookie);
		return 0;
	}
}


static char* get_original_file_path(char *in)
{
	int pos_slash = 0, pos_dot = 0, i = 0;
	for(i = strlen(in) -1; i >= 0;  i--){
		if(in[i] == '.'){
			pos_dot = i;
		}
		if(in[i] == '/'){
			pos_slash = i;
			break;
		}
	}
	if (pos_dot - pos_slash == 1) return NULL; 
	pos_slash++;
	char *temp_dir = malloc(sizeof(char)*(pos_slash + 1));
	memset(temp_dir, 0, sizeof(char)*(pos_slash + 1));
	strncpy(temp_dir, in, pos_slash);
	
	char *file_name = malloc(sizeof(char)*(pos_dot + 1));
	memset(file_name, 0, sizeof(char)*(pos_dot + 1));
	strncpy(file_name, (in + pos_slash), pos_dot);
	
	DIR *dp;
    dp = opendir(temp_dir);
    struct dirent *de;
    
    if(dp == NULL) {
		free(file_name);
		free(temp_dir);
        return NULL;
    }
    int flag = 0;
    while((de = readdir(dp)) != NULL) {
	   if (strncmp(de->d_name, file_name, pos_dot - pos_slash) == 0){
			flag ++;
			break;
		}
    }
    if(flag == 0){
		
		free(file_name);
		free(temp_dir);
		return NULL;
	}
	closedir(dp);
	free(file_name);
	char *to_return = malloc(sizeof(char)*(strlen(temp_dir) + strlen(de->d_name) + 1));
	memset(to_return, 0, sizeof(char)*(strlen(temp_dir) + strlen(de->d_name) + 1));
	strcpy(to_return, temp_dir);
	strcat(to_return, de->d_name);
	free(temp_dir);
	return to_return;
}

static int dot_checker(const char *path){
	
	int i = 0, length = strlen(path);
    for (; i < length; i ++){
		if (path[i] == '.') return 1;
	}
	return 0;
    
}

static char* change_extention(char *file_name)
{
    int pos = 0, i = 0;
	for(i = strlen(file_name) -1; i >= 0;  i--){
		if(file_name[i] == '.'){
			pos = i;
			break;
		}
	}
	if (pos - 1 == 0) pos = strlen(file_name);
	char *exten = ".png";
	char *temp = malloc(sizeof(char)*(pos + strlen(exten)) + 1);
	memset(temp,'\0', sizeof(char)*(pos + strlen(exten)) + 1);
	strncpy(temp, file_name, pos);
	strcat (temp,exten);
    return temp;
}

static int love_png_getattr(const char *path, struct stat *st_data)
{
    int res;
    char *upath=translate_path(path);
	res = lstat(upath, st_data);
	
	if (S_ISDIR(st_data->st_mode)){
		// res = lstat(upath, st_data);
		st_data->st_mode = S_IFDIR | 0444;
	}
	else{
		if (strcmp(path, "/autorun.inf") == 0 || strcmp(path, "/.directory") == 0){
			// res = lstat(upath, st_data);
		}
		else{
		
			char * temp = get_original_file_path(upath);
			if (temp == NULL){
				// res = lstat(upath, st_data);
			}
			else{
				char *trash = convert_text_to_PNG(temp);
				res = lstat(temp, st_data);
				st_data->st_size = (size_t) global_len;
				st_data->st_mode = S_IFREG | 0444;
				free(temp);
				free(trash);
			}
		
			
		}
		
	}
	free(upath);
    if (res == -1){
		return -errno;
	}
    return res;
}

static int love_png_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
	
    DIR *dp;
    struct dirent *de;
    int res;

    (void) offset;
	(void) fi;
    char *upath=translate_path(path);

    dp = opendir(upath);
    
    if(dp == NULL) {
        res = -errno;
        return res;
    }
    
    while((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0){
			if (filler(buf, de->d_name, &st, 0))
				break;
		}
		else if(de->d_type == DT_DIR){
			if (filler(buf, de->d_name, &st, 0)){
				break;
			}
		}
        else{
			
			char * concat;
			if (strcmp(upath, "src/") == 0){
				concat = malloc(sizeof(char)*(strlen(upath)+strlen(de->d_name)+1));
				memset(concat, 0, strlen(upath)+strlen(de->d_name)+1);
				strcpy(concat, upath);
				strcat(concat, de->d_name);
			}
			else{
				concat = malloc(sizeof(char)*(strlen(upath)+strlen(de->d_name)+2));
				memset(concat, 0, strlen(upath)+strlen(de->d_name)+2);
				strcpy(concat, upath);
				strcat(concat, "/");
				strcat(concat, de->d_name);
			}
			
			if(file_type_checker(concat) == 1){
				
				char *temp;
				if(dot_checker(de->d_name)== 0){
					char *temp_name = malloc(sizeof(char)*strlen(de->d_name) + 2);
					strcpy(temp_name, de->d_name);
					temp_name[strlen(de->d_name)] = '.';
					temp_name[strlen(de->d_name) + 1] = '\0';
					temp = change_extention(temp_name);
				}
				else temp = change_extention(de->d_name);
				if (filler(buf, temp, &st, 0)){
					free(temp);
					free(concat);
					break;
				}
				free(temp);
			}
			free(concat);
			
            
		}
    }
    
	free(upath);
    closedir(dp);
    return 0;
}

static int love_png_open(const char *path, struct fuse_file_info *finfo)
{
	
    int res;
    int flags = finfo->flags;

    if ((flags & O_WRONLY) || (flags & O_RDWR) || (flags & O_CREAT) || (flags & O_EXCL) || (flags & O_TRUNC) || (flags & O_APPEND)) {
        return -EROFS;
    }

    char *upath=translate_path(path);
	char *temp = get_original_file_path(upath);
    res = open(temp, flags);

    free(upath);
    free(temp);
    if(res == -1) {
        return -errno;
    }
    close(res);
    return 0;
}

static int love_png_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *finfo)
{   
    int fd;
    size_t res;
    (void)finfo;
	char *upath=translate_path(path);
	char *org = get_original_file_path(upath);
	if (org == NULL)
		return -errno;
	fd = open(org, O_RDONLY);
    
	if(fd == -1) {
		res = -errno;
		free(upath);
        return res;
    }
    

	char * temp = convert_text_to_PNG(org);
	size_t len = (size_t) global_len;
	if (offset < len) {
        if (offset + size > len)
            res = len - offset;
        else
			res = size;
        memcpy(buf, temp + offset, res);
    } 
    else{
        res = 0;
    }
	free(temp);
	free(org);
    free(upath);
    if((int)res == -1) {
        res = -errno;
    }
    close(fd);
    return res;
}

static int love_png_release(const char *path, struct fuse_file_info *finfo)
{
    (void) path;
    (void) finfo;
    return 0;
}


static int love_png_mknod(const char *path, mode_t mode, dev_t rdev)
{
    (void)path;
    (void)mode;
    (void)rdev;
    return -EROFS;
}

static int love_png_mkdir(const char *path, mode_t mode)
{
    (void)path;
    (void)mode;
    return -EROFS;
}

static int love_png_unlink(const char *path)
{
    (void)path;
    return -EROFS;
}

static int love_png_rmdir(const char *path)
{
    (void)path;
    return -EROFS;
}

static int love_png_symlink(const char *from, const char *to)
{
    (void)from;
    (void)to;
    return -EROFS;
}

static int love_png_rename(const char *from, const char *to)
{
    (void)from;
    (void)to;
    return -EROFS;
}

static int love_png_link(const char *from, const char *to)
{
    (void)from;
    (void)to;
    return -EROFS;
}

static int love_png_chmod(const char *path, mode_t mode)
{
    (void)path;
    (void)mode;
    return -EROFS;

}

static int love_png_chown(const char *path, uid_t uid, gid_t gid)
{
    (void)path;
    (void)uid;
    (void)gid;
    return -EROFS;
}

static int love_png_truncate(const char *path, off_t size)
{
    (void)path;
    (void)size;
    return -EROFS;
}

static int love_png_utime(const char *path, struct utimbuf *buf)
{
    (void)path;
    (void)buf;
    return -EROFS;
}

static int love_png_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *finfo)
{
    (void)path;
    (void)buf;
    (void)size;
    (void)offset;
    (void)finfo;
    return -EROFS;
}

static int love_png_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    (void)path;
    (void)name;
    (void)value;
    (void)size;
    (void)flags;
    return -EROFS;
}

static int love_png_removexattr(const char *path, const char *name)
{
    (void)path;
    (void)name;
    return -EROFS;

}


struct fuse_operations love_png_oper = {
    .getattr     = love_png_getattr,
    .readdir     = love_png_readdir,
    .mknod       = love_png_mknod,
    .mkdir       = love_png_mkdir,
    .symlink     = love_png_symlink,
    .unlink      = love_png_unlink,
    .rmdir       = love_png_rmdir,
    .rename      = love_png_rename,
    .link        = love_png_link,
    .chmod       = love_png_chmod,
    .chown       = love_png_chown,
    .truncate    = love_png_truncate,
    .utime       = love_png_utime,
    .open        = love_png_open,
    .read        = love_png_read,
    .write       = love_png_write,
    .release     = love_png_release,
    .setxattr    = love_png_setxattr,
    .removexattr = love_png_removexattr
    
};

static int love_png_parse_opt(void *data, const char *arg, int key,
                          struct fuse_args *outargs)
{
	
    (void)data;
    (void)outargs;
    if(key == FUSE_OPT_KEY_NONOPT)
    {
    	if (rw_path == 0)
        {
            rw_path = strdup(arg);
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if(key == FUSE_OPT_KEY_OPT)
    	return 1;
    else
    	exit(1);
}

static struct fuse_opt love_png_opts[] = {
    FUSE_OPT_END
};


int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int res;

    res = fuse_opt_parse(&args, &rw_path, love_png_opts, love_png_parse_opt);
    if (res != 0)
    {
        fprintf(stderr, "Invalid arguments\n");
        fprintf(stderr, "see `%s -h' for usage\n", argv[0]);
        exit(1);
    }
    if (rw_path == 0)
    {
        fprintf(stderr, "Missing readwritepath\n");
        fprintf(stderr, "see `%s -h' for usage\n", argv[0]);
        exit(1);
    }
    fuse_main(args.argc, args.argv, &love_png_oper, NULL);
    return 0;
}
