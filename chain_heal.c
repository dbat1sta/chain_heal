#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_PLAYERS 1000

struct node {
    char name[100];
    int x, y, hp;
    struct node **adj_list;
    int adj_count;
    int visited;
};

struct answer {
    struct node *best_path[MAX_PLAYERS];
    struct node *current_path[MAX_PLAYERS];
    int best_path_length;
    int current_path_length;
    int best_healing;
    int current_healing;
};

int calculate_distance_squared(struct node *a, struct node *b) {
    int dx = a->x - b->x;
    int dy = a->y - b->y;
    return dx * dx + dy * dy;
}

int is_adjacent(struct node *a, struct node *b, int jump_range) {
    return calculate_distance_squared(a, b) <= jump_range * jump_range;
}

struct node* create_node_array(int *num_players) {
    char buffer[256];
    struct node *nodes = malloc(MAX_PLAYERS * sizeof(struct node));

    *num_players = 0;
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        sscanf(buffer, "%d %d %d %d %[^\n]", &nodes[*num_players].x, &nodes[*num_players].y, &nodes[*num_players].hp, &nodes[*num_players].adj_count, nodes[*num_players].name);
        nodes[*num_players].adj_list = NULL;
        nodes[*num_players].visited = 0;
        (*num_players)++;
    }
    return nodes;
}

void setup_graph(struct node *nodes, int num_players, int jump_range) {
    for (int i = 0; i < num_players; i++) {
        int count = 0;
        for (int j = 0; j < num_players; j++) {
            if (i != j && is_adjacent(&nodes[i], &nodes[j], jump_range)) {
                count++;
            }
        }
        nodes[i].adj_list = malloc(count * sizeof(struct node *));
        nodes[i].adj_count = 0;
        for (int j = 0; j < num_players; j++) {
            if (i != j && is_adjacent(&nodes[i], &nodes[j], jump_range)) {
                nodes[i].adj_list[nodes[i].adj_count++] = &nodes[j];
            }
        }
    }
}

void dfs(struct node *current, struct answer *ans, int depth, int max_depth, double healing_reduction, double max_healing) {
    if (current->visited) return;

    int reduced_hp = current->hp * pow(healing_reduction, depth - 1); 
    ans->current_path[ans->current_path_length++] = current;
    ans->current_healing += reduced_hp;
    current->visited = 1;

    if (ans->current_healing > max_healing) {
        current->visited = 0;
        ans->current_path_length--;
        ans->current_healing -= reduced_hp;
        return;
    }

    if (depth == max_depth || current->adj_count == 0) {
        if (ans->current_healing > ans->best_healing) {
            ans->best_healing = ans->current_healing;
            ans->best_path_length = ans->current_path_length;
            memcpy(ans->best_path, ans->current_path, ans->best_path_length * sizeof(struct node *));
        }
    } else {
        for (int i = 0; i < current->adj_count; i++) {
            dfs(current->adj_list[i], ans, depth + 1, max_depth, healing_reduction, max_healing);
        }
    }

    current->visited = 0;
    ans->current_path_length--;
    ans->current_healing -= reduced_hp;
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Usage: ./chain_heal <initial_range> <jump_range> <max_depth> <max_healing> <healing_reduction>\n");
        return 1;
    }

    int initial_range, jump_range, max_depth;
    double max_healing, healing_reduction;
    sscanf(argv[1], "%d", &initial_range);
    sscanf(argv[2], "%d", &jump_range);
    sscanf(argv[3], "%d", &max_depth);
    sscanf(argv[4], "%lf", &max_healing);
    sscanf(argv[5], "%lf", &healing_reduction);

    int num_players;
    struct node *nodes = create_node_array(&num_players);
    setup_graph(nodes, num_players, jump_range);

    struct answer ans = {0};

    struct node *urgosa = NULL;
    for (int i = 0; i < num_players; i++) {
        if (strstr(nodes[i].name, "Urgosa")) {
            urgosa = &nodes[i];
            break;
        }
    }

    if (!urgosa) {
        printf("Urgosa not found!\n");
        return 1;
    }

    for (int i = 0; i < num_players; i++) {
        if (i != urgosa - nodes && is_adjacent(urgosa, &nodes[i], initial_range)) {
            dfs(&nodes[i], &ans, 1, max_depth, healing_reduction, max_healing);
        }
    }

    printf("Best Healing Path:\n");
    for (int i = 0; i < ans.best_path_length; i++) {
        printf("%s %d\n", ans.best_path[i]->name, ans.best_path[i]->hp);
    }
    printf("Total Healing: %d\n", ans.best_healing);

    for (int i = 0; i < num_players; i++) {
        free(nodes[i].adj_list);
    }
    free(nodes);

    return 0;
}
