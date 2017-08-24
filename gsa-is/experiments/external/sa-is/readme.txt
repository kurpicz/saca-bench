-----------------------------------------------------------------------------------------------------------
File Specification

main.cpp
A sample program for calling SAIS to comput the suffix array of an input file.

sais.cpp
A sample embodiment of the SA-IS algorithm presented in our paper
"Two Efficient Algorithms for Linear Time Suffix Array Construction",
which is to appear in IEEE Transactions on Computers (preprint is available 
in the  IEEE/IET Electronic Library now) and the draft can be retrieved
from http://www.cs.sysu.edu.cn/nong/.
Different from that presented in our paper, this embodiment requires no the unique
smallest sentinel in the input string, instead, a virtual sentinel is added and 
handled in the program when it is in function.
-----------------------------------------------------------------------------------------------------------
How-To

Compile
On Linux, use the command "make" to produce the executable file "sais".

Run
"sais readme.txt rel" will comput the suffix array of file "readme.txt" into file "rel",
where each byte of file "readme.txt" is taken as a character.
-----------------------------------------------------------------------------------------------------------

Thanks for dropping us a message if it is useful for you.

Enjoy!

Ge Nong
Department of Computer Science
Sun Yat-sen University 
Guangzhou, China
Email: issng@mail.sysu.edu.cn

Last Updated on May 14, 2011