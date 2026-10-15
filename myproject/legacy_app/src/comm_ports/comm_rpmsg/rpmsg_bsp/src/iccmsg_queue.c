#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/queue.h>
#include <errno.h>
#include "iccmsg_queue.h"
// 初始化消息
void init_message(message_t *msg) {
    msg->id = 0;
    msg->data = NULL;
    msg->data_len = 0;
    memset(&msg->timestamp, 0, sizeof(struct timeval));
}

// 复制消息（深拷贝）
int copy_message(message_t *dest, const message_t *src) {
    dest->id = src->id;
    dest->data_len = src->data_len;
    dest->state = src->state;
    dest->ch_type = src->ch_type;
    gettimeofday(&dest->timestamp, NULL);  // 记录接收时间
    
    if (src->data && src->data_len > 0) {
        dest->data = malloc(src->data_len);
        if (!dest->data) {
            return -1;
        }
        memcpy(dest->data, src->data, src->data_len);
    } else {
        dest->data = NULL;
        dest->data_len = 0;
    }
    
    return 0;
}

// 释放消息内存
void free_message(message_t *msg) {
    if (msg->data) {
        free(msg->data);
        msg->data = NULL;
        msg->data_len = 0;
    }
}

// 初始化线程安全队列
int init_queue(thread_safe_queue_t *tsq, int max_size) {
    TAILQ_INIT(&tsq->queue);
    pthread_mutex_init(&tsq->mutex, NULL);
    pthread_cond_init(&tsq->cond, NULL);
    tsq->closed = 0;
    tsq->max_size = max_size;
    tsq->total_messages_sent = 0;
    tsq->total_messages_received = 0;
    tsq->total_bytes_sent = 0;
    tsq->total_bytes_received = 0;
    return 0;
}

// 销毁队列
int destroy_queue(thread_safe_queue_t *tsq) {
    pthread_mutex_lock(&tsq->mutex);
    msg_node_t *node;
    while ((node = TAILQ_FIRST(&tsq->queue)) != NULL) {
        TAILQ_REMOVE(&tsq->queue, node, entries);
        free_message(&node->msg);
        free(node);
    }
    tsq->closed = 1;
    pthread_cond_broadcast(&tsq->cond);
    pthread_mutex_unlock(&tsq->mutex);
    
    pthread_mutex_destroy(&tsq->mutex);
    pthread_cond_destroy(&tsq->cond);
    return 0;
}

// 发送消息到队列
int send_message(thread_safe_queue_t *tsq, const message_t *msg) {
    pthread_mutex_lock(&tsq->mutex);
    
    // 等待直到队列未满或关闭
    while (!tsq->closed && 
           (tsq->max_size > 0 && TAILQ_EMPTY(&tsq->queue) ? 0 : (TAILQ_EMPTY(&tsq->queue) ? 0 : 1)) >= tsq->max_size) {
        pthread_cond_wait(&tsq->cond, &tsq->mutex);
    }
    
    if (tsq->closed) {
        pthread_mutex_unlock(&tsq->mutex);
        return -1;
    }
    
    msg_node_t *node = malloc(sizeof(msg_node_t));
    if (!node) {
        pthread_mutex_unlock(&tsq->mutex);
        return -1;
    }
    
    // 深拷贝消息
    if (copy_message(&node->msg, msg) != 0) {
        free(node);
        pthread_mutex_unlock(&tsq->mutex);
        return -1;
    }
    
    // 记录发送时间
    gettimeofday(&node->msg.timestamp, NULL);
    
    TAILQ_INSERT_TAIL(&tsq->queue, node, entries);
    
    // 更新统计信息
    tsq->total_messages_sent++;
    tsq->total_bytes_sent += msg->data_len;
    
    pthread_cond_signal(&tsq->cond);
    pthread_mutex_unlock(&tsq->mutex);
    
    return 0;
}

/**
 * 带超时等待的消息接收函数
 * @param tsq 线程安全队列指针
 * @param msg 输出参数，用于存储读取到的消息
 * @param timeout_ms 超时时间（毫秒），>0：等待指定时长；0：非阻塞；<0：无限阻塞（还原原逻辑）
 * @return 0：成功读取消息；-1：队列关闭且无消息；-2：超时无消息；-3：消息拷贝失败；-4：参数错误
 */
int receive_message_timeout(thread_safe_queue_t *tsq, message_t *msg, int timeout_ms) {
    // 1. 参数合法性检查
    if (tsq == NULL || msg == NULL) {
        return -4; // 参数错误
    }

    // 2. 加锁（保护队列操作）
    pthread_mutex_lock(&tsq->mutex);

    // 3. 构造超时时间（仅当timeout_ms>0时）
    struct timespec abs_timeout;
    if (timeout_ms > 0) {
        // 获取当前系统时间（秒+纳秒）
        if (clock_gettime(CLOCK_REALTIME, &abs_timeout) != 0) {
            pthread_mutex_unlock(&tsq->mutex);
            return -4; // 获取时间失败
        }
        // 转换超时毫秒到秒+纳秒，累加到当前时间（绝对超时时间）
        abs_timeout.tv_sec += timeout_ms / 1000;
        abs_timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
        // 处理纳秒溢出（超过1秒）
        if (abs_timeout.tv_nsec >= 1000000000) {
            abs_timeout.tv_sec += 1;
            abs_timeout.tv_nsec -= 1000000000;
        }
    }

    // 4. 带超时的等待逻辑（处理虚假唤醒）
    int wait_ret = 0;
    while (TAILQ_EMPTY(&tsq->queue) && !tsq->closed) {
        if (timeout_ms < 0) {
            // timeout_ms<0：无限阻塞（还原原逻辑）
            wait_ret = pthread_cond_wait(&tsq->cond, &tsq->mutex);
        } else if (timeout_ms == 0) {
            // timeout_ms=0：非阻塞，直接退出循环
            wait_ret = ETIMEDOUT;
            break;
        } else {
            // timeout_ms>0：带超时等待
            wait_ret = pthread_cond_timedwait(&tsq->cond, &tsq->mutex, &abs_timeout);
        }

        // 检查等待结果
        if (wait_ret == ETIMEDOUT) {
            // 超时：解锁并返回-2
            pthread_mutex_unlock(&tsq->mutex);
            return -2;
        } else if (wait_ret != 0) {
            // 其他等待错误（如信号中断）：解锁并返回-4
            pthread_mutex_unlock(&tsq->mutex);
            return -4;
        }
        // 无错误且未超时：继续循环检查队列（处理虚假唤醒）
    }

    // 5. 检查队列最终状态
    if (tsq->closed && TAILQ_EMPTY(&tsq->queue)) {
        // 队列关闭且空：返回-1
        pthread_mutex_unlock(&tsq->mutex);
        return -1;
    }

    // 6. 有消息时的处理逻辑（和原函数一致）
    msg_node_t *node = TAILQ_FIRST(&tsq->queue);
    if (node) {
        TAILQ_REMOVE(&tsq->queue, node, entries);

        // 深拷贝消息
        int copy_ret = copy_message(msg, &node->msg);
        free_message(&node->msg);
        free(node);

        if (copy_ret != 0) {
            pthread_mutex_unlock(&tsq->mutex);
            return -3; // 拷贝失败
        }

        // 更新统计
        tsq->total_messages_received++;
        tsq->total_bytes_received += msg->data_len;
    }

    // 7. 唤醒生产者（保留原逻辑）
    pthread_cond_signal(&tsq->cond);
    pthread_mutex_unlock(&tsq->mutex);

    // 成功读取：返回0
    return 0;
}

// 接收消息从队列
int receive_message(thread_safe_queue_t *tsq, message_t *msg) {
    pthread_mutex_lock(&tsq->mutex);
    
    while (TAILQ_EMPTY(&tsq->queue) && !tsq->closed) {
        pthread_cond_wait(&tsq->cond, &tsq->mutex);
    }
    
    if (tsq->closed && TAILQ_EMPTY(&tsq->queue)) {
        pthread_mutex_unlock(&tsq->mutex);
        return -1;
    }
    
    msg_node_t *node = TAILQ_FIRST(&tsq->queue);
    if (node) {
        TAILQ_REMOVE(&tsq->queue, node, entries);
        
        // 深拷贝消息到输出参数
        int result = copy_message(msg, &node->msg);
        free_message(&node->msg);
        free(node);
        
        if (result != 0) {
            pthread_mutex_unlock(&tsq->mutex);
            return -1;
        }
        
        // 更新统计信息
        tsq->total_messages_received++;
        tsq->total_bytes_received += msg->data_len;
    }
    
    pthread_cond_signal(&tsq->cond); // 唤醒可能等待的发送者
    pthread_mutex_unlock(&tsq->mutex);
    return 0;
}

// 创建消息
message_t* create_message(int id, size_t data_size) {
    // 限制消息大小不超过最大值
    if (data_size > MAX_MESSAGE_SIZE) {
        printf("Warning: Message size %zu exceeds MAX_MESSAGE_SIZE %d, limiting to %d\n", 
               data_size, MAX_MESSAGE_SIZE, MAX_MESSAGE_SIZE);
        data_size = MAX_MESSAGE_SIZE;
    }
    
    message_t *msg = malloc(sizeof(message_t));
    if (!msg) return NULL;
    
    init_message(msg);
    msg->id = id;
    msg->data_len = data_size;
    
    if (data_size > 0) {
        msg->data = malloc(data_size);
        if (!msg->data) {
            free(msg);
            return NULL;
        }
        // 填充一些数据模式
        for (size_t i = 0; i < data_size; i++) {
            msg->data[i] = (char)(i % 256);
        }
    }
    
    return msg;
}

// 带宽测试生产者线程
void* bandwidth_producer(void *arg) {
    test_params_t *params = (test_params_t*)arg;
    thread_safe_queue_t *tsq = params->queue;
    
    printf("Producer thread starting: sending %d messages of size %d bytes\n", 
           params->message_count, params->message_size);
    
    for (int i = 0; i < params->message_count; i++) {
        message_t *msg = create_message(i, params->message_size);
        if (msg) {
            if (send_message(tsq, msg) != 0) {
                printf("Producer: Failed to send message %d\n", i);
                free_message(msg);
                free(msg);
                break;
            }
            free_message(msg);
            free(msg);
        } else {
            printf("Producer: Failed to create message %d\n", i);
            break;
        }
    }
    
    return NULL;
}

// 带宽测试消费者线程
void* bandwidth_consumer(void *arg) {
    test_params_t *params = (test_params_t*)arg;
    thread_safe_queue_t *tsq = params->queue;
    
    printf("Consumer thread starting: expecting %d messages\n", params->message_count);
    
    int received_count = 0;
    struct timeval start_time, end_time;
    long total_latency_us = 0;
    
    gettimeofday(&start_time, NULL);
    
    while (received_count < params->message_count) {
        message_t msg;
        init_message(&msg);
        
        if (receive_message(tsq, &msg) == 0) {
            // 计算延迟
            gettimeofday(&end_time, NULL);
            if (msg.timestamp.tv_sec > 0) { // 确保时间戳有效
                long latency_us = (end_time.tv_sec - msg.timestamp.tv_sec) * 1000000L +
                                 (end_time.tv_usec - msg.timestamp.tv_usec);
                if (latency_us > 0) total_latency_us += latency_us;
            }
            
            free_message(&msg);
            received_count++;
            
            if (received_count % 1000 == 0) {
                printf("Consumer: Received %d messages\n", received_count);
            }
        } else {
            printf("Consumer: Queue closed or error after receiving %d messages\n", received_count);
            break;
        }
    }
    
    printf("Consumer: Total received %d messages\n", received_count);
    return NULL;
}

// 简化的带宽测试函数
void run_bandwidth_test(int message_size, int message_count, int num_threads) {
    thread_safe_queue_t tsq;
    if (init_queue(&tsq, DEFAULT_QUEUE_SIZE) != 0) {
        printf("Failed to initialize queue\n");
        return;
    }
    
    printf("\n=== Bandwidth Test ===\n");
    printf("Message size: %d bytes\n", message_size);
    printf("Message count: %d\n", message_count);
    printf("Number of threads: %d\n", num_threads);
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    pthread_t *producers = malloc(num_threads * sizeof(pthread_t));
    pthread_t *consumers = malloc(num_threads * sizeof(pthread_t));
    
    test_params_t params;
    params.message_size = message_size;
    params.message_count = message_count / num_threads; // 每个线程处理的消息数
    params.num_threads = num_threads;
    params.queue_size = DEFAULT_QUEUE_SIZE;
    params.queue = &tsq;  // 正确传递队列指针
    
    // 创建生产者线程
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&producers[i], NULL, bandwidth_producer, &params) != 0) {
            printf("Failed to create producer thread %d\n", i);
        }
    }
    
    // 创建消费者线程
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&consumers[i], NULL, bandwidth_consumer, &params) != 0) {
            printf("Failed to create consumer thread %d\n", i);
        }
    }
    
    // 等待所有线程完成
    for (int i = 0; i < num_threads; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }
    
    gettimeofday(&end_time, NULL);
    
    // 计算统计信息
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                         (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    
    long total_bytes = (long)message_count * message_size;
    double throughput_mbps = (total_bytes * 8.0) / (elapsed_time * 1024 * 1024);
    
    printf("Time elapsed: %.3f seconds\n", elapsed_time);
    printf("Total bytes transferred: %ld\n", total_bytes);
    printf("Throughput: %.2f Mbps\n", throughput_mbps);
    printf("Throughput: %.2f MB/s\n", total_bytes / (elapsed_time * 1024 * 1024));
    
    free(producers);
    free(consumers);
    destroy_queue(&tsq);
}

// 高负载测试
void run_stress_test() {
    printf("\n=== Stress Test ===\n");
    
    // 测试不同消息大小的性能
    int message_sizes[] = {64, 256, MAX_MESSAGE_SIZE}; // 使用MAX_MESSAGE_SIZE
    int sizes_count = sizeof(message_sizes) / sizeof(message_sizes[0]);
    
    for (int i = 0; i < sizes_count; i++) {
        if (message_sizes[i] > MAX_MESSAGE_SIZE) continue; // 确保不超过最大限制
        
        printf("\n--- Testing with message size: %d bytes ---\n", message_sizes[i]);
        
        // 小数据量测试
        run_bandwidth_test(message_sizes[i], 1000, 1);
        
        // 中等数据量测试
        run_bandwidth_test(message_sizes[i], 5000, 2);
    }
}

// 延迟测试
void run_latency_test() {
    printf("\n=== Latency Test ===\n");
    
    thread_safe_queue_t tsq;
    if (init_queue(&tsq, DEFAULT_QUEUE_SIZE) != 0) {
        printf("Failed to initialize queue\n");
        return;
    }
    
    const int test_count = 10000;
    struct timeval start_time, end_time;
    long total_latency_us = 0;
    int successful_transfers = 0;
    
    printf("Testing message latency with %d messages of %d bytes...\n", test_count, 256);
    
    gettimeofday(&start_time, NULL);
    
    for (int i = 0; i < test_count; i++) {
        message_t *msg = create_message(i, 256); // 256字节消息
        if (msg) {
            struct timeval send_time;
            gettimeofday(&send_time, NULL);
            msg->timestamp = send_time;
            
            if (send_message(&tsq, msg) == 0) {
                message_t received_msg;
                init_message(&received_msg);
                
                if (receive_message(&tsq, &received_msg) == 0) {
                    struct timeval receive_time;
                    gettimeofday(&receive_time, NULL);
                    
                    long latency_us = (receive_time.tv_sec - received_msg.timestamp.tv_sec) * 1000000L +
                                     (receive_time.tv_usec - received_msg.timestamp.tv_usec);
                    if (latency_us > 0) {  // 确保延迟值有效
                        total_latency_us += latency_us;
                        successful_transfers++;
                    }
                    
                    free_message(&received_msg);
                }
            }
            free_message(msg);
            free(msg);
        }
    }
    
    double avg_latency = successful_transfers > 0 ? (double)total_latency_us / successful_transfers : 0;
    
    printf("Average message latency: %.2f microseconds\n", avg_latency);
    printf("Successful transfers: %d/%d\n", successful_transfers, test_count);
    
    destroy_queue(&tsq);
}

int iccmsg_queue_test() {
    printf("Linux Thread-Safe Message Queue Bandwidth Test\n");
    printf("==============================================\n");
    printf("MAX_MESSAGE_SIZE set to: %d bytes\n", MAX_MESSAGE_SIZE);
    
    // 基础带宽测试
    run_bandwidth_test(MAX_MESSAGE_SIZE, 10000, 1);  // 实际会限制为492字节
    
    // 多线程带宽测试
    run_bandwidth_test(MAX_MESSAGE_SIZE, 10000, 2);  // 实际会限制为492字节
    
    // 测试正好等于最大值的消息
    run_bandwidth_test(MAX_MESSAGE_SIZE, 5000, 1);
    
    // 延迟测试
    run_latency_test();
    
    // 压力测试
    run_stress_test();
    
    printf("\n=== Summary ===\n");
    printf("The message queue implementation supports:\n");
    printf("- Thread-safe operations\n");
    printf("- Configurable message sizes (max %d bytes)\n", MAX_MESSAGE_SIZE);
    printf("- Memory-safe message handling\n");
    printf("- Bandwidth and latency measurements\n");
    
    return 0;
}



