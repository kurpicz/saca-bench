#ifndef RANDOMLIB_H
#define RANDOMLIB_H

void   RandomInitialise(int,int);
double RandomUniform(void);
double RandomGaussian(double,double);
int    RandomInt(int,int);
double RandomDouble(double,double);

void kbsRandomize();


#endif
