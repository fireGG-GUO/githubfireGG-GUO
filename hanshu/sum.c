//本题要求实现一个计算m~n（m<n）之间所有整数的和的简单函数。

#include <stdio.h>
int sum( int m, int n )
{
    int sum = 0;
    int i;
    if (m<n){
    	for (i=m;i<n+1;i++){
	    sum+=i;
	    //printf("sum=%d" ,sum);
	}
    }else{
	for (i=n;i<m+1;i++){
            sum+=i;
	    //printf("sum=%d" ,sum);
        }
    }
    
    return sum;
}
