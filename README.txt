Instruction:

1. Unzip all the files in a3.zip
2. make router
3. In hostX 
	./nse-linux386 hostY 12180
4. In hostY
	./router K hostX 12180 1218K (For K = 1..5)
5. All the logs will be in the routerK.log file. (For K == 1..5)
6. make clean

************************************************

I used ubuntu1604-002 to run nse, ubuntu1604-006 to run routers, 
and use ubuntu1604-008 to check log files. 
The program works perfectly on undergrad student environment.

************************************************

make --version
GNU Make 4.1
Built for x86_64-pc-linux-gnu
Copyright (C) 1988-2014 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

g++ --version
g++ (Ubuntu 5.4.1-2ubuntu1~16.04) 5.4.1 20160904
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************

Notes:

For the on-log incomplete RIB, "R-1" stands for unreachable.
"9999999" stands for infinitely far.

