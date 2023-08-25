#include <stdio.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t cond;
int ready = 0;

void* wait_for_signal(void* data) {
    pthread_mutex_lock(&lock);
    while (!ready) {
        pthread_cond_wait(&cond, &lock);
    }
    printf("Received signal!\n");
    pthread_mutex_unlock(&lock);
    return NULL;
}

void* send_signal(void* data) {
    pthread_mutex_lock(&lock);
    ready = 1;
    pthread_cond_signal(&cond);
    printf("Signal sent.\n");
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t thread_id1, thread_id2;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread_id1, NULL, wait_for_signal, NULL);
    pthread_create(&thread_id2, NULL, send_signal, NULL);

    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return 0;
}

