#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>

#include "stm32f103.h"
#include "printf.h"
#include "tasklet.h"

struct tasklet *tasklet_scheduled_head = NULL;
struct tasklet *tasklet_info_head = NULL;

void tasklet_register(struct tasklet *tasklet, const char *name, tasklet_run_fn *run)
{
	if (tasklet->state & TASKLET_REGISTERED) {
		printf("%x already registered\n", (int)tasklet);
		return;
	}
	if (tasklet->cnt != 0) {
		printf("%x cnt not 0\n", (int)tasklet);
		return;
	}
	tasklet->run = run;
	tasklet->name = name;
	tasklet->state = TASKLET_REGISTERED;
	tasklet->next_info = tasklet_info_head;
	tasklet->next_scheduled = NULL;
	tasklet_info_head = tasklet;
}

static void tasklet_schedule_unlocked(struct tasklet *tasklet)
{
	if (tasklet->state & TASKLET_SCHEDULED)
		return;

	tasklet->next_scheduled = tasklet_scheduled_head;
	tasklet_scheduled_head = tasklet;
	tasklet->state |= TASKLET_SCHEDULED;
}

void tasklet_schedule(struct tasklet *tasklet)
{
	uint32_t saved;
	if (!(tasklet->state & TASKLET_REGISTERED)) {
		printf("tasklet_schedule: %x (%s) not registered\n", (int)tasklet, tasklet->name);
		return;
	}
	__disable_irq_save(&saved);
	tasklet_schedule_unlocked(tasklet);
	__restore_irq_save(saved);
}

static void tasklet_run_next_scheduled(void)
{
	uint32_t saved;
	struct tasklet *pend;
	__disable_irq_save(&saved);
	pend = tasklet_scheduled_head;
	tasklet_scheduled_head = pend->next_scheduled;
	__restore_irq_save(saved);

	if (!(pend->state & TASKLET_SCHEDULED)) {
		printf("%x (%s) on runqueue not scheduled\n", (int)pend, pend->name);
		return;
	}

	pend->next_scheduled = NULL;
	__disable_irq_save(&saved);
	pend->state = (pend->state | TASKLET_RUNNING) & ~TASKLET_SCHEDULED;
	__restore_irq_save(saved);

	// Run, lola, run!
	pend->run(pend);

	__disable_irq_save(&saved);
	pend->state &= ~TASKLET_RUNNING;
	pend->cnt++;
	__restore_irq_save(saved);
}

void tasklet_run_scheduled(void)
{
	if (!tasklet_scheduled_head) {
		//printf("no scheduled tasklets\n");
		return;
	}
	while (tasklet_scheduled_head) {
		tasklet_run_next_scheduled();
	}
}

void tasklet_stats(void)
{
	struct tasklet *t = tasklet_info_head;
	printf("tasklet stats:\n");
	while (t) {
		printf("  %x: %x %s %d %d %d\n",
			(int)t, (int)t->state, t->name, (int)t->cnt, (int)t->ccount, (int)t->ccount_max);
		t = t->next_info;
	}
}
