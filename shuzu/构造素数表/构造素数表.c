
/**************************************
构造素数表
欲构造n以内（不含）的素数表
1.开辟prime[n],被初始化其所有元素为1，prime[x]为1表示x是素数
2.令x=2；
3.如果x是素数，则对于(i=2;x*i<n;i++)令prime[i*x]=0；
4.令x++，如果x<n,重复3.否则结束
***************************************/
#include<stdio.h>

void Prime(int n);

int main()
{
    int n;

    printf("输入n：");
    scanf("%d", &n);

    Prime(n);

}

void Prime(int n)
{
    int i,x;
    int prime[n];
    int num[n];
    int cnt=0;

    for (i=0; i<n; i++){
        prime[i]=1;
    }
    printf("原始素数表为：\n");
    for(i=0; i<n; i++){
        printf("%d ",i);
    }

    for (i=0; i<n; i++){
        printf("%d ",prime[i]);
    }
    printf("\n");
    prime[0]=0;prime[1]=0;prime[2]=1;

    for(x=3; x<n; x++){
	for(i=2; i<n && i!=x; i++){
            if(x%i==0){prime[x]=0;cnt++;}
	}
    }
    printf("素数表为：\n");
    for(i=0; i<n; i++){
        printf("%d ",i);
    }

    for(i=0; i<n; i++){
        printf("%d ",prime[i]);
    }
    printf("\n");
    printf("即%d以下的素数有%d个，分别为：\n",n,cnt);
    for(i=0; i<n; i++){
        if(prime[i]==1){printf("%d ", i);}
    }
    printf("\n");
}
