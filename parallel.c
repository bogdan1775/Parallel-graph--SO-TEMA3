// SPDX-License-Identifier: BSD-3-Clause

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "log/log.h"
#include "os_graph.h"
#include "os_threadpool.h"
#include "utils.h"

#define NUM_THREADS 4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;
/* TODO: Define graph synchronization mechanisms. */

pthread_mutex_t lock;

/* TODO: Define graph task argument. */

static void process_node(unsigned int idx)
{
	/* TODO: Implement thread-pool based processing of graph. */
	os_node_t *node = graph->nodes[idx];

	pthread_mutex_lock(&lock);
	sum = sum + node->info;
	pthread_mutex_unlock(&lock);

	for (unsigned int i = 0; i < node->num_neighbours; i++) {
		pthread_mutex_lock(&lock);
		if (graph->visited[node->neighbours[i]] == 0) {
			graph->visited[node->neighbours[i]] = 1;
			pthread_mutex_unlock(&lock);
			void *p;

			memcpy(&p, &node->neighbours[i], sizeof(unsigned int));
			os_task_t *task = create_task((void *)process_node, p, NULL);

			enqueue_task(tp, task);
		} else {
			pthread_mutex_unlock(&lock);
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);

	/* TODO: Initialize graph synchronization mechanisms. */
	pthread_mutex_init(&lock, NULL);

	tp = create_threadpool(NUM_THREADS);

	// il marchez ca vizitat ca pornesc din el
	graph->visited[0] = 1;
	process_node(0);
	wait_for_completion(tp);
	destroy_threadpool(tp);

	printf("%d", sum);

	// pentru destroy
	pthread_mutex_destroy(&lock);

	return 0;
}
