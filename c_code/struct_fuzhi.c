#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define N 50

struct Student {
	int no;
	char *name;
};


int main ()
{
	struct Student s1 = {1, "zhangsan"}; // 静态存储区无法修改
	struct Student s2;
    // s2. name = "lisi"; s2.name[0] = 'L'; // 段错误
	// strcpy(s2.name, s1.name); // 段错误，s2.name 未分配存储空间为野指针，徐使用malloc
	s2.name = malloc(N * sizeof(char) + 1);
	memset(s2.name, 0, sizeof(s2.name));
	strcpy(s2.name, "lisi");
	s2.name[0] = 'L';
	s2.name[4] = 'e';
	s2.name[5] = 'n';
	s2.name[6] = 'd';
	printf("%d, %s \n",s1.no, s1.name );
	printf("%d, %s \n",s2.no, s2.name );
	free(s2.name);
	return 0;
}
