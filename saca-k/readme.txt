-----------------------------------------------------------------------------------------------------------
File Specification

main.cpp
A sample program for calling SACA-K to comput the suffix array of an input file.

saca-k.cpp
A sample embodiment of the SACA-K algorithm presented in this article: 
G. Nong, Practical Linear-Time O(1)-Workspace Suffix Sorting for Constant Alphabets, 
ACM Transactions on Information Systems, Scheduled to Appear in July 2013.
A draft for this article can be retrieved from http://code.google.com/p/ge-nong/.

-----------------------------------------------------------------------------------------------------------
How-To

Compile
On Linux, use the command "make" to produce the executable file "saca-k".

Run
"saca-k readme.txt rel" will comput the suffix array of file "readme.txt" into file "rel",
where each byte of file "readme.txt" is taken as a character.
-----------------------------------------------------------------------------------------------------------

Thanks for dropping us a message if it is useful for you.

Enjoy!

Ge Nong
Department of Computer Science
Sun Yat-sen University 
Guangzhou, China
Email: issng@mail.sysu.edu.cn

Last Updated on April 13, 2013