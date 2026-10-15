#include "iccmsg_common.h"
#include "iccmsg_ll.h"

/*
    查找rpmsg管理节点,由R核创建
*/
int iccmsg_find_name(const char *name)
{
    int ret = 0;
    char buf[256];
    snprintf(buf, sizeof(buf), "ls -1 /sys/bus/rpmsg/devices | grep %s", name);
    ret = system(buf);
    return ret;
}

/**
 * @brief 判断RPMSG节点的类型（ipcc/virtio/未知）
 * @param node_name 节点名称
 * @return 节点类型枚举值
 */
static rpmsg_type_t get_rpmsg_node_type(const char *node_name)
{
    if (strstr(node_name, "soc:ipcc@")) {
        return RPMSG_TYPE_IPCC;
    } else if (strstr(node_name, "virtio")) {
        return RPMSG_TYPE_VIRTIO;
    } else {
        return RPMSG_TYPE_UNKNOWN;
    }
}


// 函数：从RPMSG节点名称中解析ept值
// 返回：成功返回ept值，失败返回-1
static int parse_ept_from_name(const char *node_name)
{
    char *name_copy = strdup(node_name);
    if (!name_copy) {
        LOG_PRINT(LOG_ERR,"strdup failed.\n");
        return -1;
    }

    char *last_dot = strrchr(name_copy, '.');
    int ept = -1;
    if (last_dot) {
        char *ept_str = last_dot + 1;
        char *endptr;
        // 使用strtol转换，检查是否为有效数字
        errno = 0;
        long ept_long = strtol(ept_str, &endptr, 10);
        if (errno == 0 && endptr != ept_str && *endptr == '\0') {
            ept = (int)ept_long;
        }
    }

    free(name_copy);
    return ept;
}

/**
 * @brief 查找指定ept值和类型的RPMSG节点，返回首个匹配的节点路径
 * @param target_ept 目标ept值（如500、30）
 * @param type_filter 类型过滤（RPMSG_TYPE_UNKNOWN表示不过滤类型）
 * @return 成功返回动态分配的节点路径字符串（需手动free），失败返回NULL
 */
static char *find_rpmsg_node_by_ept(int target_ept, rpmsg_type_t type_filter)
{
    // 打开RPMSG设备目录
    DIR *dir = opendir(RPMSG_DEVICES_DIR);
    if (!dir) {
        LOG_PRINT(LOG_ERR,"opendir %s failed.\n", RPMSG_DEVICES_DIR);
        return NULL;
    }

    struct dirent *entry;
    char *node_path = NULL;

    // 遍历目录中的所有项
    while ((entry = readdir(dir)) != NULL) {
        // 跳过.和..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 1. 解析当前节点的ept值，不匹配则跳过
        int ept = parse_ept_from_name(entry->d_name);
        if (ept != target_ept) {
            continue;
        }

        // 2. 判断节点类型，若指定了过滤类型则检查匹配
        rpmsg_type_t node_type = get_rpmsg_node_type(entry->d_name);
        if (type_filter != RPMSG_TYPE_UNKNOWN && node_type != type_filter) {
            continue;
        }

        // 3. 找到匹配节点，拼接完整路径
        node_path = (char *)malloc(32);
        if (!node_path) {
            LOG_PRINT(LOG_ERR,"malloc failed.\n", RPMSG_DEVICES_DIR);
            closedir(dir);
            return NULL;
        }
        snprintf(node_path, 32, "%s", entry->d_name);
        break;  // 找到首个匹配节点后退出遍历
    }

    // 关闭目录句柄
    closedir(dir);

    // 未找到匹配节点时返回NULL
    return node_path;
}

/**
 * @brief   iccmsg 底层rpmsg初始化函数，绑定远端节点，生成本地节点，本地节点ept数值 = 远端节点ept数值+1
 * @param   iccmsg_ll: iccmsg 硬件实例
 * @param   remote_ept: 管理节点endpoint
 * @return  0: 初始化成功
 */
int iccmsg_ll_init(iccmsg_ll_t *iccmsg_ll, int remote_ept, char *app_name)
{

    // 依据remote_ept 查找远端ipcc与virtio节点名称: soc:ipcc@1.ipcc-echo.-1.30 virtio0.rpmsg-echo.-1.30
    iccmsg_ll->remote_ipcc_node = find_rpmsg_node_by_ept(remote_ept,RPMSG_TYPE_IPCC);
    iccmsg_ll->remote_virtio_node = find_rpmsg_node_by_ept(remote_ept,RPMSG_TYPE_VIRTIO);
    if(iccmsg_ll->remote_ipcc_node == NULL || iccmsg_ll->remote_virtio_node == NULL){
        LOG_PRINT(LOG_ERR, "Can't find remote endpoint\n");
        return -1;
    }

    LOG_PRINT(LOG_DEBUG, "Remote endpoint: %s, %s\n",iccmsg_ll->remote_ipcc_node,iccmsg_ll->remote_virtio_node);

    if (rpch_check_path(iccmsg_ll->remote_ipcc_node) || rpch_check_path(iccmsg_ll->remote_virtio_node)) {
        LOG_PRINT(LOG_ERR, "rpch_check_path failed.\n");
        return -EINVAL;
    }
    
    // ept信息记录，本地ept默认使用远端ept+1
    iccmsg_ll->remote_ept = remote_ept;
    iccmsg_ll->local_ept_info.src = remote_ept+1;
    iccmsg_ll->local_ept_info.dst = 0xFFFFFFFF;
    if(NULL != app_name)
        snprintf(iccmsg_ll->local_ept_info.name, sizeof(iccmsg_ll->local_ept_info.name), "%s", app_name);
    else
        snprintf(iccmsg_ll->local_ept_info.name, sizeof(iccmsg_ll->local_ept_info.name), "%s", DEFAULT_LOCAL_EPT_NAME);

    // 创建字符设备节点
    LOG_PRINT(LOG_DEBUG, "rpch_create ipcc...\n");
    iccmsg_ll->charfd_ipcc = rpch_create(iccmsg_ll->ipcc_dev_path, iccmsg_ll->remote_ipcc_node, iccmsg_ll->local_ept_info);
    if (iccmsg_ll->charfd_ipcc < 0) {
        LOG_PRINT(LOG_ERR, "rpch_create ipcc point failed.\n");
        return -EINVAL;
    }
    LOG_PRINT(LOG_DEBUG, "rpch_create : %s.\n",iccmsg_ll->ipcc_dev_path);
    LOG_PRINT(LOG_DEBUG, "rpch_create virtio...\n");
    iccmsg_ll->charfd_virtio = rpch_create(iccmsg_ll->virtio_dev_path, iccmsg_ll->remote_virtio_node, iccmsg_ll->local_ept_info);
    if (iccmsg_ll->charfd_virtio < 0) {
        LOG_PRINT(LOG_ERR, "rpch_create virtio point failed.\n");
        return -EINVAL;
    }
    LOG_PRINT(LOG_DEBUG, "rpch_create : %s.\n",iccmsg_ll->virtio_dev_path);

    // 打开本地字符设备节点获取文件描述符
    iccmsg_ll->fd_ipcc = open(iccmsg_ll->ipcc_dev_path, O_RDWR | O_NONBLOCK );
    if (iccmsg_ll->fd_ipcc < 0) {
        perror("Failed to open rpmsg device.");
        close(iccmsg_ll->charfd_ipcc);
        return -1;
    }
    iccmsg_ll->fd_virtio = open(iccmsg_ll->virtio_dev_path, O_RDWR | O_NONBLOCK );
    if (iccmsg_ll->fd_virtio < 0) {
        perror("Failed to open rpmsg device.");
        close(iccmsg_ll->charfd_virtio);
        return -1;
    }
    LOG_PRINT(LOG_DEBUG, "iccmsg_ll_init info:%s:fd_ipcc = %d, fd_virtio = %d.\n",app_name, iccmsg_ll->fd_ipcc, iccmsg_ll->fd_virtio);
    return 0;
}

int iccmsg_ll_deinit(iccmsg_ll_t *iccmsg_ll)
{
    int ret = 0;
    //close all fd
    close(iccmsg_ll->fd_virtio);
    close(iccmsg_ll->fd_ipcc);
    close(iccmsg_ll->charfd_virtio);
    close(iccmsg_ll->charfd_ipcc);
    ret = unbind_rpmsg_chrdev(iccmsg_ll->remote_ipcc_node);
    if(ret)
        LOG_PRINT(LOG_ERR, "unbind_rpmsg_chrdev ipcc_node failed.\n");

    ret = unbind_rpmsg_chrdev(iccmsg_ll->remote_virtio_node);
    if(ret)
        LOG_PRINT(LOG_ERR, "unbind_rpmsg_chrdev virtio_node failed.\n");

    return ret;
}

