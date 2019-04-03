x64 free protect<br>  
<br>  
Features <br>  
1.process/thread handle protect <br>  
2.anti taskmgr.exe <br>  
3.hide process <br>  
4.anti-debugger(user/kernel debugger) <br>  
 <br>  
how to use <br>  
1. <br>  
#include "sdk.h" <br>  
2. <br>  
Defining global variables <br>  
CProtect app_protect; <br>  
3. <br>  
start protect : <br>  
 <br>  
if(!app_protect.protect_process()) {  <br>  
  ExitProcess(0);  <br>  
}  <br>  
4.  <br>  
stop protect:  <br>  
app_protect.unprotect_process(); <br>   
  <br>  
5.  <br>  
build release\x64  <br>  
  <br>  
6.  <br>  
Copy file SKProDriver.sys to xxprotect_sdk\x64\Release  <br>  
  <br>  
Support platform:  <br>  
win7-win10 x64  <br>  
  <br>  
Reference  <br>  
https://bbs.pediy.com/thread-250404.htm  <br>  
https://bbs.pediy.com/thread-250523.htm  <br>  
https://bbs.pediy.com/thread-250468.htm  <br>  
