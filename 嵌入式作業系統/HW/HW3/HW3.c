#include <stdio.h>

#include <string.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <unistd.h>

#include <stdlib.h>

#include <arpa/inet.h>

#include <pthread.h>

#include <stdarg.h>

#include <stdbool.h>

#include <signal.h>

#include <sys/sem.h>

#include <errno.h>



#define MAX_FOOD_ITEMS 6

#define MAX_ORDER_LEN 256

#define MAX_CALLBACK_LEN 256

#define MAX_IP_LEN 16



__thread int threadid = 0;

pthread_mutex_t mutex, mutex1;

int time_wait1_val = 0;

int time_wait2_val = 0;

int *time_wait1 = &time_wait1_val;

int *time_wait2 = &time_wait2_val;



void signal_handler(int signum) {

    if (signum == SIGINT) {

        pthread_mutex_destroy(&mutex);


        exit(EXIT_SUCCESS);

    }

}



struct food {

    int index;

    int shop_index;

    int price;

    const char *name;

};



struct TcpThread {

    int connfd;

    int shop_index;

    char buffer[256];

    char callback[256];

    struct food food_list[MAX_FOOD_ITEMS];

    int order_list[MAX_FOOD_ITEMS];

    int time;

    int human;

};



int store_dist[] = {3, 5, 8};

struct food food_list[] = {

    {0, 0, 60, "cookie"},

    {1, 0, 80, "cake"},

    {2, 1, 40, "tea"},

    {3, 1, 70, "boba"},

    {4, 2, 120, "fried-rice"},

    {5, 2, 50, "Egg-drop-soup"}

};



const char *get_food_name(int index) {

    for (size_t i = 0; i < sizeof(food_list) / sizeof(food_list[0]); ++i)

        if (food_list[i].index == index)

            return food_list[i].name;

    return "Unknown";

}



int get_shop_index(const char *food_name) {

    for (size_t i = 0; i < sizeof(food_list) / sizeof(food_list[0]); ++i)

        if (strcmp(food_name, food_list[i].name) == 0)

            return food_list[i].shop_index;

    return -1;

}



int get_index(const char *food_name) {

    for (size_t i = 0; i < sizeof(food_list) / sizeof(food_list[0]); ++i)

        if (strcmp(food_name, food_list[i].name) == 0)

            return food_list[i].index;

    return -1;

}



void errexit(const char *format, ...) {

    va_list args;

    va_start(args, format);

    vfprintf(stderr, format, args);

    va_end(args);

    exit(EXIT_FAILURE);

}



void callback_order_list(struct TcpThread *thread) {

    int count = 0;

    char temp_key[50];

    thread->callback[0] = '\0';

    for (int i = 0; i < MAX_FOOD_ITEMS; i++) {

        if (thread->order_list[i]) {

            if (count) strcat(thread->callback, "|");

            const char *food_name = get_food_name(i);

            sprintf(temp_key, "%s %d", food_name, thread->order_list[i]);

            strcat(thread->callback, temp_key);

            count++;

        }

    }

    strcat(thread->callback, "\n");

}



void *tcp_thread_main(void *arg) {

    struct TcpThread *thread = (struct TcpThread *)arg;

    threadid = thread->connfd;

    while (1) {

        char *temp, *action_vec[3] = {NULL};

        int i = 0;

        if (read(thread->connfd, thread->buffer, sizeof(thread->buffer)) < 0){

            errexit("Error : read()\n");

            close(thread->connfd);

            free(thread);

            return NULL;

        }


        if (strcmp(thread->buffer, "") == 0) break;


        temp = strtok(thread->buffer, " ");

        while (temp && i < 3) action_vec[i++] = temp, temp = strtok(NULL, " ");



        char *action = action_vec[0], *food_name = action_vec[1];

        if (strcmp(action, "order") == 0) {

            int number, shop_index_temp = get_shop_index(food_name);

            sscanf(action_vec[2], "%d", &number);

            if (thread->shop_index < 0) thread->shop_index = shop_index_temp;

            else if (thread->shop_index != shop_index_temp) {


                callback_order_list(thread);

                if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

                    errexit("Error : write()\n");

                continue;

            }

            thread->order_list[get_index(food_name)] += number;

            callback_order_list(thread);

        } else if (strcmp(action, "confirm\n") == 0 || strcmp(action, "confirm") == 0) {


            int total = 0;

            for (int i = 0; i < MAX_FOOD_ITEMS; i++)

                total += thread->food_list[i].price * thread->order_list[i];



            if(total == 0) {

                sprintf(thread->callback, "Please order some meals\n");

            } else {

                pthread_mutex_lock(&mutex);

                if(*time_wait1 > *time_wait2) {

                    *time_wait2 += store_dist[thread->shop_index];

                    thread->time = *time_wait2;

                    thread->human = 2;

                } else {

                    *time_wait1 += store_dist[thread->shop_index];

                    thread->time = *time_wait1;

                    thread->human = 1;

                }


                pthread_mutex_unlock(&mutex);



                if(thread->time > 30){


                    sprintf(thread->callback, "Your delivery will take a long time, do you want to wait?\n");


                    if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

                        errexit("Error : write()\n");

                    continue;

                }



                sprintf(thread->callback, "Please wait a few minutes...\n");


                if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

                    errexit("Error : write()\n");



                sleep(thread->time);

                pthread_mutex_lock(&mutex);

                if(thread->human == 2)

                    *time_wait2 -= store_dist[thread->shop_index];

                else

                    *time_wait1 -= store_dist[thread->shop_index];

                pthread_mutex_unlock(&mutex);



                sprintf(thread->callback, "Delivery has arrived and you need to pay %d$\n", total);


                if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

                    errexit("Error : write()\n");

                break;

            }

        } else if (strcmp(action, "cancel") == 0) break;

        else if (strcmp(action, "shop") == 0) {

            sprintf(thread->callback,

                "Dessert Shop:3km\n- cookie:$60|cake:$80\n"

                "Beverage Shop:5km\n- tea:$40|boba:$70\n"

                "Diner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n");

        } else if (strcmp(action, "Yes\n") == 0 || strcmp(action, "Yes") == 0) {


            int total = 0;

            for (int i = 0; i < MAX_FOOD_ITEMS; i++)

                total += thread->food_list[i].price * thread->order_list[i];



            if(total == 0 ){

                sprintf(thread->callback, "Please order some meals\n");


                if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

                    errexit("Error : write()\n");

            }



            sprintf(thread->callback, "Please wait a few minutes...\n");


            if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

                errexit("Error : write()\n");



            sleep(thread->time);

            pthread_mutex_lock(&mutex);

            if(thread->human == 2) *time_wait2 -= store_dist[thread->shop_index];

            else *time_wait1 -= store_dist[thread->shop_index];

            pthread_mutex_unlock(&mutex);



            sprintf(thread->callback, "Delivery has arrived and you need to pay %d$\n", total);



            if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

                errexit("Error : write()\n");

            break;

        } else if (strcmp(action, "No\n") == 0) break;

        else break;


        if (write(thread->connfd, thread->callback, sizeof(thread->callback)) == -1)

            errexit("Error : write()\n");



        memset(thread->buffer, 0, sizeof(thread->buffer));

        memset(thread->callback, 0, sizeof(thread->callback));

    }



    close(thread->connfd);

    free(thread);

    return NULL;

}


int main(int argc, char *argv[]) {


    signal(SIGINT, signal_handler);

    if (argc != 2) {

        fprintf(stderr, "Usage: %s <port>\n", argv[0]);


        exit(EXIT_FAILURE);

    }



    int port = atoi(argv[1]), server_socket, client_socket;

    struct sockaddr_in server, client;

    socklen_t client_len = sizeof(struct sockaddr_in);



    server.sin_family = AF_INET;

    server.sin_addr.s_addr = INADDR_ANY;

    server.sin_port = htons(port);



    pthread_mutex_init(&mutex, NULL);

    pthread_mutex_init(&mutex1, NULL);



    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {

        perror("Could not create socket");


        exit(EXIT_FAILURE);

    }



    int optval = 1;

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {

        perror("Bind failed");

        close(server_socket);


        exit(EXIT_FAILURE);

    }



    listen(server_socket, 10);



    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len))) {

        pthread_t client_thread;

        struct TcpThread *new_thread = malloc(sizeof(struct TcpThread));

        if (!new_thread) {

            perror("Malloc failed");

            close(client_socket);

            continue;

        }

        new_thread->connfd = client_socket;

        new_thread->shop_index = -1;

        new_thread->time = 0;

        new_thread->human = 0;

        memset(new_thread->buffer, 0, sizeof(new_thread->buffer));

        memset(new_thread->callback, 0, sizeof(new_thread->callback));

        memset(new_thread->order_list, 0, sizeof(new_thread->order_list));

        memcpy(new_thread->food_list, food_list, sizeof(food_list));



        if (pthread_create(&client_thread, NULL, tcp_thread_main, (void *)new_thread) != 0) {

            perror("Thread creation failed");

            free(new_thread);

            close(client_socket);

            continue;

        }



        pthread_detach(client_thread);

    }



    if (client_socket < 0) {

        perror("Accept failed");

        close(server_socket);

        exit(EXIT_FAILURE);

    }



    close(server_socket);


    return 0;

}