#include <stdio.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>

/*
 * $(CC) -fPIC -shared -o hook.so hook.c -ldl
 * arm-brcm-linux-uclibcgnueabi-gcc -fPIC -shared -o hook.so hook.c -ldl
 *  */
typedef int (*LISTEN)(int sockfd, int backlog);
typedef int (*BIND)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
typedef char* (*NVRAM_GET)(char* in);
typedef int (*MTD_ERASE)(const char * in);
typedef int (*MOUNT)(const char *, const char *, const char *, int, int);

static void* get_libc_handle() {
    static void *handle = NULL;
    if(!handle) {
        handle = dlopen("libc.so.0", RTLD_LAZY);
    }

    return handle;
}

static void* get_nvram_handle() {
    static void *handle = NULL;
    if(!handle) {
        handle = dlopen("libnvram.so", RTLD_LAZY);
    }

    return handle;
}

static void* get_acos_handle() {
    static void *handle = NULL;
    if(!handle) {
        handle = dlopen("libacos_shared.so", RTLD_LAZY);
    }

    return handle;
}

int listen(int sockfd, int backlog) {
    static LISTEN old_listen = NULL;
    void* handle = get_libc_handle();

    if(!old_listen) {
        old_listen = (LISTEN)dlsym(handle, "listen");
    }
    //printf("hack function invoked. s1=<%d> s2=<%d>\n", sockfd, backlog);
    return old_listen(sockfd, backlog);
}

/* char* nvram_safe_get(char* in) {
    void* handle = get_nvram_handle();
    static NVRAM_GET old_handle = NULL;

    if(!old_handle) {
        old_handle = (NVRAM_GET)dlsym(handle, "nvram_safe_get");
    }
    if(NULL == old_handle) {
        printf("null\n");
        return NULL;
    }
    return old_handle(in);
} */

char* nvram_get(char* in) {
    void* handle = get_nvram_handle();
    static NVRAM_GET old_handle = NULL;

    if(!old_handle) {
        old_handle = (NVRAM_GET)dlsym(handle, "nvram_get");
    }
    //printf("hack nvram_get function invoked. in=%s\n", in);
    return old_handle(in);
}

static int _atoi(char *str) {
    //printf("hear2\n");

    if (NULL == str || *str == '\0') {
       return 0;
    }

    int res = 0;	 // Initialize result
    int sign = 1;	 // Initialize sign as positive
    int i = 0;	 // Initialize index of first digit

    if (str[0] == '-') {
        sign = -1;
        i++;  	 // Also update index of first digit
    }
    for (; str[i] != '\0'; ++i)
    {
        if ( str[i] <= '0' || str[i] >= '9')	// If string contain character it will terminate
            return 0;
            
        res = res*10 + str[i] - '0';
    }
    return sign*res;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
#define BUF_MAX (64)
    static BIND old_handle = NULL;
    char buf[BUF_MAX];
    struct  sockaddr_in* in_addr = (struct  sockaddr_in* )addr;
    void* handle = get_libc_handle();
    char * remote_port = nvram_get("http_rmport");
    //printf("hear\n");
    //printf("rmport=%s\n", remote_port);
    unsigned short rmport = (unsigned short)(_atoi(remote_port) & 0xFFFF);
    //printf("rmport=%d\n", rmport);

    if(!old_handle) {
        old_handle = (BIND)dlsym(handle, "bind");
    }

    if(addrlen < BUF_MAX) {
        if(htons(80) == in_addr->sin_port) {
            in_addr = (struct sockaddr_in*)buf;
            memcpy((void*)in_addr, (void*)addr, addrlen);
            in_addr->sin_port = htons(8880);
        } else if(htons(rmport) == in_addr->sin_port) {
            in_addr = (struct sockaddr_in*)buf;
            memcpy((void*)in_addr, (void*)addr, addrlen);
            in_addr->sin_port = htons(8883);
        }
    }

    //printf("hack bind function invoked. port=%d addr=0x%02x addrlen=%d\n", htons(in_addr->sin_port), in_addr->sin_addr.s_addr, addrlen);
    return old_handle(sockfd, (const struct sockaddr*)in_addr, addrlen);
}

static int _strcmp(const char *s1,  const char *s2) {
    uint8_t *p1 = (uint8_t*)s1;
    uint8_t *p2 = (uint8_t*)s2;

    if(NULL == s1 || NULL == s2) {
        return -2;
    }

    while(*p1 != '\0') {
        if(*p2 == '\0') {
            return 1;
        }
        if(*p2 > *p1) {
            return -1;
        }
        if(*p1 > *p2) {
            return 1;
        }

        p1++;
        p2++;
    }

    if(*p2 != '\0') {
        return -1;
    }

    return 0;
}

int mtd_erase(const char * in) {
    static MTD_ERASE old_handle = NULL;
    void* handle = get_acos_handle();
    if(!old_handle) {
        old_handle = (MTD_ERASE)dlsym(handle, "mtd_erase");
    }

    if(0 == _strcmp(in, "brcmnand")) {
        //Ignore erase brcmnand
        return 0;
    }

    //printf("mtd_erase called, in=%s\n", in);
    return old_handle(in);
}

int mount(const char *dpath, const char *spath, const char *t, int a1, int a2) {
    static MOUNT old_handle = NULL;
    static is_mount = 0;
    void* handle = get_libc_handle();

    if(!old_handle) {
        old_handle = (MOUNT)dlsym(handle, "mount");
    }

    if(0 == _strcmp(spath, "/tmp/media/nand")) {
        if(is_mount) {
            //ignore it. mount already
            return 0;
        }

        is_mount = 1;
    }

    //printf("mount called %s %s %s\n", dpath, spath, t); 
    return old_handle(dpath, spath, t, a1, a2);
}

