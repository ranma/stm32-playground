#ifndef _TASKLET_H_
#define _TASKLET_H_

#define TASKLET_REGISTERED  0x00000002
#define TASKLET_SCHEDULED   0x00000004
#define TASKLET_RUNNING     0x00000008

struct tasklet;

typedef void (tasklet_run_fn)(struct tasklet *ctx);

struct tasklet {
	struct tasklet *next_scheduled;
	struct tasklet *next_info;
	uint32_t state;
	uint32_t cnt;
	uint32_t ccount;
	uint32_t ccount_max;
	const char *name;
	tasklet_run_fn *run;
};

void tasklet_register(struct tasklet *tasklet, const char *name, tasklet_run_fn *run);
void tasklet_schedule(struct tasklet *tasklet);
void tasklet_run_scheduled(void);
void tasklet_stats(void);

#endif
