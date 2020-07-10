/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INTERNET OF THINGS FIRST BOOT APPLICATION SOFTWARE
 *
 * DNS Captive Portal
 * Use of the DNS Captive Portal will redirect a user's
 * device to the network selection page.
 *
 * Author:        James Huggard
 * Last Modified: 09/04/2020
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include "captive_portal.h"

#define DNS_LEN 512

#define FLAG_QR (1<<7)
#define FLAG_AA (1<<2)
#define FLAG_TC (1<<1) //Truncated
#define FLAG_RD (1<<0)

#define FLAG_RA (1<<7)

#define QTYPE_A  1
#define QTYPE_NS 2
#define QTYPE_CNAME 5
#define QTYPE_SOA 6
#define QTYPE_WKS 11
#define QTYPE_PTR 12
#define QTYPE_HINFO 13
#define QTYPE_MINFO 14
#define QTYPE_MX 15
#define QTYPE_TXT 16
#define QTYPE_URI 256

#define QCLASS_IN 1
#define QCLASS_ANY 255
#define QCLASS_URI 256

static int sockFd;

// Header for DNS packet
typedef struct {
	uint16_t id;
	uint8_t flags;
	uint8_t rcode;
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
} DnsHeader;

// Type and class information for each DNS query
typedef struct {
	uint16_t type;
	uint16_t class;
} DnsQuestionFooter;

// Header and Footer that make up a DNS query
typedef struct {
	DnsHeader header;
	DnsQuestionFooter footer;
} DnsQuery;

// Footer for a DNS reply - followed by rdata
typedef struct {
	uint16_t name;
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t rdlength;
} DnsResponseFooter;

/*
 * @brief Function to extract information from a dns query
 * @param data Query received
 * @param length Length of query
 * @param query Output struct with information from received query
 * @param verbose Specifies whether debug information should be printed
 * @return ESP_OK on success
 */
esp_err_t get_dns_query_info(char* data, unsigned short length, DnsQuery *query, bool verbose) {
	int len;
	char *p = data;
	// Get Header
	query->header.id = (int)p[0]*256 + (int)p[1];
	query->header.flags = p[2];
	query->header.rcode = p[3];
	query->header.qdcount = (int)p[4]*256 + (int)p[5];
	query->header.ancount = (int)p[6]*256 + (int)p[7];
	query->header.nscount = (int)p[8]*256 + (int)p[9];
	query->header.arcount = (int)p[10]*256 + (int)p[11];

	if(verbose) {
		printf("--------HEADER--------\n");
		printf("ID:      %hu\n", query->header.id);
		printf("FLAGS:   %d \n", query->header.flags);
		printf("RCODE:   %d \n", query->header.rcode);
		printf("QDCOUNT: %hu\n", query->header.qdcount);
		printf("ANCOUNT: %hu\n", query->header.ancount);
		printf("NSCOUNT: %hu\n", query->header.nscount);
		printf("ARCOUNT: %hu\n", query->header.arcount);
	}

	if (query->header.ancount || query->header.nscount || query->header.arcount) {
		printf("This is a reply...");
		return ESP_FAIL;
	}
	if (query->header.flags&FLAG_TC) {
		printf("Truncated. Can't use this.");
		return ESP_FAIL;
	}

	if(verbose) printf("--------BODY--------\n");
	p = &p[12];

	while(p[0] != 0) {
		len = (int) p++[0];
		for (int i = 0; i < len; i++) {
			if(verbose) printf("%c", p[i]);
		}
		p += len;
		if(p[0] != 0 && verbose) {
			printf(".");
		}

	}
	if(verbose) printf("\n");
	p++;

	query->footer.type = (int)p[0]*256 + (int)p[1];
	query->footer.class = (int)p[2]*256 + (int)p[3];

	if(verbose) {
		printf("--------FOOTER--------\n");
		printf("QTYPE:   %hu\n", query->footer.type);
		printf("QCLASS:  %hu\n", query->footer.class);
	}

	return ESP_OK;

}

/*
 * @brief Convert a uint32_t IPv4 address to a uint8_t array
 * @param ip The IP to be converted
 * @param footer Array of numbers which make up plaintext IPv4 address
 * @return ESP_OK on success
 */
esp_err_t get_rdata(uint32_t ip, uint8_t *footer) {

	footer[3] = floor(ip / pow(2, 24));
	ip -= footer[3] * pow(2, 24);

	footer[2] = floor(ip / pow(2, 16));
	ip -= footer[2] * pow(2, 16);

	footer[1] = floor(ip / pow(2, 8));
	ip -= footer[1] * pow(2, 8);

	footer[0] = ip;

	return ESP_OK;

}

/*
 * @brief convert a 16-bit uint16_t to 2 chars
 * @param from The uint16_t to be converted
 * @param to The char array output
 * @return ESP_OK on success
 */
esp_err_t copy_uint16(uint16_t from, char *to) {
	uint8_t num = floor(from / pow(2, 8));
	to[0] = (char)num;

	to[1] = (char)from-num*pow(2,8);

	return ESP_OK;
}

/*
 * @brief Alter a received DNS query to form a reply
 * @param header Altered DNS response header
 * @param footer Type and class info from original request
 * @param rdata IPv4 Address to be transmitted as reply
 * @param reply Pointer to beginning of response
 * @param original_length Length of original DNS query
 * @return Length of response
 */
int alter_query_to_reply(DnsHeader *header, DnsResponseFooter *footer, uint8_t *rdata, char *reply, unsigned short original_length) {
	char *footer_ptr = &reply[original_length]; // Set pointer to end of query
	char *header_ptr = reply;

	// Copy over header - ID
	copy_uint16(header->id, header_ptr);
	header_ptr += sizeof(uint16_t);

	memcpy(header_ptr, &header->flags, sizeof(uint8_t));
	header_ptr += sizeof(uint8_t);

	memcpy(header_ptr, &header->rcode, sizeof(uint8_t));
	header_ptr += sizeof(uint8_t);

	copy_uint16(header->qdcount, header_ptr);
	header_ptr += sizeof(uint16_t);

	copy_uint16(header->ancount, header_ptr);
	header_ptr += sizeof(uint16_t);

	copy_uint16(header->nscount, header_ptr);
	header_ptr += sizeof(uint16_t);

	copy_uint16(header->arcount, header_ptr);
	header_ptr += sizeof(uint16_t);

	// Move rend to beginning of footer
	copy_uint16(footer->name, footer_ptr);
	footer_ptr += sizeof(uint16_t);

	copy_uint16(footer->type, footer_ptr);
	footer_ptr += sizeof(uint16_t);

	copy_uint16(footer->class, footer_ptr);
	footer_ptr += sizeof(uint16_t);

	for (int i = 0; i < 3; i++) {
		footer_ptr[i] = 0;
	}
	footer_ptr[2] = 1;
	footer_ptr += sizeof(uint32_t);

	copy_uint16(footer->rdlength, footer_ptr);
	footer_ptr += sizeof(uint16_t);

	// Copy over footer and IP address
	memcpy(footer_ptr, rdata, 4);
	footer_ptr += 4;

	printf("Response Data:\n");

	for (int i = 0; i < footer_ptr-reply; i++) {
		printf("%d: %d \n", i, reply[i]);
	}

	return footer_ptr-reply;
}

/*
 * @brief Function to handle reception of a DNS query
 * @param premote_addr Socket info
 * @param pusrdata Data in query
 * @param length Length of query
 */
static void captive_portal_recv(struct sockaddr_in *premote_addr, char *pusrdata, unsigned short length) {
	char reply[DNS_LEN];
	int transmit_length;
	DnsQuery query;
	DnsHeader response_header;
	DnsResponseFooter response_footer;
	uint32_t ip_address;


	// Sanity Checks
	if (length > DNS_LEN) {
		printf("Packet larger than DNS implementation");
		return;
	}
	if (length < sizeof(DnsHeader)) {
		printf("Packet too short");
		return;
	}

	// Response will be same as request with some added data
	memcpy(reply, pusrdata, length);

	ESP_ERROR_CHECK(get_dns_query_info(pusrdata, length, &query, 0));

	// Build response header
	response_header = query.header;
	response_header.flags |= FLAG_QR;
	response_header.rcode |= FLAG_RA;
	response_header.ancount = 1;

	if (query.footer.type == QTYPE_A) {
		// This is a request for an IPv4 address

		uint8_t rdata[4];

		response_footer.name = pow(2, 15) + pow(2, 14) + 12;
		response_footer.type = QTYPE_A;    // A record
		response_footer.class = QCLASS_IN; // An internet address
		response_footer.ttl = 10;
		response_footer.rdlength = 4;

		// Get IP address
		ip_address = get_ap_ip_address();
		get_rdata(ip_address, rdata);

		transmit_length = alter_query_to_reply(&response_header, &response_footer, rdata, reply, length);

		sendto(sockFd, (uint8_t*)reply, transmit_length, 0, (struct sockaddr *)premote_addr, sizeof(struct sockaddr_in));
	}

}

/*
 * @brief Captive portal task to be run
 */
static void captive_portal_task(void *pvParameters) {
	struct sockaddr_in server_addr;
	uint32_t ret;
	struct sockaddr_in from;
	socklen_t fromlen;
	char udp_msg[DNS_LEN];

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(53);
	server_addr.sin_len = sizeof(server_addr);

	do {
		sockFd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockFd == -1) {
			printf("captive_portal_task failed to create socket\n");
			vTaskDelay(1000/portTICK_RATE_MS);
		}
	} while (sockFd ==-1);

	do {
		ret = bind(sockFd, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if (ret !=0) {
			printf("captive_portal_task failed to bind socket \n");
			vTaskDelay(1000/portTICK_RATE_MS);
		}
	} while (ret!=0);

	printf("Captive Portal Initiated.");

	while (1) {
		memset(&from, 0, sizeof(from));
		fromlen=sizeof(struct sockaddr_in);
		ret = recvfrom(sockFd, (uint8_t *)udp_msg, DNS_LEN, 0, (struct sockaddr *)&from, (socklen_t *)&fromlen);
		if (ret > 0) {
			captive_portal_recv(&from,udp_msg,ret);
		}
	}

	close(sockFd);
	vTaskDelete(NULL);
}

void captive_portal_init() {
	xTaskCreate(captive_portal_task, (const char *)"captive_portal_task", 10000, NULL, 3, NULL);
}

