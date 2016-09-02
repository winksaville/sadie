/*
 * copyright 2016 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

//#define NDEBUG

#include <ac_inet_link.h>
#include <ac_inet_hton.h>
#include <ac_inet_ntoh.h>
#include <ac_arp.h>
#include <ac_ether.h>

#include <ac_assert.h>
#include <ac_comp_mgr.h>
#include <ac_debug_printf.h>
#include <ac_printf.h>
#include <ac_memset.h>
#include <ac_memcpy.h>
#include <ac_msg.h>
#include <ac_msg_pool.h>
#include <ac_string.h>
#include <ac_status.h>

#if 1
//#include <arpa/inet.h>

//#include <net/ethernet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#endif

typedef struct {
  AcComp comp;
  ac_u32 a_u32;
  AcInt fd;             ///< File descriptor for the interface
  char* ifname[2];      ///< Name of interfaces to try
  AcUint ifindex;       ///< Index of the interface
} AcCompIpv4LinkLayer;

#if 1
/**
 * Display memory
 */
void ac_print_mem(char* leader, void *mem, int len, char* format, char sep, char* trailer) {
  if (leader != AC_NULL) {
    ac_printf(leader);
  }
  unsigned char* p = (unsigned char*)mem;

  for (int i = 0; i < len; i++) {
    if (i != 0) ac_printf("%c", sep);
    ac_printf(format, p[i]);
  }

  if (trailer != AC_NULL) {
    ac_printf(trailer);
  }
}

/**
 * Display hex memory
 */
void ac_println_hex(char* leader, void *mem, int len, char sep) {
  ac_print_mem(leader, mem, len, "%02x", sep, "\n");
}

/**
 * Display hex memory
 */
void ac_println_dec(char* leader, void *mem, int len, char sep) {
  ac_print_mem(leader, mem, len, "%d", sep, "\n");
}

void  ac_println_sockaddr_ll(char* leader, struct sockaddr_ll* addr) {
  if (leader != AC_NULL) {
    ac_printf(leader);
  }
  ac_printf("family=%0d protocol=0x%0x ifindex=%d hatype=%d pkttype=%d halen=%d addr=",
      addr->sll_family, AC_NTOH_U16(addr->sll_protocol), addr->sll_ifindex,
      addr->sll_hatype, addr->sll_pkttype, addr->sll_halen);
  ac_println_hex(AC_NULL, addr->sll_addr, addr->sll_halen, ':');
}

/**
 * Set ifr_name
 *
 * @return 0 if OK
 */
AcStatus set_ifname(struct ifreq* ifr, const char* ifname) {
  AcStatus status;

  // Copy ifname to ifr.ifr_name if it fits
  if (ac_strlen(ifname) >= (sizeof(ifr->ifr_name) - 1)) {
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }
  ac_strcpy(ifr->ifr_name, ifname);

  status = AC_STATUS_OK;

done:
  return status;
}

/**
 * Get index for ifname
 *
 * @return 0 if OK
 */
AcStatus get_ifindex(AcInt fd, const char* ifname, AcUint* ifindex) {
  AcStatus status;
  struct ifreq ifr;

  status = set_ifname(&ifr, ifname);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Issue iotcl to get the index
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    status = AC_STATUS_ERR;
  }

  *ifindex = ifr.ifr_ifindex;

done:
  return status;
}

/**
 * init a struct ethhdr
 *
 * @param dst_addr
 */
void init_ethhdr(struct ethhdr* pEthHdr, const void *dst_addr, const void* src_addr, int ll_protocol) {
  ac_memcpy(pEthHdr->h_dest, dst_addr, AC_ETHER_ADDR_LEN);
  ac_memcpy(pEthHdr->h_source, src_addr, AC_ETHER_ADDR_LEN);
  pEthHdr->h_proto = ll_protocol;
}

/**
 * Init sockaddr_ll
 */
void init_sockaddr_ll(struct sockaddr_ll* pSockAddrLl,
    const unsigned char ethernet_addr[AC_ETHER_ADDR_LEN],
    const int ifindex,
    const int protocol) {
  ac_memset(pSockAddrLl, 0, sizeof(struct sockaddr_ll));

  pSockAddrLl->sll_family = AF_PACKET;
  pSockAddrLl->sll_protocol = AC_HTON_U16(protocol);
  pSockAddrLl->sll_ifindex = ifindex;
  pSockAddrLl->sll_halen = AC_ETHER_ADDR_LEN;
  ac_memcpy(pSockAddrLl->sll_addr, ethernet_addr, AC_ETHER_ADDR_LEN);
}

/**
 * Init sockaddr_ll
 */
void init_broadcast_sockaddr_ll(struct sockaddr_ll* pSockAddrLl,
    const int ifindex,
    const int protocol) {
  unsigned char ethernet_broadcast_addr[AC_ETHER_ADDR_LEN] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };

  init_sockaddr_ll(pSockAddrLl, ethernet_broadcast_addr, ifindex, AC_ETHER_PROTO_ARP);
}
#endif

static void send_error_rsp(AcComp* comp, AcMsg* msg, AcStatus status) {
}

static ac_bool comp_ipv4_ll_process_msg(AcComp* comp, AcMsg* msg) {
  AcStatus status;
  AcCompIpv4LinkLayer* this = (AcCompIpv4LinkLayer*)comp;
  AC_UNUSED(this);

  ac_debug_printf("%s:+msg->op=%lx\n", this->comp.name, msg->op);

  switch (msg->op) {
    case (AC_INIT_CMD): {
      ac_debug_printf("%s: AC_INIT_CMD ifname=%s\n", this->comp.name, this->ifname[0]);
#if 1
      // Open an AF_PACKET socket
      this->fd = socket(AF_PACKET, SOCK_RAW, AC_HTON_U16(ETH_P_ALL));
      if (this->fd < 0) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could not open SOCK_RAW errno=%u\n", this->comp.name, errno);
        ac_fail((char*)str);
      }

      // Get the interface index
      status = get_ifindex(this->fd, this->ifname[0], &this->ifindex);
      if (status != AC_STATUS_OK) {
        ac_printf("%s: Could not get interface index for ifname=%s errno=%d\n", this->comp.name, this->ifname[0], errno);
        status = get_ifindex(this->fd, this->ifname[1], &this->ifindex);
        if (status != AC_STATUS_OK) {
          AcU8 str[256];
          ac_snprintf(str, sizeof(str),
                "%s: Could not get interface index for ifname=%s errno=%d\n", this->comp.name, this->ifname[1], errno);
          ac_fail((char*)str);
        }
      }
#endif
      break;
    }
    case (AC_DEINIT_CMD): {
      ac_debug_printf("%s: AC_DEINIT_CMD\n", this->comp.name);
      break;
    }
    case (AC_INET_SEND_ARP_CMD): {
      AcInetSendArpExtra* send_arp_extra = (AcInetSendArpExtra*)msg->extra;
      ac_printf("%s: AC_INET_SEND_ARP_CMD proto=%x", this->comp.name, send_arp_extra->proto);
      ac_println_dec(" proto_addr=", send_arp_extra->proto_addr, send_arp_extra->proto_addr_len, '.');
      break;
    }
    case (AC_INET_SEND_PACKET_CMD): {
      ac_debug_printf("%s: AC_INET_SEND_PACKET_CMD\n", this->comp.name);
      break;
    }
    default: {
      ac_debug_printf("%s: AC_STATUS_UNRECOGNIZED_PROTOCOL send error rsp\n", this->comp.name);
      send_error_rsp(comp, msg, AC_STATUS_UNRECOGNIZED_PROTOCOL);
      break;
    }
  }

  AcMsgPool_ret_msg(msg);

  ac_debug_printf("%s:-msg->op=%lx\n", this->comp.name, msg->op);
  return AC_TRUE;
}

static AcCompIpv4LinkLayer comp_ipv4_ll = {
  .comp.name=(ac_u8*)INET_LINK_COMP_IPV4_NAME,
  .comp.process_msg = comp_ipv4_ll_process_msg,
  .ifname[0] = "eno1",
  .ifname[1] = "eth0"
};

/**
 * see ac_inet_link_internal.h
 */
void AcInetLink_deinit(AcCompMgr* cm) {
  ac_debug_printf("AcInetLink_deinit:+cm=%p\n", cm);

  ac_assert(AcCompMgr_rmv_comp(&comp_ipv4_ll.comp) == AC_STATUS_OK);

  ac_debug_printf("AcInetLink_deinit:-cm=%p\n", cm);
}

/**
 * see ac_inet_link.h
 */
void AcInetLink_init(AcCompMgr* cm) {
  ac_debug_printf("AcInetLink_init:+cm=%p\n", cm);

  ac_assert(AcCompMgr_add_comp(cm, &comp_ipv4_ll.comp) == AC_STATUS_OK);

  ac_debug_printf("AcInetLink_init:-cm=%p\n", cm);
}
