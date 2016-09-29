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
#include <ac_thread.h>

#define _DEFAULT_SOURCE // Needed for struct ether_arp

#include <errno.h>
#include <string.h>

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#define __USE_GNU // Be sure we get TEMP_FAILURE_RETRY defined
#include <unistd.h>

typedef struct {
  AcComp comp;                        ///< The component
  ac_thread_rslt_t reader_thread_rslt;///< Result of creating the reader thread
  AcInt fd;                           ///< File descriptor for the interface
  AcU32 ifname_idx;                   ///< Index to the current ifname[ifname_idx]
  char* ifname[6];                    ///< Name of interfaces to try
  AcUint ifindex;                     ///< Index of the interface
  AcU8 ifmac_addr[AC_ETHER_ADDR_LEN]; ///< Mac address for this interface
  struct sockaddr_in ifipv4_addr;     ///< IPV4 address for this interface
  AcU8 padding[AC_ETHER_MIN_LEN];     ///< Padding for a packet contents are zero's
} AcCompIpv4LinkLayer;

/**
 * Display memory
 */
void ac_printw_mem(
    ac_writer* writer,    ///< writer
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len_in_elems,   ///< Number of elements to dump
    AcU32 bytes_per_elem, ///< Bytes per element 1, 2, 4, 8
    char* format,         ///< Format string such as %x %d ...
    char* sep,            ///< Seperate between elements
    char* trailer) {      ///< Trailer if AC_NULL no trailer
  if (leader != AC_NULL) {
    ac_printfw(writer, leader);
  }
  unsigned char* p = (unsigned char*)mem;

  for (int i = 0; i < len_in_elems; i++) {
    if ((i != 0) && (sep != AC_NULL)) ac_printf("%s", sep);
    switch(bytes_per_elem) {
      case 8: ac_printfw(writer, format, ((AcU64*)p)[i]); break;
      case 4: ac_printfw(writer, format, ((AcU32*)p)[i]); break;
      case 2: ac_printfw(writer, format, ((AcU16*)p)[i]); break;
      case 1:
      default: ac_printfw(writer, format, ((AcU8*)p)[i]); break;
    }
  }

  if (trailer != AC_NULL) {
    ac_printfw(writer, trailer);
  }
}

/**
 * Display memory
 */
void ac_print_mem(
    char* leader,         ///< leader if AC_NULL no leader
    void *mem,            ///< Address of first memory location
    AcU32 len_in_elems,   ///< Number of elements to dump
    AcU32 bytes_per_elem, ///< Bytes per element 1, 2, 4, 8
    char* format,         ///< Format string such as %x %d ...
    char* sep,            ///< Seperate between elements
    char* trailer) {      ///< Trailer if AC_NULL no trailer
  ac_printw_mem(AcPrintf_get_writer(), leader, mem, len_in_elems, bytes_per_elem,
      format, sep, trailer);
}

/**
 * Display a buffer
 */
void ac_print_buff(AcU8 *p, AcS32 len) {
  while (len > 0) {
    ac_printf("%p: ", p);

    if (len > 8) {
      ac_print_mem(AC_NULL, p, 8, 1, "%02x", " ", "  ");
      len -= 8;
      p += 8;
    }

    AcU32 cnt;
    if (len > 8) {
      cnt = 8;
    } else {
      cnt = len;
    }
    ac_print_mem(AC_NULL, p, cnt, 1, "%02x", " ", "\n");
    len -= cnt;
    p += cnt;
  }
}

/**
 * Display hex memory
 */
void ac_println_hex(char* leader, void *mem, AcU32 len, char* sep) {
  ac_print_mem(leader, mem, len, 1, "%02x", sep, "\n");
}

/**
 * Display hex memory
 */
void ac_println_dec(char* leader, void *mem, AcU32 len, char* sep) {
  ac_print_mem(leader, mem, len, 1, "%d", sep, "\n");
}

void  ac_println_sockaddr_ll(char* leader, struct sockaddr_ll* addr) {
  if (leader != AC_NULL) {
    ac_printf(leader);
  }
  ac_printf("family=%d protocol=0x%x ifindex=%d hatype=%d pkttype=%d halen=%d addr=",
      addr->sll_family, AC_NTOH_U16(addr->sll_protocol), addr->sll_ifindex,
      addr->sll_hatype, addr->sll_pkttype, addr->sll_halen);
  ac_println_hex(AC_NULL, addr->sll_addr, addr->sll_halen, ":");
}

/**
 * Print struct ethhdr
 */
ac_u32 format_proc_ethhdr(ac_writer* writer, const char* type_str, ac_va_list args) {
  struct ethhdr* addr = (struct ethhdr*)ac_va_arg(args, void*);
  ac_u32 consumed = sizeof(void*)/sizeof(ac_uint);

  ac_printw_mem(writer, "{dest=", addr->h_dest, AC_ETHER_ADDR_LEN, 1, "%02x", ":", " ");
  ac_printw_mem(writer, "source=", addr->h_source, AC_ETHER_ADDR_LEN, 1, "%02x", ":", " ");
  ac_printfw(writer, "proto=%d}", addr->h_proto);

  return consumed;
}

/**
 * Print struct sockaddr_ll
 */
ac_u32 format_proc_sockaddr_ll(ac_writer* writer, const char* type_str, ac_va_list args) {
  struct sockaddr_ll* addr = (struct sockaddr_ll*)ac_va_arg(args, void*);
  ac_u32 consumed = sizeof(void*)/sizeof(ac_uint);

  ac_printfw(writer, "{family=%d protocol=0x%x ifindex=%d hatype=%d pkttype=%d halen=%d addr=",
      addr->sll_family, AC_NTOH_U16(addr->sll_protocol), addr->sll_ifindex,
      addr->sll_hatype, addr->sll_pkttype, addr->sll_halen);
  ac_printw_mem(writer, AC_NULL, addr->sll_addr, addr->sll_halen, 1, "%02x", ":", AC_NULL);
  ac_printfw(writer, "}");

  return consumed;
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
    status = AC_STATUS_LINUX_ERR(errno);
    goto done;
  }

  *ifindex = ifr.ifr_ifindex;

  status = AC_STATUS_OK;

done:
  ac_debug_printf("get_ethernet_ipv4_addr:-status=%d.%d %s\n",
      AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
  return status;
}

/**
 * Get MAC address of NIC in network order
 *
 * @return 0 if OK
 */
AcStatus get_ethernet_mac_addr(int fd, const char* ifname, AcU8 mac_addr[AC_ETHER_ADDR_LEN]) {
  AcStatus status;
  struct ifreq ifr;

  status = set_ifname(&ifr, ifname);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Issue iotcl to get the hardware address
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    status = AC_STATUS_LINUX_ERR(errno);
    goto done;
  }

#if 0
  ac_println_hex("ifr.ifr_hwaddr.sa_data=", &ifr.ifr_hwaddr.sa_data, sizeof(ifr.ifr_hwaddr.sa_data), ':');
#endif

  ac_memcpy(mac_addr, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);

  status = AC_STATUS_OK;

done:
  ac_debug_printf("get_ethernet_mac_addr:-status=%d.%d %s\n",
      AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
  return status;
}

/**
 * Get IPV4 address of NIC in network order
 *
 * @return 0 if OK
 */
AcStatus get_ethernet_ipv4_addr(int fd, const char* ifname, struct sockaddr_in* ipv4_addr) {
  AcStatus status;
  struct ifreq ifr;

  status = set_ifname(&ifr, ifname);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Issue iotcl to get the index
  if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    status = AC_STATUS_LINUX_ERR(errno);
    goto done;
  }

  // Be sure address fits
  ac_static_assert(sizeof(*ipv4_addr) <= sizeof(ifr.ifr_addr),
    "struct sockaddr_in larger than ifr.ifr_addr");

  // Get ipv4 address
  ac_memcpy(ipv4_addr, &ifr.ifr_addr, sizeof(*ipv4_addr));

  status = AC_STATUS_OK;

done:
  ac_debug_printf("get_ethernet_ipv4_addr:-status=%d.%d %s\n",
      AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
  return status;
}



/**
 * Initialize a struct ether_arp
 *
 * @param fd is the socket file descriptro
 * @param ifname is the name of the interface
 * @param ipv4_addr_str is a ipv4 dotted decimal address
 * @param pArpReq is the ether_arp to initialize
 *
 * @return 0 if OK
 */
void init_ether_arp(AcCompIpv4LinkLayer* this, struct ether_arp* pArpReq,
    AcU16 protocol, AcU32 proto_addr_len, AcU8* proto_addr) {

  // Initialize ethernet arp request
  pArpReq->arp_hrd = AC_HTON_U16(ARPHRD_ETHER);
  pArpReq->arp_pro = AC_HTON_U16(protocol);
  pArpReq->arp_hln = AC_ETHER_ADDR_LEN;
  pArpReq->arp_pln = proto_addr_len;
  pArpReq->arp_op = AC_HTON_U16(ARPOP_REQUEST);

  // Copy the proto_addr to target protocol address (tpa)
  ac_memcpy(pArpReq->arp_tpa, proto_addr, proto_addr_len);

  // Zero the target hardware address (arp_tha)
  ac_memset(&pArpReq->arp_tha, 0, sizeof(pArpReq->arp_tha));

  // Get Source hardware address to arp source hardware address (arp_sha)
  ac_memcpy(pArpReq->arp_sha, this->ifmac_addr, pArpReq->arp_hln);

  // Get source ipv4 address to arp source protocol address (arp_spa)
  ac_memcpy(pArpReq->arp_spa, &this->ifipv4_addr.sin_addr, AC_IPV4_ADDR_LEN);
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
    const int addr_len,
    const unsigned char* addr,
    const int ifindex,
    const int protocol) {
  ac_memset(pSockAddrLl, 0, sizeof(struct sockaddr_ll));

  pSockAddrLl->sll_family = AF_PACKET;
  pSockAddrLl->sll_protocol = AC_HTON_U16(protocol);
  pSockAddrLl->sll_ifindex = ifindex;
  pSockAddrLl->sll_halen = addr_len;
  ac_memcpy(pSockAddrLl->sll_addr, addr, addr_len);
}

/**
 * Init ethernet broadcast sockaddr_ll
 */
void init_ether_broadcast_sockaddr_ll(struct sockaddr_ll* pSockAddrLl, const int ifindex, const AcU16 protocol) {
  unsigned char ethernet_broadcast_addr[AC_ETHER_ADDR_LEN] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };

  init_sockaddr_ll(pSockAddrLl, AC_ETHER_ADDR_LEN, ethernet_broadcast_addr, ifindex, protocol);
}

AcStatus send_arp(AcCompIpv4LinkLayer* this, AcU16 protocol, AcU32 proto_addr_len, AcU8* proto_addr) {
  ac_printf("%s:+send_arp proto=%u proto_addr_len=%u", this->comp.name, protocol, proto_addr_len);
  ac_println_dec(" proto_addr=", proto_addr, proto_addr_len, ".");

  AcStatus status;
  struct sockaddr_ll dst_addr;

  // The destination is an ethenet broadcast address
  init_ether_broadcast_sockaddr_ll(&dst_addr, this->ifindex, AC_ETHER_PROTO_ARP);
  //ac_println_sockaddr_ll("send_ethernet_arp_ipv4: ", &dst_addr);
  ac_printf("send_ethernet_arp_ipv4: dst_addr=%{sockaddr_ll}\n", &dst_addr);

  // Initialize ethernet arp request
  struct ether_arp arp_req;
  init_ether_arp(this, &arp_req, protocol, proto_addr_len, proto_addr);

  // Initialize ethernet header
  struct ethhdr ether_hdr;
  init_ethhdr(&ether_hdr, &dst_addr, &arp_req.arp_sha, dst_addr.sll_protocol);

  // Initialize iovec for msghdr
  struct iovec iov[3];
  int iovlen = 2;
  iov[0].iov_base = &ether_hdr;
  iov[0].iov_len = sizeof(ether_hdr);
  iov[1].iov_base = &arp_req;
  iov[1].iov_len = sizeof(arp_req);

  int len = sizeof(ether_hdr) + sizeof(arp_req);
  if (len < AC_ETHER_MIN_LEN) {
    // We need to padd out the frame to 60 bytes
    // so add an extr iov;
    iov[iovlen].iov_base = this->padding;
    iov[iovlen].iov_len = AC_ETHER_MIN_LEN - len;
    iovlen += 1;
  }

  // Initialize msghdr
  struct msghdr mh;
  mh.msg_name = &dst_addr;
  mh.msg_namelen = sizeof(dst_addr);
  mh.msg_iov = iov;
  mh.msg_iovlen = iovlen;
  mh.msg_control = NULL;
  mh.msg_controllen = 0;
  mh.msg_flags = 0;

  //println_hex("send_ethernet_arp_ipv4: mh=", &packet, len, ' ');
  int count = sendmsg(this->fd, &mh, 0);
  if (count < 0) {
    status = AC_STATUS_LINUX_ERR(errno);
  } else {
    status = AC_STATUS_OK;
  }
  ac_printf("%s:-send_arp sent count=%d status=%d.%d %s\n", this->comp.name, count,
      AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
  return status;
}

static void send_error_rsp(AcComp* comp, AcMsg* msg, AcStatus status) {
}

void* reader_thread(void* param) {
  AcCompIpv4LinkLayer* this = param;
  ac_debug_printf("%s: reader_thread:+\n", this->comp.name);

  AcStatus status;
  struct sockaddr_ll src_addr;
  struct ethhdr ether_hdr;
  static AcU8 buffer[2048];

  struct iovec iov[2];
  int iovlen = 2;
  iov[0].iov_base = &ether_hdr;
  iov[0].iov_len = sizeof(ether_hdr);
  iov[1].iov_base = &buffer;
  iov[1].iov_len = sizeof(buffer);

  // Initialize msghdr
  struct msghdr mh;
  mh.msg_name = &src_addr;
  mh.msg_namelen = sizeof(src_addr);
  mh.msg_iov = iov;
  mh.msg_iovlen = iovlen;
  mh.msg_control = NULL;
  mh.msg_controllen = 0;
  mh.msg_flags = 0;

  while (AC_TRUE) {
    int count = recvmsg(this->fd, &mh, 0);
    if (count < 0) {
      status = AC_STATUS_LINUX_ERR(errno);
      switch (errno) {
        case EINTR:
          ac_debug_printf("%s: reader_thread: %s, continue\n", this->comp.name, strerror(errno));
          continue;
        case EAGAIN:
        case EBADF:
        case ECONNREFUSED:
        case EFAULT:
        case EINVAL:
        case ENOMEM:
        case ENOTCONN:
        case ENOTSOCK:
        default:
          ac_debug_printf("%s: reader_thread: %s unexpected, goto done\n", this->comp.name, strerror(errno));
          goto done;
          break;
      }
    } else {
      status = AC_STATUS_OK;
      AcS32 len = count - iov[0].iov_len;
      ac_printf("%s: reader_thread len=%d ethhdr=%{ethhdr}\n", this->comp.name, len, &ether_hdr);
#if 0
      AcU8 *p = iov[1].iov_base;
      ac_print_buff(p, len);
#endif
    }
  }

done:
  ac_debug_printf("%s: reader_thread:- status=%d.%d %s\n", this->comp.name,
          AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
  return AC_NULL;
}

static ac_bool comp_ipv4_ll_process_msg(AcComp* comp, AcMsg* msg) {
  AcStatus status;
  AcCompIpv4LinkLayer* this = (AcCompIpv4LinkLayer*)comp;
  AC_UNUSED(this);

  ac_debug_printf("%s:+msg->op=%lx\n", this->comp.name, msg->op);

  switch (msg->op) {
    case (AC_INIT_CMD): {
      ac_debug_printf("%s: AC_INIT_CMD ifname=%s\n", this->comp.name, this->ifname[0]);

      // Open an AF_PACKET socket
      this->fd = socket(AF_PACKET, SOCK_RAW, AC_HTON_U16(ETH_P_ALL));
      ac_printf("%s: fd=%d\n", this->comp.name, this->fd);
      if (this->fd < 0) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could not open SOCK_RAW errno=%u\n", this->comp.name, errno);
        ac_fail((char*)str);
      }

      // Get the interface index
      ac_printf("%s: ifname count=%d\n", this->comp.name, AC_ARRAY_COUNT(this->ifname));
      for (this->ifname_idx = 0; this->ifname_idx < AC_ARRAY_COUNT(this->ifname); this->ifname_idx++) {
        status = get_ifindex(this->fd, this->ifname[this->ifname_idx], &this->ifindex);
        if (status == AC_STATUS_OK) {
          break;
        } else {
          ac_debug_printf("%s: Could not get interface index for ifname=%s errno=%d\n",
              this->comp.name, this->ifname[this->ifname_idx], errno, strerror(errno));
        }
      }
      if (status != AC_STATUS_OK) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could NOT get interface index address status=%d.%d %s\n", this->comp.name,
            AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
        ac_fail((char*)str);
      }
      ac_printf("%s: ifname_idx=%d ifname=%s\n", this->comp.name, this->ifname_idx, this->ifname[this->ifname_idx]);

      // Get the mac address for our interface
      status = get_ethernet_mac_addr(this->fd, this->ifname[this->ifname_idx], this->ifmac_addr);
      if (status != AC_STATUS_OK) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could get interface mac address status=%d.%d %s\n", this->comp.name,
            AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
        ac_fail((char*)str);
      }
      ac_printf("%s: ifmac_addr=", this->comp.name);
      ac_println_hex("send_ethernet_arp_ipv4: pArpReq->arp_sha=",
          this->ifmac_addr, sizeof(this->ifmac_addr), ":");

      // Get the ipv4 address for our interface
      status = get_ethernet_ipv4_addr(this->fd, this->ifname[this->ifname_idx], &this->ifipv4_addr);
      if (status != AC_STATUS_OK) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could get interface ipv4 address status=%d.%d %s\n", this->comp.name,
            AC_STATUS_MAJOR(status), AC_STATUS_MINOR(status), strerror(AC_STATUS_MINOR(status)));
        ac_fail((char*)str);
      }
      ac_printf("%s: ", this->comp.name);
      ac_println_dec("ifipv4_addr.sin_addr=", &this->ifipv4_addr.sin_addr, AC_IPV4_ADDR_LEN, ".");

      // Initialize padding array
      ac_memset(this->padding, 0, sizeof(this->padding));

      // Create a reader thread
      this->reader_thread_rslt = ac_thread_create(0, &reader_thread, this);
      ac_assert(this->reader_thread_rslt.status == AC_STATUS_OK);

      break;
    }
    case (AC_DEINIT_CMD): {
      TEMP_FAILURE_RETRY (close(this->fd));
      ac_debug_printf("%s: AC_DEINIT_CMD\n", this->comp.name);
      break;
    }
    case (AC_INET_SEND_ARP_CMD): {
      AcInetSendArpExtra* send_arp_extra = (AcInetSendArpExtra*)msg->extra;
      ac_printf("%s: AC_INET_SEND_ARP_CMD proto=%x", this->comp.name, send_arp_extra->proto);
      ac_println_dec(" proto_addr=", send_arp_extra->proto_addr, send_arp_extra->proto_addr_len, ".");

      send_arp(this, send_arp_extra->proto, send_arp_extra->proto_addr_len, send_arp_extra-> proto_addr);

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
  .ifname_idx = 0,
  .ifname[0] = "sadie1", // Our preferred interface, usually a macvlan interface
  .ifname[1] = "eth0",   // envy laptop wired
  .ifname[2] = "wlo1",   // envy laptop wireless
  .ifname[3] = "eno1",   // wink-desktop
  .ifname[4] = "enp0s26f7ulu3", // mpaw
  .ifname[5] = "wlp2s0", // mpawl
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

  ac_assert(ac_printf_register_format_proc_str(format_proc_sockaddr_ll, "sockaddr_ll") == AC_STATUS_OK);
  ac_assert(ac_printf_register_format_proc_str(format_proc_ethhdr, "ethhdr") == AC_STATUS_OK);

  AcIpv6Addr ipv6_addr = {
    .ary_u16[7] = 0x3210,
    .ary_u16[6] = 0x7654,
    .ary_u16[5] = 0xba98,
    .ary_u16[4] = 0xfedc,
    .ary_u16[3] = 0xcdef,
    .ary_u16[2] = 0x89ab,
    .ary_u16[1] = 0x4567,
    .ary_u16[0] = 0x0123,
  };
  ac_print_mem("ipv6_addr=", &ipv6_addr, AC_ARRAY_COUNT(ipv6_addr.ary_u16), sizeof(ipv6_addr.ary_u16[0]),
    "%04x", ":", "\n");

  struct sockaddr_ll sock_addr = {
    .sll_family = 1,
    .sll_protocol = AC_HTON_U16(2345),
    .sll_ifindex = 6,
    .sll_hatype = 7,
    .sll_pkttype = 8,
    .sll_halen = 6,
    .sll_addr = { 0xa, 0xb, 0xc, 0xd, 0xe, 0xf }
  };

  struct ethhdr ether_hdr = {
    .h_dest = { 1, 2, 3, 4, 5, 6 },
    .h_source = { 7, 8, 9, 0xa, 0xb, 0xc},
    .h_proto = 12,
  };
  ac_printf("sockaddr_ll=%{sockaddr_ll}\nethhdr=%{ethhdr}\n", &sock_addr, &ether_hdr);

  ac_assert(AcCompMgr_add_comp(cm, &comp_ipv4_ll.comp) == AC_STATUS_OK);

  ac_debug_printf("AcInetLink_init:-cm=%p\n", cm);
}
