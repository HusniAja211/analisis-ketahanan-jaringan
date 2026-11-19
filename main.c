
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_DEVICE 20
#define MAX_NAME 30
#define INF INT_MAX

// ==========================================
// STRUKTUR DATA
// ==========================================

typedef struct Edge {
    int destination;
    int weight;
    struct Edge *next;
} Edge;

typedef struct {
    char name[MAX_NAME];
    int active;
    Edge *adjacency;
} Device;

typedef struct {
    Device devices[MAX_DEVICE];
    int deviceCount;
} Graph;

// ==========================================
// INPUT
// ==========================================

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int readInteger(const char *prompt) {
    char input[100];
    char *end;
    long value;

    while (1) {
        printf("%s", prompt);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            exit(1);
        }

        value = strtol(input, &end, 10);

        while (*end == ' ' || *end == '\t') {
            end++;
        }

        if (end != input && (*end == '\n' || *end == '\0')
            && value >= 0 && value <= INT_MAX) {
            return (int)value;
        }

        printf("Input tidak valid! Masukkan angka positif atau nol.\n");
    }
}

void readString(const char *prompt, char *output, int size) {
    printf("%s", prompt);

    if (fgets(output, size, stdin) == NULL) {
        exit(1);
    }

    output[strcspn(output, "\n")] = '\0';
}

// ==========================================
// MANAJEMEN GRAPH
// ==========================================

void initializeGraph(Graph *graph) {
    graph->deviceCount = 0;

    for (int i = 0; i < MAX_DEVICE; i++) {
        graph->devices[i].active = 1;
        graph->devices[i].adjacency = NULL;
    }
}

int findDevice(Graph *graph, const char *name) {
    for (int i = 0; i < graph->deviceCount; i++) {
        if (strcmp(graph->devices[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

void addDevice(Graph *graph) {
    char name[MAX_NAME];

    if (graph->deviceCount >= MAX_DEVICE) {
        printf("Kapasitas perangkat sudah penuh!\n");
        return;
    }

    readString("Nama perangkat: ", name, MAX_NAME);

    if (strlen(name) == 0) {
        printf("Nama perangkat tidak boleh kosong!\n");
        return;
    }

    if (findDevice(graph, name) != -1) {
        printf("Perangkat sudah terdaftar!\n");
        return;
    }

    int index = graph->deviceCount;

    strcpy(graph->devices[index].name, name);
    graph->devices[index].active = 1;
    graph->devices[index].adjacency = NULL;

    graph->deviceCount++;

    printf("Perangkat %s berhasil ditambahkan.\n", name);
}

void insertEdge(int source, int destination, int weight,
                Graph *graph) {
    Edge *newEdge = malloc(sizeof(Edge));

    if (newEdge == NULL) {
        printf("Gagal mengalokasikan memori!\n");
        exit(1);
    }

    newEdge->destination = destination;
    newEdge->weight = weight;
    newEdge->next = graph->devices[source].adjacency;

    graph->devices[source].adjacency = newEdge;
}

Edge *findEdge(Graph *graph, int source, int destination) {
    Edge *current = graph->devices[source].adjacency;

    while (current != NULL) {
        if (current->destination == destination) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

void addConnection(Graph *graph) {
    char sourceName[MAX_NAME];
    char destinationName[MAX_NAME];

    readString("Perangkat asal: ", sourceName, MAX_NAME);
    readString("Perangkat tujuan: ", destinationName, MAX_NAME);

    int source = findDevice(graph, sourceName);
    int destination = findDevice(graph, destinationName);

    if (source == -1 || destination == -1) {
        printf("Salah satu perangkat tidak ditemukan!\n");
        return;
    }

    if (source == destination) {
        printf("Perangkat tidak dapat terhubung ke dirinya sendiri!\n");
        return;
    }

    if (findEdge(graph, source, destination) != NULL) {
        printf("Koneksi sudah tersedia!\n");
        return;
    }

    int weight = readInteger("Bobot koneksi: ");

    if (weight <= 0) {
        printf("Bobot harus lebih besar dari nol!\n");
        return;
    }

    // Graph tidak berarah: simpan dua arah.
    insertEdge(source, destination, weight, graph);
    insertEdge(destination, source, weight, graph);

    printf("Koneksi berhasil ditambahkan!\n");
}

// ==========================================
// MENAMPILKAN TOPOLOGI
// ==========================================

void displayGraph(Graph *graph) {
    printf("\n========== TOPOLOGI JARINGAN ==========\n");

    if (graph->deviceCount == 0) {
        printf("Belum ada perangkat.\n");
        return;
    }

    for (int i = 0; i < graph->deviceCount; i++) {
        printf("\n%s [%s]\n",
            graph->devices[i].name,
            graph->devices[i].active ? "AKTIF" : "NONAKTIF");

        Edge *current = graph->devices[i].adjacency;

        if (current == NULL) {
            printf("  Tidak memiliki koneksi.\n");
        }

        while (current != NULL) {
            printf("  -> %s (bobot: %d)\n",
                graph->devices[current->destination].name,
                current->weight);

            current = current->next;
        }
    }
}

// ==========================================
// BFS DAN CONNECTED COMPONENTS
// ==========================================

void analyzeComponents(Graph *graph) {
    int visited[MAX_DEVICE] = {0};
    int queue[MAX_DEVICE];

    int componentCount = 0;
    int activeCount = 0;

    printf("\n========== ANALISIS KONEKTIVITAS ==========\n");

    for (int i = 0; i < graph->deviceCount; i++) {
        if (graph->devices[i].active) {
            activeCount++;
        }
    }

    if (activeCount == 0) {
        printf("Tidak ada perangkat aktif.\n");
        return;
    }

    for (int i = 0; i < graph->deviceCount; i++) {
        if (!graph->devices[i].active || visited[i]) {
            continue;
        }

        componentCount++;

        int front = 0;
        int rear = 0;

        queue[rear++] = i;
        visited[i] = 1;

        printf("Komponen %d: ", componentCount);

        while (front < rear) {
            int currentDevice = queue[front++];

            printf("%s ",
                graph->devices[currentDevice].name);

            Edge *current = graph->devices[currentDevice].adjacency;

            while (current != NULL) {
                int neighbor = current->destination;

                if (current->weight > 0
                    && graph->devices[neighbor].active
                    && !visited[neighbor]) {

                    visited[neighbor] = 1;
                    queue[rear++] = neighbor;
                }

                current = current->next;
            }
        }

        printf("\n");
    }

    printf("\nJumlah perangkat aktif: %d\n", activeCount);
    printf("Jumlah komponen: %d\n", componentCount);

    if (componentCount == 1) {
        printf("Status: Jaringan terhubung.\n");
    } else {
        printf("Status: Jaringan TIDAK terhubung sepenuhnya.\n");
    }
}

// ==========================================
// SIMULASI KEGAGALAN PERANGKAT
// ==========================================

void simulateDeviceFailure(Graph *graph) {
    char name[MAX_NAME];

    readString("Nama perangkat yang ingin dinonaktifkan: ", name, MAX_NAME);

    int index = findDevice(graph, name);

    if (index == -1) {
        printf("Perangkat tidak ditemukan!\n");
        return;
    }

    if (!graph->devices[index].active) {
        printf("Perangkat sudah nonaktif!\n");
        return;
    }

    printf("\nSimulasi kegagalan perangkat: %s\n", name);

    // Nonaktifkan perangkat sementara.
    graph->devices[index].active = 0;

    analyzeComponents(graph);

    // Pulihkan kondisi awal.
    graph->devices[index].active = 1;

    printf("\nSimulasi selesai. Perangkat dipulihkan.\n");
}

// ==========================================
// SIMULASI KEGAGALAN KONEKSI
// ==========================================

void simulateConnectionFailure(Graph *graph) {
    char sourceName[MAX_NAME];
    char destinationName[MAX_NAME];

    readString("Perangkat pertama: ", sourceName, MAX_NAME);
    readString("Perangkat kedua: ", destinationName, MAX_NAME);

    int source = findDevice(graph, sourceName);
    int destination = findDevice(graph, destinationName);

    if (source == -1 || destination == -1) {
        printf("Perangkat tidak ditemukan!\n");
        return;
    }

    Edge *edgeA = findEdge(graph, source, destination);
    Edge *edgeB = findEdge(graph, destination, source);

    if (edgeA == NULL || edgeB == NULL) {
        printf("Koneksi tidak ditemukan!\n");
        return;
    }

    printf("\nSimulasi kegagalan koneksi: %s -- %s\n",
        sourceName, destinationName);

    // Putuskan koneksi sementara.
    edgeA->weight = -edgeA->weight;
    edgeB->weight = -edgeB->weight;

    analyzeComponents(graph);

    // Pulihkan bobot koneksi.
    edgeA->weight = -edgeA->weight;
    edgeB->weight = -edgeB->weight;

    printf("\nSimulasi selesai. Koneksi dipulihkan.\n");
}

// ==========================================
// DIJKSTRA - SHORTEST PATH
// ==========================================

void shortestPath(Graph *graph) {
    char sourceName[MAX_NAME];
    char destinationName[MAX_NAME];

    readString("Perangkat asal: ", sourceName, MAX_NAME);
    readString("Perangkat tujuan: ", destinationName, MAX_NAME);

    int source = findDevice(graph, sourceName);
    int destination = findDevice(graph, destinationName);

    if (source == -1 || destination == -1) {
        printf("Perangkat tidak ditemukan!\n");
        return;
    }

    if (!graph->devices[source].active
        || !graph->devices[destination].active) {
        printf("Perangkat asal atau tujuan sedang nonaktif!\n");
        return;
    }

    int distance[MAX_DEVICE];
    int visited[MAX_DEVICE] = {0};
    int previous[MAX_DEVICE];

    for (int i = 0; i < graph->deviceCount; i++) {
        distance[i] = INF;
        previous[i] = -1;
    }

    distance[source] = 0;

    for (int count = 0; count < graph->deviceCount; count++) {
        int current = -1;
        int minimum = INF;

        // Cari perangkat belum dikunjungi dengan jarak terkecil.
        for (int i = 0; i < graph->deviceCount; i++) {
            if (graph->devices[i].active
                && !visited[i]
                && distance[i] < minimum) {

                minimum = distance[i];
                current = i;
            }
        }

        if (current == -1) {
            break;
        }

        visited[current] = 1;

        Edge *edge = graph->devices[current].adjacency;

        while (edge != NULL) {
            int neighbor = edge->destination;

            // Bobot negatif menandakan koneksi sedang putus.
            if (edge->weight > 0
                && graph->devices[neighbor].active
                && !visited[neighbor]
                && distance[current] != INF
                && distance[current] + edge->weight < distance[neighbor]) {

                distance[neighbor] = distance[current] + edge->weight;
                previous[neighbor] = current;
            }

            edge = edge->next;
        }
    }

    if (distance[destination] == INF) {
        printf("Tidak ada jalur yang menghubungkan kedua perangkat.\n");
        return;
    }

    int path[MAX_DEVICE];
    int pathCount = 0;

    int current = destination;

    while (current != -1) {
        path[pathCount++] = current;
        current = previous[current];
    }

    printf("\n========== HASIL DIJKSTRA ==========\n");
    printf("Jalur terpendek: ");

    for (int i = pathCount - 1; i >= 0; i--) {
        printf("%s", graph->devices[path[i]].name);

        if (i > 0) {
            printf(" -> ");
        }
    }

    printf("\nTotal bobot: %d\n", distance[destination]);
}

// ==========================================
// MEMORI
// ==========================================

void freeGraph(Graph *graph) {
    for (int i = 0; i < graph->deviceCount; i++) {
        Edge *current = graph->devices[i].adjacency;

        while (current != NULL) {
            Edge *temp = current;
            current = current->next;
            free(temp);
        }
    }
}

// ==========================================
// MENU UTAMA
// ==========================================

void displayMenu(void) {
    printf("\n====================================\n");
    printf("       NETWORK RESILIENCE ANALYZER\n");
    printf("====================================\n");
    printf("1. Tambah perangkat\n");
    printf("2. Tambah koneksi\n");
    printf("3. Tampilkan topologi jaringan\n");
    printf("4. Analisis konektivitas\n");
    printf("5. Simulasi kegagalan perangkat\n");
    printf("6. Simulasi kegagalan koneksi\n");
    printf("7. Cari jalur terpendek (Dijkstra)\n");
    printf("0. Keluar\n");
    printf("====================================\n");
}

int main(void) {
    Graph graph;
    initializeGraph(&graph);

    int choice;

    do {
        displayMenu();
        choice = readInteger("Pilih menu: ");

        switch (choice) {
            case 1:
                addDevice(&graph);
                break;

            case 2:
                addConnection(&graph);
                break;

            case 3:
                displayGraph(&graph);
                break;

            case 4:
                analyzeComponents(&graph);
                break;

            case 5:
                simulateDeviceFailure(&graph);
                break;

            case 6:
                simulateConnectionFailure(&graph);
                break;

            case 7:
                shortestPath(&graph);
                break;

            case 0:
                printf("Menutup program...\n");
                break;

            default:
                printf("Pilihan tidak tersedia!\n");
        }

    } while (choice != 0);

    freeGraph(&graph);

    return 0;
}
