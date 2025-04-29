#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>  // optional if you want heavier DFS work

#define MAX_VERTICES 10000

struct Graph {
    int V;  //no of vertices
    int** adj; // adj matrix
};

void initGraph(struct Graph* g, int vertices) {
    g->V = vertices;
    g->adj = (int**)malloc(vertices * sizeof(int*));
    for (int i = 0; i < vertices; i++) {
        g->adj[i] = (int*)calloc(vertices, sizeof(int));
    }
}

void freeGraph(struct Graph* g) {
    for (int i = 0; i < g->V; i++) {
        free(g->adj[i]);
    }
    free(g->adj);
}

void addEdge(struct Graph* g, int u, int v) {
    g->adj[u][v] = 1;
    g->adj[v][u] = 1;
}

void generateRandomEdges(struct Graph* g, int max_edges) {
    int edges = 0;
    while (edges < max_edges) {
        int u = rand() % g->V;
        int v = rand() % g->V;
        if (u != v && g->adj[u][v] == 0) {
            addEdge(g, u, v);
            edges++;
        }
    }
}

void sequentialDFS(struct Graph* g, int start, int* visited) {
    visited[start] = 1;
    printf("%d ", start);
    for (int i = 0; i < g->V; i++) {
        if (g->adj[start][i] && !visited[i]) {
            sequentialDFS(g, i, visited);
        }
    }
}

void parallelDFS(struct Graph* g, int start, int* visited) {
    // Double-check locking style, avoid big critical section
    if (__sync_lock_test_and_set(&visited[start], 1) == 1) {
        return;  // Already visited
    }

    printf("%d ", start);

    for (int i = 0; i < g->V; i++) {
        if (g->adj[start][i]) {
            if (!visited[i]) {
                #pragma omp task firstprivate(i)
                {
                    parallelDFS(g, i, visited);
                }
            }
        }
    }
    #pragma omp taskwait
}

void parallelDFSWrapper(struct Graph* g, int start, int* visited) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            parallelDFS(g, start, visited);
        }
    }
}

int main() {
    int i;
    double inputs[5][6];  // [vertices, -, -, sequential_time, parallel_time, speedup]
    int start;

    srand(time(NULL));
    
    omp_set_num_threads(8); // Set threads manually (you can adjust it)

    for (i = 0; i < 5; i++) {
        printf("Enter the number of vertices for graph %d (1 to 10000): ", i + 1);
        scanf("%lf", &inputs[i][0]);
        int V = (int)inputs[i][0];

        if (V < 1 || V > 10000) {
            printf("Number of vertices must be between 1 and 10000.\n");
            return 1;
        }

        int max_edges = V * (V - 1) / 4;  // Fewer edges

        printf("Automatically generating up to %d edges for graph %d.\n", max_edges, i + 1);
        struct Graph g;
        initGraph(&g, V);
        generateRandomEdges(&g, max_edges);

        printf("Enter the starting vertex for DFS for graph %d: ", i + 1);
        scanf("%d", &start);
        if (start < 0 || start >= V) {
            printf("Invalid starting vertex. Please enter between 0 and %d.\n", V - 1);
            return 1;
        }

        int* visited = (int*)calloc(V, sizeof(int));

        double seq_start_time = omp_get_wtime();
        printf("DFS traversal starting from node %d (Sequential): ", start);
        sequentialDFS(&g, start, visited);
        printf("\n");
        double seq_end_time = omp_get_wtime();
        inputs[i][3] = seq_end_time - seq_start_time;

        for (int j = 0; j < V; j++) visited[j] = 0;

        double par_start_time = omp_get_wtime();
        printf("DFS traversal starting from node %d (Parallel): ", start);
        parallelDFSWrapper(&g, start, visited);
        printf("\n");
        double par_end_time = omp_get_wtime();
        inputs[i][4] = par_end_time - par_start_time;

        if (inputs[i][4] > 0)
            inputs[i][5] = inputs[i][3] / inputs[i][4];
        else
            inputs[i][5] = 0.0;

        free(visited);
        freeGraph(&g);
    }

    printf("\n---------------------------------------------------------------\n");
    printf("| Graph | Vertices | Seq Time (s) | Par Time (s) | Speedup    |\n");
    printf("---------------------------------------------------------------\n");
    for (i = 0; i < 5; i++) {
        printf("| %5d | %8.0lf | %13.6f | %13.6f | %10.2f |\n",
               i + 1, inputs[i][0], inputs[i][3], inputs[i][4], inputs[i][5]);
    }
    printf("---------------------------------------------------------------\n");

    return 0;
}
