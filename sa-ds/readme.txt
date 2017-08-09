-----------------------------------------------------------------------------------------------------------
File Specification

is.cpp
A sample program for calling SA_DS to comput the suffix array of an input file.


core.cpp
A sample embodiment of the SA-DS algorithm presented in our paper:
G. Nong, S. Zhang and W. H. Chan, Two Efficient Algorithms for Linear Time 
Suffix Array Construction, IEEE Transactions on Computers, Vol. 60, No. 10, Oct. 2011.

A draft for this article is available at:
http://code.google.com/p/ge-nong/

Different from that presented in our paper, this embodiment requires no the unique
smallest sentinel in the input string, instead, a virtual sentinel is added and 
handled in the program when it is in function.
-----------------------------------------------------------------------------------------------------------
How-To

Compile
On Linux, use the command "make" to produce the executable file "is".

Run
"is readme.txt rel" will comput the suffix array of file "readme.txt" into file "rel",
where each byte of file "readme.txt" is taken as a character.
-----------------------------------------------------------------------------------------------------------

Thanks for dropping us a message if it is useful for you.

Enjoy!

Ge Nong
Professor
Department of Computer Science
Sun Yat-sen University 
Guangzhou, P.R.C.
Email: issng@mail.sysu.edu.cn

Last Updated on Jan. 07, 2012