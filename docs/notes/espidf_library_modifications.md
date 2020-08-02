As part of the move to ESP-IDF, some libraries were modified to get the system to compile.  This is a list of the modifications made.

## ESP32 Core Wifi library (`components/arduino/libraries/WiFi`)

These modifications were taken from [this ESP32 Arduino issue](https://github.com/espressif/arduino-esp32/issues/3760#issuecomment-591145500).

### `components/arduino/libraries/WiFi/src/ETH.cpp`, line 196

change
```c++
ip_addr_t dns_ip = dns_getserver(dns_no);
return IPAddress(dns_ip.u_addr.ip4.addr);
```

to

```c++
const ip_addr_t * dns_ip = dns_getserver(dns_no);
return IPAddress(dns_ip->u_addr.ip4.addr);
```

### `components/arduino/libraries/WiFi/src/WiFiSTA.cpp`, line 491

change

```c++
ip_addr_t dns_ip = dns_getserver(dns_no);
return IPAddress(dns_ip.u_addr.ip4.addr);
```

to

```c++
const ip_addr_t * dns_ip = dns_getserver(dns_no);
return IPAddress(dns_ip->u_addr.ip4.addr);
```
