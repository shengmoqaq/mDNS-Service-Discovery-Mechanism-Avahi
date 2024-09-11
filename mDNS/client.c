#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 标志位，用于确保只打印一次服务信息
int service_printed = 0;

int ipv4_printed = 0;   // IPv4 解析完成标志
int ipv6_printed = 0;   // IPv6 解析完成标志

void resolve_callback(AvahiServiceResolver *resolver, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event,
    const char *name, const char *type, const char *domain, const char *hostname,
    const AvahiAddress *address, uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags, void *userdata) {
    if(event == AVAHI_RESOLVER_FOUND) {
        char address_str[AVAHI_ADDRESS_STR_MAX];
        avahi_address_snprint(address_str, sizeof(address_str), address);

        if(protocol == AVAHI_PROTO_INET && !ipv4_printed) { 
            printf("Service Hostname: %s\n", hostname);
            printf("Resolved IPv4 IP Address: %s\n", address_str);
            ipv4_printed = 1;
        } 
        else if(protocol == AVAHI_PROTO_INET6 && !ipv6_printed) {
            printf("Resolved IPv6 IP Address: %s\n", address_str);
            ipv6_printed = 1;
        }

        // 只有在两种地址都打印后，才打印服务信息并设置标志位
        if(ipv4_printed && ipv6_printed && !service_printed) {
            printf("Service Name: %s\n", name);
            printf("Service Type: %s\n", type);

            while(txt) {
                char *key, *value;
                avahi_string_list_get_pair(txt, &key, &value, NULL);
                if(strcmp(key, "mqtt_server_address") == 0) {
                    printf("Service Address (from TXT record): %s\n", value);
                }
                avahi_free(key);
                avahi_free(value);
                txt = avahi_string_list_get_next(txt);
            }
            service_printed = 1;
        }
    }

    avahi_service_resolver_free(resolver);
}

void browse_callback(AvahiServiceBrowser *browser, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event,
    const char *name, const char *type, const char *domain, AvahiLookupResultFlags flags, void *userdata) {
    AvahiClient *client = userdata;
    if(event == AVAHI_BROWSER_NEW) {
        avahi_service_resolver_new(client, interface, AVAHI_PROTO_INET, name, type, domain, AVAHI_PROTO_UNSPEC, 0, resolve_callback, client);
        avahi_service_resolver_new(client, interface, AVAHI_PROTO_INET6, name, type, domain, AVAHI_PROTO_UNSPEC, 0, resolve_callback, client);
    }
}


int main() {
    AvahiThreadedPoll *poll = avahi_threaded_poll_new();
    if(!poll) {
        fprintf(stderr, "Failed to create threaded poll.\n");
        return EXIT_FAILURE;
    }

    AvahiClient *client = avahi_client_new(avahi_threaded_poll_get(poll), AVAHI_CLIENT_NO_FAIL, NULL, NULL, NULL);
    if(!client) {
        fprintf(stderr, "Failed to create Avahi client.\n");
        avahi_threaded_poll_free(poll);
        return EXIT_FAILURE;
    }

    AvahiServiceBrowser *browser = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_mqtt._tcp", NULL, 0, browse_callback, client);
    if(!browser) {
        fprintf(stderr, "Failed to create service browser.\n");
        avahi_client_free(client);
        avahi_threaded_poll_free(poll);
        return EXIT_FAILURE;
    }

    printf("Service browser started.\n");

    avahi_threaded_poll_start(poll);

    while(1) {
        pause();
    }

    avahi_service_browser_free(browser);
    avahi_client_free(client);
    avahi_threaded_poll_free(poll);

    return EXIT_SUCCESS;
}
