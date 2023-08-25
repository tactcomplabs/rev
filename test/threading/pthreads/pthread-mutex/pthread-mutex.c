
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t lock;
int counter = 0;

void* increment_counter(void* data) {
    pthread_mutex_lock(&lock);
    counter++;
    printf("Counter: %d\n", counter);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t thread_id1, thread_id2;
    pthread_mutex_init(&lock, NULL);

    pthread_create(&thread_id1, NULL, increment_counter, NULL);
    pthread_create(&thread_id2, NULL, increment_counter, NULL);

    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}

