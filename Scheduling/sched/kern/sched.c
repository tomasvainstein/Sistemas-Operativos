#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define MAX_ENVS 100000
#define MAX_TICKETS                                                            \
	11264  // NENVS es 1024 y los tickets por proceso son como maximo 11 (prioridad 10 + 1).
#define PRIORITY_BOOST_INTERVAL 20

static unsigned int rand_seed = 123;
static int ticks_since_last_boost = 0;

typedef struct sched_stats {
	int sched_calls;      // Number of times the scheduler was called
	int processes_execs;  // Number of processes executed
	int boosted_times;    // Number of times the priorities were boosted

	int executed_ids[MAX_ENVS];         // IDs of the executed processes
	int executed_priorities[MAX_ENVS];  // Priorities of the executed processes
} Statistics;

Statistics stats;

unsigned int rand();
void assign_tickets();
struct Env *select_winner();
void boost_priorities();
void decrease_priority(struct Env *env);
void set_statistics(struct Env *env);
void print_stats();
void sched_halt(void);

unsigned int
rand()
{
	const unsigned int RAND_MAX = 32767;
	const unsigned int RAND_A = 1103515245;
	const unsigned int RAND_C = 12345;

	rand_seed = (RAND_A * rand_seed + RAND_C) % RAND_MAX;
	return rand_seed;
}

void
assign_tickets()
{
	int total_tickets = 0;
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			envs[i].tickets = envs[i].priority + 1;
			total_tickets += envs[i].tickets;
		}
	}

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			envs[i].tickets =
			        envs[i].tickets * MAX_TICKETS / total_tickets;
		}
	}
}

struct Env *
select_winner()
{
	int winning_ticket = rand() % MAX_TICKETS;
	int accumulated_tickets = 0;

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			accumulated_tickets += envs[i].tickets;
			if (accumulated_tickets >= winning_ticket) {
				return &envs[i];
			}
		}
	}
	return NULL;
}

void
boost_priorities()
{
	stats.boosted_times++;
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE ||
		    envs[i].env_status == ENV_RUNNING) {
			envs[i].priority = 10;
		}
	}
}

void
decrease_priority(struct Env *env)
{
	if (env->priority > 0) {
		env->priority--;
	}
}

void
set_statistics(struct Env *env)
{
	stats.processes_execs++;
	stats.executed_ids[stats.processes_execs] = env->env_id;
	stats.executed_priorities[stats.processes_execs] = env->priority;
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN


	struct Env *currenv = curenv;
	int i = 0;
	if (currenv) {
		i = ENVX(curenv->env_id);  // Get the index of the current environment
	}

	for (int j = 0; j < NENV; j++) {
		i = (i + 1) % NENV;
		if (envs[i].env_status == ENV_RUNNABLE) {
			env_run(&envs[i]);
		}
	}

	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	if (currenv && currenv->env_status == ENV_RUNNING) {
		env_run(currenv);
	}


#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.
	struct Env *currenv = curenv;

	stats.sched_calls++;
	assign_tickets();

	ticks_since_last_boost++;
	if (ticks_since_last_boost >= PRIORITY_BOOST_INTERVAL) {
		boost_priorities(envs, NENV);
		ticks_since_last_boost = 0;
	}

	struct Env *winner = select_winner(envs, NENV);

	struct Env *env_to_run = NULL;
	if (winner != NULL) {
		env_to_run = winner;
	} else {
		if (currenv && currenv->env_status == ENV_RUNNING) {
			env_to_run = currenv;
		}
	}

	if (env_to_run) {
		decrease_priority(env_to_run);
		set_statistics(env_to_run);
		env_run(env_to_run);
	}

#endif

	// Without scheduler, keep runing the last environment while it exists
	// if (curenv) {
	// 	env_run(curenv);
	// }

	// sched_halt never returns
	sched_halt();
}

void
print_stats()
{
	cprintf("\n=== Scheduler Statistics\n");
	cprintf("Sched was called %d times\n", stats.sched_calls);
	cprintf("Sched executed processes %d times\n", stats.processes_execs);

	cprintf("Processes executed: ");
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_id != 0) {
			cprintf("%d / ", envs[i].env_id);
		}
	}
	cprintf("\n");

	cprintf("Boosted priorities %d times\n\n", stats.boosted_times);

	cprintf("Historical Executed IDs:\n");
	for (int i = 0; i < NENV; i++) {
		if (stats.executed_ids[i] != 0) {
			cprintf("[%d] %d -> priority: %d\n",
			        i,
			        stats.executed_ids[i],
			        stats.executed_priorities[i]);
		}
	}
	cprintf("\n");
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		print_stats();
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
