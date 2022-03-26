#include <stdio.h>

int main(){
    int number[100];
    int cnt = 0;
    int sum = 0;
    int x;
    int i = 0;
    double avr = 0;

    //读入用户输入的值
    scanf("%d", &x);
    while(x!=-1){
	sum += x;
	number[cnt] = x;    
        scanf("%d",&x);
	cnt++;

    }
    avr = sum*1.0/cnt;
    printf("%f\n", avr);
    
    //while(number[i] > avr)
    //{
	//printf("%d\n", number[i]);
	//i++;
    //}
    for (i = 0;i<cnt;i++)
    {
	if(number[i] > avr){
	    printf("%d\n",number[i]);
	}
    }



}
