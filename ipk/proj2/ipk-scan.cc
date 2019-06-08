#include <stdio.h>
#include <getopt.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <pcap.h>
#include <net/ethernet.h>
#include <csignal>
#include <stdlib.h>
#include <time.h>

//Define maximum pakcet length as 4096 bytes. Our actual packets will be much shorter
#define PCKT_LEN 4096

/**
 * @brief Exception used in argument parsing 
 */
class ArgumentException{};
/**
 * @brief Exception used in parsing recieved data 
 */
class HeaderException{};
/**
 * @brief Exception used in interface selecting
 */
class InterfaceException{};
/**
 * @brief Exception used in packet sending
 */
class SendException : public std::exception 
{
    int retval;
    public:
        SendException(const int val) : retval{val} {}

        virtual const char *what() const throw()
        {
            return std::to_string(retval).c_str();
        }
};

/**
 * @brief Pseudo header used for IPv4 checksum calculation
 *  
 * <a href="https://tools.ietf.org/html/rfc793">Source</a> 
 */
struct pseudo_header
{
	unsigned int source_address;
	unsigned int dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;
	
	struct tcphdr tcp;
};

/**
 * @brief Pseudo header for IPv6 checksum calculation for TCP 
 *
 * <a href="https://tools.ietf.org/html/rfc2460">Source</a> 
 */
struct pseudo_header_v6_TCP
{
    struct in6_addr source_address;
    struct in6_addr dest_address;
    unsigned int protocol_length;
    unsigned char zero[3];
    unsigned char next_header;

    struct tcphdr tcp;
};

/**
 * @brief Pseudo header for IPv6 checksum calculation for UDP
 *
 * <a href="https://tools.ietf.org/html/rfc2460">Source</a> 
 */
struct pseudo_header_v6_UDP
{
    struct in6_addr source_address;
    struct in6_addr dest_address;
    unsigned int protocol_length;
    unsigned char zero[3];
    unsigned char next_header;

    struct udphdr udp;
};

/**
 * @brief Stores information about target ports specified in command line arguments
 */
struct arg_ports
{
    std::vector<int> port_vec;
    bool is_range;
};
/**
 * @brief Global variable storing pcap handle. Needs to be global for singal handling to work 
 */
pcap_t *handle;

/**
 * @brief Checksum calculating function
 *
 * @param ptr pointer to data where the checksum should be calculated
 * @param nbytes Number of bytes to be calculated
 * 
 * <a href="https://people.cs.clemson.edu/~yfeaste/cybersecurity/CPSC360/code/cCode/UnixExamples/exampleTime1/checksum.c">Source</a> 
 * @return uin16_t checksum 
 */
uint16_t csum(unsigned short *ptr,int nbytes) 
{
	long sum;
	unsigned short oddbyte;
	short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}


/**
 * @brief Finds IP or IPv6 address of target
 *
 * @param host String containing the target host name
 * @param data struct sockaddr_storage where IP data is stored
 *
 * <a href="http://man7.org/linux/man-pages/man3/getaddrinfo.3.html">Source</a> 
 */
void get_dest_ip(const char *host, struct sockaddr_storage *data)
{
    //getaddrinfo variables
    struct addrinfo hints, *res;
    int status;

    //Sets hints data structure
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_RAW;

    if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0)
    {
        std::cerr << "Getaddrinfo error: " << gai_strerror(status) << std::endl;
        throw InterfaceException();
    }
    memcpy(data, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
}

/**
 * @brief Finds IP of specified interface on local machine
 *
 * @param interface String containing the name of interface
 * @param sock_fd File descriptor of open socket
 * @param data struct sockaddr_storage where IP data is stored
 * @param family Specifies the IP family (AF_INET or AF_INET6)
 *
 * <a href="http://man7.org/linux/man-pages/man3/getifaddrs.3.html">Source</a> 
 */
void get_own_ip(const char *interface, int sock_fd, struct sockaddr_storage *data, int family)
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("Getifaddrs: ");
        throw InterfaceException();
    }

    //Loop through interfaces and search for specified interface
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        
        if (strcmp(interface, ifa->ifa_name) == 0 && ifa->ifa_addr->sa_family == family)
        {
            if (family == AF_INET)
                memcpy(data, (struct sockaddr_in *)ifa->ifa_addr, sizeof(struct sockaddr_in));
            else
                memcpy(data, (struct sockaddr_in6 *)ifa->ifa_addr, sizeof(struct sockaddr_in6));

            break;
        }
    }
}

/**
 * @brief Prints timeout message
 *
 * @param proto Protocol
 * @param port Port
 */
void print_timeout(const std::string proto, int port)
{
    std::cout << '\t' << port << '/' << proto << '\t' << "filtered (timeout)" <<  std::endl;
}

/**
 * @brief Prints TCP port status (open/closed) according to recieved packet
 *
 * @param packet Byte array containing the packet data
 * @param is_ipv6 Check if packet is IPv4 or IPv6
 */
void print_tcp_stats(const u_char *packet, bool is_ipv6)
{
    struct tcphdr *tcp_header;
    if(is_ipv6)
        tcp_header = (struct tcphdr *) (packet + sizeof(struct ip6_hdr) + sizeof(struct ethhdr));
    else
        tcp_header = (struct tcphdr *) (packet + sizeof(struct iphdr) + sizeof(struct ethhdr));


    if (tcp_header->syn && tcp_header->ack)
        std::cout << '\t' << ntohs(tcp_header->source) << "/tcp" << '\t' << "open" << std::endl;
    else if (tcp_header->ack && tcp_header->rst)
        std::cout << '\t' << ntohs(tcp_header->source) << "/tcp" << '\t' << "closed" << std::endl;
    else
        throw HeaderException();
}

/**
 * @brief Handles SIGALRM
 *
 * @param sig Singal number
 */
void alarm_handler(int sig)
{
    if (sig == SIGALRM)
    {
        //Break the pcap_next waiting loop
        pcap_breakloop(handle);
    }
}

/**
 * @brief Fills the IPv6 header data structure
 *
 * @param header Pointer to header data structure
 * @param dst_ip struct in6_addr containing destination IP address
 * @param own_ip struct in6_addr containg own IP address
 * @param datagram Byte array where the header is located
 * @param next_hdr Specifies the next header down stack (TCP or UDP)
 */
void fill_IPV6_header(struct ip6_hdr **header, struct in6_addr dst_ip, struct in6_addr own_ip, uint8_t *datagram, int next_hdr)
{
    (*header)->ip6_ctlun.ip6_un2_vfc = 0x60;
    (*header)->ip6_ctlun.ip6_un1.ip6_un1_flow = htonl(6<<28);
    (*header)->ip6_ctlun.ip6_un1.ip6_un1_hlim = 0xFFU;
    (*header)->ip6_ctlun.ip6_un1.ip6_un1_nxt = next_hdr;
    (*header)->ip6_ctlun.ip6_un1.ip6_un1_plen = htons((next_hdr == IPPROTO_TCP) ? sizeof(struct tcphdr)
                                                                                : sizeof(struct udphdr));
    (*header)->ip6_dst = dst_ip;
    (*header)->ip6_src = own_ip;
}

/**
 * @brief Fills the IPv4 header data structure
 *
 * @param header Pointer to header data structure
 * @param dst_ip 32 bits of destination IP address
 * @param own_ip 32 bits of own IP address
 * @param datagram Byte array where the header is located
 */
void fill_IP_header(struct iphdr **header, int dst_ip, int own_ip, uint8_t *datagram)
{
    (*header)->ihl = 5;
    (*header)->version = 4;
    (*header)->tos = 0;
    (*header)->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
    (*header)->id = htons (54321);	//Id of this packet
    (*header)->frag_off = htons(16384);
    (*header)->ttl = 64;
    (*header)->protocol = IPPROTO_TCP;
    (*header)->check = 0;		//Set to 0 before calculating checksum
    (*header)->saddr = own_ip;
    (*header)->daddr = dst_ip;

    (*header)->check = csum ((uint16_t *) datagram, (*header)->tot_len >> 1);
}

/**
 * @brief Fills the TCP header data structure
 *
 * @param header Pointer to TCP header data structure
 * @param src_port Source port
 * @param dst_port Destination port
 */
void fill_TCP_header(struct tcphdr **header, int src_port, int dst_port)
{
	(*header)->source = htons(src_port);
	(*header)->dest = htons(dst_port);
	(*header)->seq = htonl(1105024978);
	(*header)->ack_seq = 0;
	(*header)->doff = sizeof(struct tcphdr) / 4;		//Size of tcp header
	(*header)->fin=0;
	(*header)->syn=1;
	(*header)->rst=0;
	(*header)->psh=0;
	(*header)->ack=0;
	(*header)->urg=0;
	(*header)->window = htons ( 14600 );	// maximum allowed window size
	(*header)->check = 0; 
	(*header)->urg_ptr = 0;
}

/**
 * @brief Fills the UDP header data structure
 *
 * @param header Pointer to UDP header
 * @param src_port Source port
 * @param dst_port Destination port
 */
void fill_UDP_header(struct udphdr **header, int src_port, int dst_port)
{
    (*header)->source = htons(src_port);
    (*header)->dest = htons(dst_port);
    (*header)->len = htons(8);
    (*header)->check = 0;
}


/**
 * @brief Splits string by delimiter into a vector of integers
 *
 * @param s Input string
 * @param delim Delimiter by which to split
 *
 * <a href="https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c">Source</a>
 *
 * @return Returns a vector of integers converted from strings
 */
std::vector<int> split(std::string s, char delim)
{
    std::vector<int> retval;
    std::stringstream ss(s);
    std::string item;
    int port;
    while (std::getline(ss, item, delim))
    {
        port = std::stoi(item);
        if (port < 0 || port > 65535)
        {
            std::cerr << "Invalid port number. Port numbers can be between 0 and 65535" << std::endl;
            throw ArgumentException();
        }

        retval.push_back(port);
    }
    return retval;

}

/**
 * @brief Parses ports from commandline aruments
 *
 * @param port_string String with command line argument
 * @param ports Reference to struct arg_ports where the result will be stored
 */
void get_ports(std::string port_string, arg_ports &ports)
{
    std::vector<int> port_vec;
    // xx-xx variant
    if (port_string.find('-') != std::string::npos)
    {
        try {
            port_vec = split(port_string, '-');
        }
        catch (...) {
            std::cerr << "Cannot parse arguments" << std::endl;
            throw ArgumentException();
        }
        if (port_vec.size() != 2)
        {
            std::cerr << "Invalid syntax. Format can be xx-xx or xx,xx,xx" << std::endl;
            throw ArgumentException();
        }

        ports.port_vec = port_vec;
        ports.is_range = true;
    }
    // xx,xx,xx variant
    else if (port_string.find(',') != std::string::npos)
    {
        try {
            port_vec = split(port_string, ',');
        }
        catch (...) {
            std::cerr << "Cannot parse arguments" << std::endl;
            throw ArgumentException();
        }
        if (port_vec.size() < 2)
        {
            std::cerr << "Unexpected ','. Format can be xx-xx or xx,xx,xx" << std::endl;
            throw ArgumentException();
        }

        ports.port_vec = port_vec;
        ports.is_range = false;
    }
    // Only one port specified
    else
    {
        try {
            if (std::stoi(port_string) < 0 || std::stoi(port_string) > 65535)
            {
                std::cerr << "Invalid port number. Port numbers can be between 0 and 65535" << std::endl;
                throw ArgumentException();
            }

            port_vec.push_back(std::stoi(port_string));
            ports.port_vec = port_vec;
            ports.is_range = false;
        }
        catch (...) {
            std::cerr << "Cannot parse arguments" << std::endl;
            throw ArgumentException();
        }
    }
}

/**
 * @brief Runs a UDP scan on specified port
 *
 * @param sock File descriptor of an open socket
 * @param datagram Byte array where data is stored
 * @param dest struct sockaddr_storage storing destination address for sendto
 * @param src_port Source port
 * @param dest_port Destination port
 * @param src_ip_string Source IP string for pcap_filter
 * @param dst_ip_string Destination IP string for pcap_filter
 * @param src_addr struct sockaddr_storage containing IPv4 or IPv6 address of source
 * @param dst_addr struct sockaddr_storage containing IPv4 or IPv6 address of destination
 * @param is_ipv6 Bool specifying which address family to scan
 */
void scan_UDP(int sock, uint8_t *datagram, struct sockaddr_storage dest, int src_port, int dest_port, char *src_ip_string, char *dst_ip_string, struct sockaddr_storage *src_addr, struct sockaddr_storage *dst_addr, bool is_ipv6)
{
    //Set the correct offset in datagram
    struct udphdr * header_udp = (struct udphdr *) (datagram + (is_ipv6 ? sizeof(struct ip6_hdr)
                                                                        : sizeof(struct iphdr)));
    struct pseudo_header_v6_UDP psh_UDP;
    struct bpf_program fp;
    char filter_exp[128];
    bpf_u_int32 net = {0};
    struct pcap_pkthdr header;
    const u_char *packet;
    memset(header_udp, 0, sizeof(tcphdr));
    bool recvd = false;

    fill_UDP_header(&header_udp, src_port, dest_port);

    //Calculate checksum if IPv6
    if (is_ipv6)
    {
        psh_UDP.dest_address = ((struct sockaddr_in6 *) dst_addr)->sin6_addr;
        psh_UDP.source_address = ((struct sockaddr_in6 *) src_addr)->sin6_addr;
        psh_UDP.protocol_length = htons(sizeof(struct udphdr));
        psh_UDP.next_header = IPPROTO_UDP;
        memcpy(&psh_UDP.udp, header_udp, sizeof(struct udphdr));

        header_udp->check = csum( (unsigned short*) &psh_UDP , sizeof (struct pseudo_header_v6_UDP));
    }

    //Rewrite the filter expression
    snprintf(filter_exp, sizeof(filter_exp), 
            "dst host %s and src host %s and icmp [icmptype]=icmp-unreach and icmp [icmpcode]=3", src_ip_string, dst_ip_string);

    //Compile and set filter
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1)
    {
        std::cerr << "Could not parse filter: " << pcap_geterr(handle) << std::endl;
        throw SendException(3);
    }

    if (pcap_setfilter(handle, &fp) == -1)
    {
        std::cerr << "Could not set filter: " << pcap_geterr(handle) << std::endl;
        throw SendException(3);
    }


    //Set up signal handling for timeouts
    signal(SIGALRM, alarm_handler);
    //Scan 5 times to try and get ICMP message back
    for (int i = 0; i < 5; i++)
    {
        if ( sendto (sock, datagram ,
                    is_ipv6 ? sizeof(struct ip6_hdr) + sizeof(struct udphdr)
                            : sizeof(struct iphdr) + sizeof(struct udphdr) 
                            , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
            perror("sendto");
            throw SendException(4);
        }
        //Set a timer of 3 seconds for pcap_next and then call break_loop in signal handler
        alarm(3);
        packet = pcap_next(handle, &header);
        if (packet != NULL)
        {
            //If recieved an ICMP message, stop sending packets and return that the port is closed
            std::cout << '\t' << dest_port  << "/udp" << '\t' << "closed" << std::endl;
            recvd = true;
            break;
        }
    }
    //If no ICMP message was recieved after 5 tries, the port is either open or filtered
    if (recvd == false)
    {
        std::cout << '\t' << dest_port << "/udp" << '\t' << "open/filtered" << std::endl;
    }
    //Free filter memory
    pcap_freecode(&fp);
} 

/**
 * @brief Loops through all specified ports and scans them
 *
 * @param udp_ports struct arg_ports containing information about specified ports
 * @param sock File descriptor of an open socket
 * @param datagram Byte array where data is stored
 * @param dest struct sockaddr_storage storing destination address for sendto
 * @param src_port Source port
 * @param src_ip_string Source IP string for pcap_filter
 * @param dst_ip_string Destination IP string for pcap_filter
 * @param src_addr struct sockaddr_storage containing IPv4 or IPv6 address of source
 * @param dst_addr struct sockaddr_storage containing IPv4 or IPv6 address of destination
 * @param is_ipv6 Bool specifying which address family to scan
 */
void scan_UDP_wrapper(struct arg_ports udp_ports, 
        int sock, 
        uint8_t *datagram, 
        struct sockaddr_storage dest, 
        int src_port, 
        char *src_ip_string, 
        char *dst_ip_string, 
        struct sockaddr_storage *src_addr, 
        struct sockaddr_storage *dst_addr, 
        bool is_ipv6)
{
    //xx-xx variant
    if (udp_ports.is_range)
    {
        for (int i = udp_ports.port_vec[0]; i <= udp_ports.port_vec[1]; i++)
        {
            scan_UDP(sock, datagram, dest, src_port, i, src_ip_string, dst_ip_string, src_addr, dst_addr, is_ipv6);
        }
    }
    //xx,xx,xx or single port variant
    else
    {
        for (int port : udp_ports.port_vec)
        {
            scan_UDP(sock, datagram, dest, src_port, port, src_ip_string, dst_ip_string, src_addr, dst_addr, is_ipv6);
        }
    }
}
/**
 * @brief Runs a TCP scan on specified port
 *
 * @param sock File descriptor of an open socket
 * @param datagram Packet data
 * @param dest Destination data for sendto
 * @param src_port Source port
 * @param dest_port Destination port
 * @param src_ip_string Source IP string for pcap_filter
 * @param dst_ip_string Destination IP string for pcap_filter
 * @param src_addr struct sockaddr_storage containing source IPv4 or IPv6 data
 * @param dst_addr struct sockaddr_storage containing destination IPv4 or IPv6 data
 * @param is_ipv6 Bool specifiying which address family to scan
 */
void scan_TCP(int sock, uint8_t *datagram, struct sockaddr_storage dest, int src_port, int dest_port, char *src_ip_string, char *dst_ip_string, struct sockaddr_storage *src_addr, struct sockaddr_storage *dst_addr, bool is_ipv6)
{
    //Set the correst offset in datagram
    struct tcphdr * header_tcp = (struct tcphdr *) (datagram + (is_ipv6 ? sizeof(struct ip6_hdr)
                                                                        : sizeof(struct iphdr)));
    struct pseudo_header psh;
    struct pseudo_header_v6_TCP psh6;
    struct bpf_program fp;
    char filter_exp[256];
    bpf_u_int32 net = {0};
    struct pcap_pkthdr header;
    const u_char *packet;
    memset(header_tcp, 0, sizeof(tcphdr));
    memset(&psh, 0, sizeof(pseudo_header));
    memset(&psh6, 0, sizeof(pseudo_header_v6_TCP));

    fill_TCP_header(&header_tcp, src_port, dest_port);
    //Fill the correct pseudo header
    if (is_ipv6)
    {
        psh6.dest_address = ((struct sockaddr_in6 *) dst_addr)->sin6_addr;
        psh6.source_address = ((struct sockaddr_in6 *) src_addr)->sin6_addr;
        psh6.protocol_length = htons(sizeof(struct tcphdr));
        psh6.next_header = IPPROTO_TCP;
        memcpy(&psh6.tcp, header_tcp, sizeof(struct tcphdr));

        header_tcp->check = csum( (unsigned short*) &psh6 , sizeof (struct pseudo_header_v6_TCP));
    }
    else
    {
        psh.source_address = ((struct sockaddr_in *) src_addr)->sin_addr.s_addr;
        psh.dest_address = ((struct sockaddr_in *) dst_addr)->sin_addr.s_addr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.tcp_length = htons(sizeof(struct tcphdr));
        memcpy(&psh.tcp, header_tcp, sizeof(struct tcphdr));

        header_tcp->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));
    }

    //Rewrite the filter expression
    snprintf(filter_exp, sizeof(filter_exp), 
            "dst host %s and src host %s and tcp src port %d and tcp dst port %d", 
            src_ip_string, dst_ip_string, dest_port, src_port);

    //Compile and set the filter
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1)
    {
        std::cerr << "Could not parse filter: " << pcap_geterr(handle) << std::endl;
        throw SendException(3);
    }

    if (pcap_setfilter(handle, &fp) == -1)
    {
        std::cerr << "Could not set filter: " << pcap_geterr(handle) << std::endl;
        throw SendException(3);
    }

    //Send the packet
    if ( sendto (sock, datagram , is_ipv6 ? sizeof(struct ip6_hdr) + sizeof(struct tcphdr) 
                                          : sizeof(struct iphdr) + sizeof(struct tcphdr),
                                          0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
    {
        perror("sendto");
        throw SendException(4);
    } 
    //Set up signal handling
    signal(SIGALRM, alarm_handler);
    //Set timeout to 1 second
    alarm(1);
    //Wait for response
    packet = pcap_next(handle, &header);
    if (packet != NULL)
        print_tcp_stats(packet, is_ipv6); //Recieved response packet, print its relevant data
    else
    {
        //If no response recieved, try again one more time
        if ( (sendto (sock, datagram , is_ipv6 ? sizeof(struct ip6_hdr) + sizeof(struct tcphdr) 
                                                      : sizeof(struct iphdr) + sizeof(struct tcphdr)
                                                      , 0 , (struct sockaddr *) &dest, sizeof (dest))) < 0)
        {
            perror("sendto");
            throw SendException(3);
        }
        alarm(3);
        packet = pcap_next(handle, &header);
        if (packet != NULL)
            print_tcp_stats(packet,is_ipv6); //Response recieved
        else
            print_timeout("tcp", dest_port); //No response, return that the port is filtered
    }
    pcap_freecode(&fp);

}

/**
 * @brief Loops through all specified ports and scans them
 *
 * @param tcp_ports struct arg_ports containing information about specified ports
 * @param sock File descriptor of an open socket
 * @param datagram Byte array where data is stored
 * @param dest struct sockaddr_storage storing destination address for sendto
 * @param src_port Source port
 * @param src_ip_string Source IP string for pcap_filter
 * @param dst_ip_string Destination IP string for pcap_filter
 * @param src_addr struct sockaddr_storage containing IPv4 or IPv6 address of source
 * @param dst_addr struct sockaddr_storage containing IPv4 or IPv6 address of destination
 * @param is_ipv6 Bool specifying which address family to scan
 */
void scan_TCP_wrapper(struct arg_ports tcp_ports, 
        int sock, 
        uint8_t *datagram, 
        struct sockaddr_storage dest, 
        int src_port, 
        char *src_ip_string, 
        char *dst_ip_string, 
        struct sockaddr_storage *src_addr, 
        struct sockaddr_storage *dst_addr,
        bool is_ipv6)
{
    if (tcp_ports.is_range)
    {
        for (int i = tcp_ports.port_vec[0]; i <= tcp_ports.port_vec[1]; i++)
        {
            scan_TCP(sock, datagram, dest, src_port, i, src_ip_string, dst_ip_string, src_addr, dst_addr, is_ipv6);
        }
    }
    else
    {
        for (int port : tcp_ports.port_vec)
        {
            scan_TCP(sock, datagram, dest, src_port, port, src_ip_string, dst_ip_string, src_addr, dst_addr, is_ipv6);
        }
    }
}

/**
 * @brief Prints help message
 */
void print_help()
{
    std::cout << "Simple TCP SYN and UDP port scanner \n"
                 "Usage:\n"
                 "./scan {-i interface} -pt tcp_ports -pu udp_ports [target]\n\n"
                 "-i (optional) specifies the local interface which will be used to send packets\n"
                 "-pt specifies the target ports for TCP SYN scanning\n"
                 "-pu specifies the target ports for UDP scanning\n"
                 "\t At least one of -pt, -pu must be specified\n"
                 "\t Format is either -pt xx-xx or -pt xx,xx,xx\n"
                 "target (compulsory) specifies the target host"<<std::endl;
}

int main(int argc, char **argv)
{
    //Long options for getopt
    static struct option long_options[] = 
    {
        {"i",   required_argument,  0,  'i'},
        {"pu",  required_argument,  0,  'u'},
        {"pt",  required_argument,  0,  't'},
        {"help",no_argument,        0,  'h'},
        {0,     0,                  0,  0  }
    };
    //Getopt variables
    int long_index = 0;
    int opt = 0;
    //Argument storage
    std::string udp;
    std::string tcp;
    std::string ifc;
    std::string host;
    //Parsed argument struct
    arg_ports tcp_ports;
    arg_ports udp_ports;
    char dst_ip_string[128];
    char own_ip_string[128];
    struct sockaddr_storage storage_dest;
    struct sockaddr_storage storage_self;
    int own_port;
    //Packet variables
    uint8_t datagram[PCKT_LEN];
    memset(datagram, 0, PCKT_LEN);
    struct iphdr *header_ip = (struct iphdr *) datagram;
    struct ip6_hdr *header_ip6 = (struct ip6_hdr *) datagram;
    //struct tcphdr * header_tcp = (struct tcphdr *) (datagram + sizeof(struct ip));
    //Socket variables
    int sock = 0;
    int one = 1;
	const int *val = &one;
    int ret;
    int ip_flag = IPPROTO_IP;
    //pcap variables
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs;
    //Control variables
    bool is_ipv6 = false;
    int family = AF_INET;

    //Initialize the source port randomly between 49152 and 65535 (the IANA ephemeral ports)
    srand(time(NULL));
    own_port = rand() % 16383 + 49152;

    //Initialize the headers
    memset(header_ip, 0, sizeof(struct iphdr));
    memset(header_ip6, 0, sizeof(struct ip6_hdr));

    //Parse arguments
    while ((opt = getopt_long_only(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch(opt)
        {
            case 'h':
                print_help();
                return 0;
            case 'i':
                ifc.assign(optarg);
                break;
            case 'u':
                udp.assign(optarg);
                break;
            case 't':
                tcp.assign(optarg);
                break;
            default:
                print_help();
                return 2;
        }
    }
    if (optind < argc)
    {
        host.assign(argv[optind]);
    }
    else
    {
        std::cerr << "Error. Host not specified" << std::endl;
        print_help();
        return 2;
    }

    //Parse individual port strings
    try
    {
        if (tcp.empty() && udp.empty())
            throw ArgumentException();
        if (!udp.empty())
            get_ports(udp, udp_ports);
        if (!tcp.empty())
            get_ports(tcp, tcp_ports);
    }
    catch(ArgumentException &e)
    {
        return 2;
    }
    catch(...)
    {
        std::cerr << "Internal error" << std::endl;
        return 1;
    }
    //If no interface was specified, look for a default one
    if (ifc.empty())
    {
        if (pcap_findalldevs(&alldevs, errbuf) != 0)
        {
            std::cerr << "findalldevs error: " << errbuf << std::endl;
            return 3;
        }
        if (alldevs == NULL)
        {
            std::cerr << "Found no interface" << std::endl;
            return 3;
        }

        for (pcap_if_t tmp = *alldevs; tmp.next != NULL; tmp = *tmp.next)
        {
            if (tmp.flags != PCAP_IF_LOOPBACK)
            {
                ifc.assign(tmp.name);
                break;
            }
        }
        pcap_freealldevs(alldevs);
    }

    //Get destination ip from host name
    get_dest_ip(host.c_str(), &storage_dest);
    //Set appropriate variables according to target address family
    if (storage_dest.ss_family == AF_INET6)
    {
        is_ipv6 = true;
        family = AF_INET6;
        ip_flag = IPPROTO_IPV6;
    }
    else if (storage_dest.ss_family == AF_INET)
    {
        is_ipv6 = false;
        family = AF_INET;
        ip_flag = IPPROTO_IP;
    }

    //Open the pcap device for listening
    handle = pcap_open_live(
            ifc.c_str(),
            BUFSIZ,
            1,
            1000,
            errbuf
        );
    if (handle == NULL)
        return 3;

    //Open the socket for sending packets
    sock = socket(family, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0)
    {
        std::cerr << "Socket error: " << std::endl;
        perror("socket");
        pcap_close(handle);
        return 4;
    }

    //Get ip address of specified interface
    get_own_ip(ifc.c_str(), sock, &storage_self, family);
    if (is_ipv6)
    {
        //Fill the ip strings for pcap_filter
        inet_ntop(family, ((struct sockaddr_in6 *) (&storage_dest))->sin6_addr.s6_addr, dst_ip_string, 128);
        inet_ntop(family, ((struct sockaddr_in6 *) (&storage_self))->sin6_addr.s6_addr, own_ip_string, 128);
        fill_IPV6_header(&header_ip6, 
                ((struct sockaddr_in6 *) &storage_dest)->sin6_addr,
                ((struct sockaddr_in6 *) &storage_self)->sin6_addr,
                datagram, IPPROTO_TCP);
    }
    else
    {
        inet_ntop(family, &((struct sockaddr_in *) (&storage_dest))->sin_addr.s_addr, dst_ip_string, 128);
        inet_ntop(family, &((struct sockaddr_in *) (&storage_self))->sin_addr.s_addr, own_ip_string, 128);
        fill_IP_header(&header_ip,
                ((struct sockaddr_in *) &storage_dest)->sin_addr.s_addr, 
                ((struct sockaddr_in *) &storage_self)->sin_addr.s_addr,  
                datagram);
    }

    //Set socket option not to include its own headers
	if ((ret = setsockopt (sock, ip_flag, IP_HDRINCL, val, sizeof (one))) < 0)
	{
        std::cerr << "Unable to setsockopt: " << std::endl;
        perror("ret");
        pcap_close(handle);
        close(sock);
		return 4;
	}

    //Start printing output
    std::cout << "Interesting ports on " << host << ':' << std::endl;
    std::cout << "\tPORT\tSTATE" << std::endl;

    //Run the scan on TCP
    if (!tcp.empty())
    {
        try
        {
            if (is_ipv6)
                header_ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt = IPPROTO_TCP;
            else
                header_ip->protocol = IPPROTO_TCP;

            scan_TCP_wrapper(tcp_ports, sock, datagram, storage_dest, 
                    own_port, own_ip_string, dst_ip_string, &storage_self, &storage_dest, is_ipv6);
        }
        catch(SendException &e)
        {
            pcap_close(handle);
            close(sock);
            return std::stoi(e.what());
        }
        catch(...)
        {
            pcap_close(handle);
            close(sock);
            return 1;
        }
    }
    //Run the scan on UDP
    if (!udp.empty())
    {
        try
        {
            if (is_ipv6)
                header_ip6->ip6_ctlun.ip6_un1.ip6_un1_nxt = IPPROTO_UDP;
            else
                header_ip->protocol = IPPROTO_UDP;

            scan_UDP_wrapper(udp_ports, sock, datagram, storage_dest, 
                    own_port, own_ip_string, dst_ip_string, &storage_self, &storage_dest, is_ipv6);
        }
        catch(SendException &e)
        {
            pcap_close(handle);
            close(sock);
            return std::stoi(e.what());
        }
        catch(...)
        {
            pcap_close(handle);
            close(sock);
            return 1;
        }
    }
    //Close all resources
    pcap_close(handle);
    close(sock);
    return 0;
}
