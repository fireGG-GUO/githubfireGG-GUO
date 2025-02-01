#include <stdio.h>
#include <string.h>

int main(void)
{
	char s1[100];
    char *s2 = "welcom";
    strncpy(s1, s2, strlen(s2));
    printf("%d\n", strlen(s2));
    printf("%s\n", s1);    
    return 0;
}
