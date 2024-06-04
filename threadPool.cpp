#include <iostream>
#include <ucontext.h>
#include <vector>

struct thread_t {
    ucontext_t ctx;
    void (*func)();
    void* args;
    int state;
    char stack[1024 * 128]; //128kB栈空间
};

struct scheduler {
    ucontext_t main;
    std::vector<thread_t> threads;
    int running_thread;
    
    scheduler():running_thread(-1) {};
};


scheduler myscheduler;

int thread_create(scheduler& myscheduler, void (*func)(), void* args) {
    thread_t *newthread = new thread_t();
    newthread->ctx.uc_link = &myscheduler.main;
    newthread->ctx.uc_stack.ss_sp = newthread->stack;
    newthread->ctx.uc_stack.ss_size = 1024*128;
    newthread->func = func;
    newthread->args = args;
    newthread->state = 0;
    myscheduler.threads.push_back(*newthread);
    return myscheduler.threads.size() - 1;
}

int thread_yield(scheduler& myscheduler) {
    if (myscheduler.running_thread == -1) return 0;
    myscheduler.threads[myscheduler.running_thread].state = 2;
    swapcontext(&myscheduler.threads[myscheduler.running_thread].ctx, &myscheduler.main);
    return 1;
}


void thread_exit() {
    myscheduler.threads[running_thread].state = 3;
    myscheduler.running_thread = -1;
}

int thread_resume(scheduler& myscheduler,int threadId) {
    if (threadId < 0 || threadId >= myscheduler.threads.size()) return -1;
    if (myscheduler.threads[threadId].state == 2) {
        //if (myscheduler.running_thread != -1) thread_yield(myscheduler);
        myscheduler.running_thread = threadId;
        myscheduler.threads[threadId].state = 1;
        swapcontext(&myscheduler.main,&myscheduler.threads[threadId].ctx);
    } else if (myscheduler.threads[threadId].state == 0) {    
        //if (myscheduler.running_thread != -1) thread_yield(myscheduler);
        myscheduler.running_thread = threadId;
        myscheduler.threads[threadId].state = 1;
        getcontext(&myscheduler.threads[threadId].ctx);
        makecontext(&myscheduler.threads[threadId].ctx, myscheduler.threads[threadId].func, 1, myscheduler.threads[threadId].args);
        swapcontext(&myscheduler.main,&myscheduler.threads[threadId].ctx);
    }
}

int scheduler_finished(scheduler& myscheduler) {
    for (int i = 0; i < myscheduler.threads.size(); i++) {
        if (myscheduler.threads[i].state != 3) return 0;
    }
    return 1;
}

void thread1() {
    std::cout << "hello" << std::endl;
    thread_exit();
}

void thread2() {
    int n = 10;
    thread_yield(myscheduler);
    while (n--)
        std::cout << "world" << std::endl;
    thread_exit();
}

int main() {
    getcontext(&myscheduler.main);
    thread_create(myscheduler, &thread1, nullptr);
    thread_create(myscheduler, &thread2, nullptr);
    if (!scheduler_finished(myscheduler)) {
        thread_resume(myscheduler, 0);
    }
    if (!scheduler_finished(myscheduler)) {
        thread_resume(myscheduler, 1);
    }
    if (!scheduler_finished(myscheduler)) {
        thread_resume(myscheduler, 1);
    }
    return 0;
}
