#include "test.h"
#include "nitro.h"

struct t_1 {
    int r_c;
    int s_c;
};

void *r_1(void *p) {
    struct t_1 *res = (struct t_1 *)p;
    nitro_socket_t *s = nitro_socket_bind("tcp://127.0.0.1:4444");

    int i;

    for (i=0; i < 10000; i++) {
        nitro_frame_t *fr = nitro_recv(s);
        if (*(int*)nitro_frame_data(fr) != i) {
            break;
        }
        nitro_frame_destroy(fr);
        int back = 10000 - i;
        fr = nitro_frame_new_copy(&back, sizeof(int));
        nitro_send(fr, s);
        nitro_frame_destroy(fr);
    }

    res->r_c = i;

    return NULL;
}

void *s_1(void *p) {
    struct t_1 *res = (struct t_1 *)p;
    nitro_socket_t *s = nitro_socket_connect("tcp://127.0.0.1:4444");

    int i;

    for (i=0; i < 10000; i++) {
        nitro_frame_t *fr = nitro_frame_new_copy(&i, sizeof(int));
        fr = nitro_frame_new_copy(&i, sizeof(int));
        nitro_send(fr, s);
        nitro_frame_destroy(fr);

        fr = nitro_recv(s);
        if (*(int*)nitro_frame_data(fr) != (10000 - i)) {
            break;
        }
        nitro_frame_destroy(fr);
    }

    res->s_c = i;

    return NULL;
}

struct t_2 {
    int got[10000];
    _Atomic (int)tot;
    nitro_socket_t *s;
    pthread_t kids[10];
    int each[10];
};

void *r_2(void *p) {
    struct t_2 *res = (struct t_2 *)p;
    nitro_socket_t *s = res->s;
    int g;
    int me = 0;

    while (1) {
        nitro_frame_t *fr = nitro_recv(s);
        g = *(int*)nitro_frame_data(fr);
        res->got[g] = 1;
        nitro_frame_destroy(fr);
        int amt = atomic_fetch_add(&res->tot, 1);
        ++me;
        if (amt >= 9999) {
            break;
        }
    }

    // hack -- kill others
    int i;
    for (i=0; i < 10; i++) {
        if (res->kids[i] == pthread_self()) {
            res->each[i] = me;
            break;
        }
    }
    nitro_queue_push(s->stype.univ.q_recv,
        nitro_frame_new_copy(&g, sizeof(int)));

    return NULL;
}

void *s_2(void *p) {
    nitro_socket_t *s = nitro_socket_connect("tcp://127.0.0.1:4445");
    sleep(1);

    int i;

    for (i=0; i < 10000; i++) {
        nitro_frame_t *fr = nitro_frame_new_copy(&i, sizeof(int));
        fr = nitro_frame_new_copy(&i, sizeof(int));
        nitro_send(fr, s);
        nitro_frame_destroy(fr);
    }


    return NULL;
}

int main(int argc, char **argv) {
    nitro_runtime_start();

    pthread_t t1, t2;

    struct t_1 acc1 = {0};
    pthread_create(&t1, NULL, r_1, &acc1);
    pthread_create(&t2, NULL, s_1, &acc1);

    void *res = NULL;

    pthread_join(t1, res);
    pthread_join(t2, res);


    TEST("r_1(1:1 rpc) all 10,000 matched", acc1.r_c == 10000);
    TEST("s_1(1:1 rpc) all 10,000 matched", acc1.s_c == 10000);

    struct t_2 acc2;
    bzero(&acc2, sizeof(acc2));
    atomic_init(&acc2.tot, 0);

    int i = 0;
    nitro_socket_t *s = nitro_socket_bind("tcp://127.0.0.1:4445");
    acc2.s = s;
    for (i=0; i < 10; i++) {
        pthread_create(&(acc2.kids[i]), NULL, r_2, &acc2);
    }
    pthread_create(&t1, NULL, s_2, &acc2);

    pthread_join(t1, res);
    for (i=0; i < 10; i++) {
        pthread_join(acc2.kids[i], res);
    }

    int set = 0;
    for (i=0; i < 10000; i++) {
        set += acc2.got[i];
    }

    TEST("r_2(q share) all 10,000 got", set == 10000);

    for (i=0; i < 10; i++) {
        TEST("r_2(q share) thread load shared",
            acc2.each[i] > 0);
    }

    SUMMARY(0);
    return 1;
}