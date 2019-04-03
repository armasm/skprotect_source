x64 free protect

Features 
1.process/thread handle protect 
2.anti taskmgr.exe 
3.hide process 
4.anti-debugger(user/kernel debugger) 
 
how to use 
1. 
#include "sdk.h" 
2. 
Defining global variables 
CProtect app_protect; 
3. 
start protect : 
 
if(!app_protect.protect_process()) {  
  ExitProcess(0);  
}  
4.  
stop protect:  
app_protect.unprotect_process();  
  
5.  
build release\x64  
  
6.  
Copy file SKProDriver.sys to xxprotect_sdk\x64\Release  
  
Support platform:  
win7-win10 x64  
  
Reference  
https://bbs.pediy.com/thread-250404.htm  
https://bbs.pediy.com/thread-250523.htm  
https://bbs.pediy.com/thread-250468.htm  
