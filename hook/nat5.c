/* code from http://www.tldp.org/HOWTO/Program-Library-HOWTO/more-examples.html 
   A few simple mods by richud.com to enable altering the Netgear ACOS_NAT firewall via the library 
   Loads the library and takes 5 args, the first is function name, next 4 are integer arguements. */
 
/* demo_dynamic.c -- demonstrate dynamic loading and
   use of the "hello" routine */
 
/* Need dlfcn.h for the routines to
   dynamically load libraries */
#include <dlfcn.h>
 
#include <stdlib.h>
#include <stdio.h>
 
/* Note that we don't have to include "libhello.h".
   However, we do need to specify something related;
   we need to specify a type that will hold the value
   we're going to get from dlsym(). */
 
/* The type "simple_demo_function" describes a function that
   takes no arguments, and returns no value: */
 
typedef void (*simple_demo_function)(int, int, int, int);
 
 
int main(int argc,char *argv[]) {
 const char *error;
 void *module;
 simple_demo_function demo_function;
 
 printf("Trying arg1 function %s\n",argv[1]);
 printf("Trying arg2 %s\n",argv[2]);
 printf("Trying arg3 %s\n",argv[3]);
 printf("Trying arg4 %s\n",argv[4]);
 printf("Trying arg5 %s\n",argv[5]);
 
 /* Load dynamically loaded library */
 module = dlopen("libnat.so", RTLD_LAZY);
 if (!module) {
   fprintf(stderr, "Couldn't open libnat.so: %s\n",
           dlerror());
   exit(1);
 }
 
 /* Get symbol */
 dlerror();
 demo_function = dlsym(module, argv[1]);
 if ((error = dlerror())) {
   fprintf(stderr, "Couldn't find %s : %s\n", argv[1],error);
   exit(1);
 }
 
 /* Now call the function in the DL library */
 (*demo_function)(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
 
 /* All done, close things cleanly */
 dlclose(module);
 return 0;
}

