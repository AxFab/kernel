#if 0

typedef struct net_hostnode net_host_t;
struct net_hostnode {
  char mac_address_[6];
  char ipv4_address_[4];
  char ipv6_address_[4];
  char hostname_[255];
  char domain_[255];
  int type_; // A - MX ...
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
// Eternet - Layer 2

struct net_ether_header {
  unsigned char destination_mac_[6];
  unsigned char source_mac_[6];
  unsigned short ether_type_;
};

#define ETHER_IPV4 0x0800
#define ETHER_ARP 0x0806
#define ETHER_IPV6 0x86DD

socket_t *Network_Eternet_open(net_host_t *rcpt, unsigned short type) {
  socket_t *socket =
  // max_length = 1500 - 14;
  socket_write(socket, rcpt->mac_address_, 6);
  socket_write(socket, __NET.mac_address_, 6);
  socket_write(socket, &type, 2);
  return socket;
}

int Network_Eternet_send(socket_t *socket) {
  size_t lg = socket_length(socket);
  int checksum = 0;
  socket_seek(socket, 14);
  // Read + checksum

  socket_write(socket, checksum, 4);
  return 0;
}

int Network_Eternet_receive(socket_t *socket) {
  struct net_ether_header header;
  socket_read(socket, &header, sizeof(header));
  memset(socket->mac_address_, header.source_mac_, 6);
  switch(_be_to_cup16(header.ether_type_)) {
    case ETHER_ARP:
      return Network_ARP_receive(socket);
    case ETHER_IPV4:
      return Network_IPV4_receive(socket);
    case ETHER_IPV6:
      return Network_IPV6_receive(socket);
  }
  return -1;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
// ARP (Address resolution protocol) - Layer 3

struct net_arpipv4_header {
  short htype_;
  short ptype_;
  char hlen_;
  char plen_;
  short oper_;
};


#define ARP_OPER_REQUEST 1
#define ARP_OPER_RESPONSE 2

socket_t *Network_ARP_IPV4_request(char *ipv4_wanted) {
  const unsigned char mac_brodcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  socket_t *socket = Network_open_ethernet(mac_brodcast, ETHER_ARP);
  struct net_arpipv4_header header;
  header.htype_ = _cpu_to_be16(1)
  header.ptype_ = _cpu_to_be16(ETHER_IPV4);
  header.hlen_ = 6;
  header.plen_ = 4;
  header.oper_ = _cpu_to_be16(ARP_OPER_REQUEST);
  socket_write(socket, &header, sizeof(header));
  socket_write(socket, __NET.mac_address_, 6);
  socket_write(socket, __NET.ipv4_address_, 4);
  socket_write(socket, __NET.mac_address_, 6);
  socket_write(socket, ipv4_wanted, 4);
  // TODO - Push on ARP stack, we are looking for this IP + timestamp.
  Network_send_ethernet(socket);
  return socket;
}

socket_t *Network_ARP_IPV6_request(char *ipv6_wanted) {
  const unsigned char mac_brodcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  socket_t *socket = Network_open_ethernet(mac_brodcast, ETHER_ARP);
  struct net_arpipv4_header header;
  header.htype_ = _cpu_to_be16(1)
  header.ptype_ = _cpu_to_be16(ETHER_IPV4);
  header.hlen_ = 6;
  header.plen_ = 4;
  header.oper_ = _cpu_to_be16(ARP_OPER_REQUEST);
  socket_write(socket, &header, sizeof(header));
  socket_write(socket, __NET.mac_address_, 6);
  socket_write(socket, __NET.ipv4_address_, 4);
  socket_write(socket, __NET.mac_address_, 6);
  socket_write(socket, ipv6_wanted, 4);
  // TODO - Push on ARP stack, we are looking for this IP + timestamp.
  Network_send_ethernet(socket);
  return socket;
}

socket_t *Network_ARP_IPV4_reply(char *ipv4_wanted) {
  char hwd_buffer[6];
  // TODO - Get the MAC address
  socket_t *socket = Network_open_ethernet(hwd_buffer, ETHER_ARP);
  struct net_arpipv4_header header;
  header.htype_ = _cpu_to_be16(1)
  header.ptype_ = _cpu_to_be16(ETHER_IPV4);
  header.hlen_ = 6;
  header.plen_ = 4;
  header.oper_ = _cpu_to_be16(ARP_OPER_RESPONSE);
  socket_write(socket, &header, sizeof(header));
  socket_write(socket, __NET.mac_address_, 6);
  socket_write(socket, __NET.ipv4_address_, 4);
  socket_write(socket, hwd_buffer, 6);
  socket_write(socket, ipv4_wanted, 4);
  Network_send_ethernet(socket);
  return socket;
}

socket_t *Network_ARP_IPV6_reply(char *ipv6_wanted) {
  char hwd_buffer[6];
  // TODO - Get the MAC address
  socket_t *socket = Network_open_ethernet(hwd_buffer, ETHER_ARP);
  struct net_arpipv4_header header;
  header.htype_ = _cpu_to_be16(1)
  header.ptype_ = _cpu_to_be16(ETHER_IPV6);
  header.hlen_ = 6;
  header.plen_ = 8;
  header.oper_ = _cpu_to_be16(ARP_OPER_RESPONSE);
  socket_write(socket, &header, sizeof(header));
  socket_write(socket, __NET.mac_address_, 6);
  socket_write(socket, __NET.ipv6_address_, 8);
  socket_write(socket, hwd_buffer, 6);
  socket_write(socket, ipv6_wanted, 8);
  Network_send_ethernet(socket);
  return socket;
}

int Network_ARP_receive(socket_t *socket) {
  char hwd_buffer[6];
  char pcl_buffer[8];
  int lg;
  struct net_arpipv4_header header;
  socket_read(socket, &header, sizeof(header));
  switch(_be_to_cup16(header.ptype_)) {
    case ETHER_IPV4:
      lg = 4;
      break;
    case ETHER_IPV6:
      lg = 6;
      break;
    default:
      return -1;
  }
  socket_read(socket, &hwd_buffer, 6);
  socket_read(socket, &pcl_buffer, lg);
  // TODO - Push on ARP cache, check if we asked for it or not + timestamp
  if (header.oper_ == _cpu_to_be16(ARP_OPER_REQUEST)) {
    socket_read(socket, &hwd_buffer, 6);
    socket_read(socket, &pcl_buffer, lg);
    // TODO - If we know this IP address, reply...
  }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
// IP (Internet protocol) - Layer 3

struct net_ipv4_header {
  int version_:4;
  int header_length_:4;
  int dscp_:6;
  int ecn_:2;
  short length_;
  short identifer_;
  short flags_:3;
  short frag_offset:13;
  char ttl_;
  char protocol_;
  short header_checksum_;
  unsigned int source_ip_;
  unsigned int destination_ip_;
}

#define IP_PTCL_ICMP 1
#define IP_PTCL_IGMP 2
#define IP_PTCL_TCP 6
#define IP_PTCL_UDP 17
#define IP_PTCL_ENCAP 41
#define IP_PTCL_OSPF 89
#define IP_PTCL_SCTP 132

#define IP_TTL 64

socket_t *Network_IP_open(net_host_t *rcpt, int length, int id, int protocol, size_t offset, size_t length) {
  struct net_ipv4_header header;
  if (length + sizeof(header) > socket->max_length) {
    return -1;
  }
  header.version_ = 4;
  header.header_length_ = sizeof(header) / 4;
  header.length_ = length + sizeof(header);
  header.identifer_ = id;
  header.flags_ = fragmt_count == 1 ? 2 : (fragmt + 1 == fragmt_count ? 0 : 4);
  header.frag_offset = fragmt;
  header.ttl_ = IP_TTL
  header.protocol_ = protocol;
  // Header checksum
  header.source_ip_ = ;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

  Payload of [46 - 1500 bytes]

  ARP
  IP
    ICMP
    UDP
      DHCP
      DNS
    TCP
      SSL/TLS
        HTTP
        Telnet
#endif