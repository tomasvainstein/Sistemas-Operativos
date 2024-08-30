#include <inc/lib.h>
int hola = 0;

void
umain(int argc, char **argv)
{
	for (int i = 0; i < 2; i++) {
		volatile unsigned long long i;
		for (i = 0; i < 10000000ULL; i++) {
		}

		int id = sys_getenvid();
		sys_env_get_priority(id);
		cprintf("Priority of env %x is %d\n", id, sys_env_get_priority(id));

		sys_env_set_priority(sys_getenvid(), 0);
		cprintf("New priority of env %x is %d\n",
		        id,
		        sys_env_get_priority(id));
	}
	return;
}
