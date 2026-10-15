#include <string.h>
#include <assert.h>
#include <can_proxy_buffer.h>

struct cp_msg_buf_t;

typedef void  *(*cp_msg_buf_create_t)(size_t count, size_t size);
typedef size_t (*cp_msg_buf_post_t)(void *handle, void *data, size_t len);
typedef size_t (*cp_msg_buf_get_t)(void *handle, void *data, size_t len);

typedef struct cp_msg_buf_t {
    void *send_handle;
    void *recv_handle;
    size_t size;
    size_t count;
    cp_msg_buf_create_t create;
    cp_msg_buf_post_t post;
    cp_msg_buf_post_t get;
} cp_msg_buf_t;

size_t os_msg_buf_post(void *handle, void *data, size_t len);
size_t os_msg_buf_get(void *handle, void *data, size_t len);
void *os_msg_buf_create(size_t buffer_count, size_t buffer_size);

static cp_msg_buf_t interface = {
    .send_handle = NULL,
    .recv_handle = NULL,
    .size = CP_BUFFER_SIZE,
    .count = CP_BUFFER_COUNT,
    .create = os_msg_buf_create,
    .post = os_msg_buf_post,
    .get = os_msg_buf_get,
};

bool can_proxy_buffer_init(void)
{
    interface.send_handle = interface.create(interface.count, interface.size);
    interface.recv_handle = interface.create(interface.count, interface.size);

    if (interface.send_handle && interface.recv_handle) {
        return true;
    }

    return false;
}

void *cp_find_handle(int dst)
{
    return (dst == DST_AP) ? interface.send_handle : interface.recv_handle;
}

/* interface frame interface post */
bool cp_if_frame_post(cp_if_frame_t *frame, int dst)
{
    void *handle = cp_find_handle(dst);
    return interface.post(handle, frame,
                          cp_if_get_frame_size(frame)) ? true : false;
}

/* interface frame interface get */
size_t cp_if_frame_get(cp_if_frame_t *frame, int dst, size_t max_data_len)
{
    void *handle = cp_find_handle(dst);
    if (!interface.get(handle, frame,
                       max_data_len + sizeof(cp_if_frame_t))) {
        return 0;
    }
    else {
        return frame->len;
    }
}