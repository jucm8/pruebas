
#include "zephyr/net/dhcpv4.h"
#include "zephyr/net/net_if.h"
#include "zephyr/net/net_mgmt.h"
#include "zephyr/net/wifi.h"
#include <string.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>
#include "wifi.h"

LOG_MODULE_REGISTER(wifi);

static struct net_mgmt_event_callback net_cb;
void net_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event, struct net_if *iface) {

    LOG_INF("Net handler ejecutado...");
    switch (mgmt_event) {
        case NET_EVENT_IPV4_ADDR_ADD:
            LOG_INF("IP added");
            break;
    }
}

/* Static IP configuration */
void configure_static_ip() {
    LOG_INF("PUNTO 1");
    struct net_if *iface = net_if_get_default();
    if (!iface) {
        LOG_INF("No default network interface");
        return;
    }

    net_dhcpv4_start(iface);
    LOG_INF("Interfaz OK");

    net_mgmt_init_event_callback(&net_cb, net_handler,
        NET_EVENT_IPV4_ADDR_ADD
    );
    net_mgmt_add_event_callback(&net_cb);

    /* struct wifi_connect_req_params wifi_params = { */
    /*     .ssid = WIFI_SSID, */
    /*     .ssid_length = strlen(WIFI_SSID), */
    /*     .psk = WIFI_PASSWORD, */
    /*     .psk_length = strlen(WIFI_PASSWORD), */
    /*     .channel = WIFI_CHANNEL_ANY, */
    /*     .security = WIFI_SECURITY_TYPE_WPA_AUTO_PERSONAL, */
    /* }; */
    /**/
    /* int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, */
                       //sizeof(struct wifi_connect_req_params));
    
    /* if (ret) { */
    /*     LOG_ERR("Wi-Fi connection failed: %d", ret); */
    /* } else { */
    /*     LOG_INF("Wi-Fi connection requested"); */
    /* } */
}
