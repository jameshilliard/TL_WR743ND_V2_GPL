/*
 * leases.c -- tools to manage DHCP leases
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"
#include "common.h"

#include "static_leases.h"


uint8_t blank_chaddr[] = {[0 ... 15] = 0};
uint8_t blank_host_name[] = {[0 ... 31] = 0};	/* add by Li Shaozhang 07Jun07 */


/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
void clear_lease(uint8_t *chaddr, uint32_t yiaddr)
{
	unsigned int i, j;

	for (j = 0; j < 16 && !chaddr[j]; j++);

	for (i = 0; i < server_config.max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}
}


/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(uint32_t state, 
								uint8_t * host_name,
								uint8_t *chaddr,
								uint32_t yiaddr,
								unsigned long lease)
{
	struct dhcpOfferedAddr *oldest;

	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);

	oldest = oldest_expired_lease();
	//printf("state:%d\n oldest:%d\n", state/* == DHCPACK*/, oldest == 0);
	if (oldest) {
		oldest->state = state;
		memcpy(oldest->host_name, host_name, 32);	/* add by Li Shaozhang, 07Jun07 */
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;

		/* lease + time(0) may overflow */
		if (lease > 0xFFFFFFFF - uptime()/*time(0)*/)
			oldest->expires = 0xFFFFFFFF;
		else
			oldest->expires = uptime()/*time(0)*/ + lease;
	}

	return oldest;
}


/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
	return (lease->expires < (unsigned long) uptime()/*time(0)*/);
}


/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = uptime()/*time(0)*/;
	unsigned int i;


	for (i = 0; i < server_config.max_leases; i++)
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;

}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(uint8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);

	return NULL;
}


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(uint32_t yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);

	return NULL;
}


/* check is an IP is taken, if it is, add it to the lease table */
int check_ip(struct dhcpMessage *oldpacket, uint32_t addr)
{
	struct in_addr temp;
	uint8_t* mac = NULL;

	mac = oldpacket->chaddr;
	
	if (arpping(addr, mac, server_config.server, server_config.arp, server_config.interface) == 0) {
		temp.s_addr = addr;
		LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds",
			inet_ntoa(temp), server_config.conflict_time);
		add_lease(DHCPNULL, blank_host_name, blank_chaddr, addr, server_config.conflict_time);
		return 1;
	} else return 0;
}


/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
uint32_t find_address(struct dhcpMessage *oldpacket, int check_expired)
{
	uint32_t addr, ret;
	struct dhcpOfferedAddr *lease = NULL;

	addr = ntohl(server_config.start); /* addr is in host order here */
	for (;addr <= ntohl(server_config.end); addr++) 
	{
		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;

		/* Only do if it isn't an assigned as a static lease */
		if(!reservedIp(server_config.static_leases, htonl(addr)))
		{
			/* lease is not taken */
			ret = htonl(addr);
			if ((!(lease = find_lease_by_yiaddr(ret)) ||

			     /* or it expired and we are checking for expired leases */
			     (check_expired  && lease_expired(lease))) &&

			     /* and it isn't on the network */
		    	     !check_ip(oldpacket, ret)) 
		    {
				return ret;
				break;
			}
		}
	}
	return 0;
}
