#include <stdio.h>
#include "list.h"

int main()
{
	list_t *list = list_open(NULL);
	list_close(list);
}
