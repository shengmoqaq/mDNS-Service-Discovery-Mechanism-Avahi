#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

int main() {
    // 创建 AvahiThreadedPoll 实例
    AvahiThreadedPoll *poll = avahi_threaded_poll_new();
    if(!poll) {
        fprintf(stderr, "Failed to create threaded poll.\n");
        return EXIT_FAILURE;
    }

    // 初始化 Avahi 客户端
    AvahiClient *client = avahi_client_new(avahi_threaded_poll_get(poll), AVAHI_CLIENT_NO_FAIL, NULL, NULL, NULL);
    
    if(!client) {
        fprintf(stderr, "Failed to create Avahi client.\n");
        avahi_threaded_poll_free(poll);  // 释放线程池
        return EXIT_FAILURE;
    }

    // 创建服务
    AvahiEntryGroup *group = avahi_entry_group_new(client, NULL, NULL);
    
    // 修改服务名称和类型为 mqtt
    const char *service_name = "MQTTServer"; 
    const char *service_type = "_mqtt._tcp";
    const char *mqtt_server_address = "mqtts://tb.com";

    // 获取本机的 IP 地址
    struct ifaddrs *ifaddr, *ifa;
    char ipv4_addr[INET_ADDRSTRLEN] = "";
    char ipv6_addr[INET6_ADDRSTRLEN] = "";

    if(getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return EXIT_FAILURE;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL) continue;

        int family = ifa->ifa_addr->sa_family;
        if(family == AF_INET) {
            inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ipv4_addr, INET_ADDRSTRLEN);
        } 
        else if(family == AF_INET6) {
            inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, ipv6_addr, INET6_ADDRSTRLEN);
        }
    }
    freeifaddrs(ifaddr);

    // 创建 TXT 记录
    AvahiStringList *txt = NULL;
    txt = avahi_string_list_add_pair(txt, "mqtt_server_address", mqtt_server_address);
    if(strlen(ipv4_addr) > 0) txt = avahi_string_list_add_pair(txt, "ipv4", ipv4_addr);
    if(strlen(ipv6_addr) > 0) txt = avahi_string_list_add_pair(txt, "ipv6", ipv6_addr);

    if(avahi_entry_group_add_service_strlst(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, service_name, service_type, NULL, NULL, 0, txt) < 0) {
        fprintf(stderr, "Failed to add service TXT record: %s\n", avahi_strerror(avahi_client_errno(client)));
        avahi_client_free(client);
        avahi_threaded_poll_free(poll);
        return EXIT_FAILURE;
    }

       // 提交服务
    if(avahi_entry_group_commit(group) < 0) {
        fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(avahi_client_errno(client)));
        avahi_client_free(client);
        avahi_threaded_poll_free(poll);  // 释放线程池
        return EXIT_FAILURE;
    }

    // 启动事件循环
    avahi_threaded_poll_start(poll);
    
    // 让程序持续运行
    printf("Service started. Press Ctrl+C to exit.\n");
    while (1) {
        pause();  // 等待信号
    }

    // 释放资源
    avahi_entry_group_free(group);
    avahi_client_free(client);
    avahi_threaded_poll_free(poll);

    return EXIT_SUCCESS;
}

