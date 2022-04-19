//本函数要求实现函数输出n行数字金字塔。
#include <stdio.h>

void pyramid(int n)
{
    int i,j,k;
    for(i=1;i<n+1;i++)
    {
        for(k=n-i;k>0;k--)
        {
            printf(" ");
        }
        for(j=0;j<i;j++){
            printf("%d ", i);
        }
        printf("\n");
    }
    
}

