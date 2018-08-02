#include <string>
#include "assert.h"

#if defined WINDOWS
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "unistd.h"
#endif

#ifdef __ANDROID__
#include <cstdlib>
#include <stdio.h>
#include <sys/endian.h>
#endif

#include "basecode/BaseLog.h"
#include "basecode/basefunc.h"
#include "basecode/NetworkAddress.h"

unsigned int NetworkAddress::m_netmask = 0;

NetworkAddress::NetworkAddress() : ipAddressSet( false )
{
	char hostname[256];
	#if defined WINDOWS
	#ifdef METRO
	#else
	gethostname( hostname, 256 );
	#endif
	#else
	strcpy( hostname, "localhost" );
	#endif
    setHostName( hostname );
    aPort = 0;
}


NetworkAddress::NetworkAddress( const Data& hostName, uint16 port ) : ipAddressSet(false)
{
    setPort( port );
    setHostName( hostName );
}


void
NetworkAddress::setHostName( const Data& theAddress )
{
    const char* cstrAddress;
    Data sAddress, sPort;
    unsigned int length = theAddress.length();
    unsigned int colonPos = length;
    cstrAddress = theAddress.c_str();
    for ( int i = 0; *cstrAddress != '\0'; cstrAddress++, i++ )
    {
        if ( *cstrAddress == ':' )
        {
            colonPos = i;
            break;
        }
    }
    if ( colonPos != length )
    {
        sAddress = theAddress.substr( 0, colonPos );
        sPort = theAddress.substr( colonPos+1, length-colonPos-1 );
        colonPos = atoi( sPort.c_str() );
        if ( colonPos )
        {
            aPort = colonPos;
        }
        else
        {
            log_out( NETWORK, LOG_ERR, "Incorrect Port" );
        }
    }
    else // No ':' in input string;
    {
        sAddress = theAddress;
    }
    rawHostName = sAddress;
    ipAddressSet = false;
}


void
NetworkAddress::setIp4Addr( unsigned int addr, uint16 port )
{
	memcpy( ipAddress, (void*)&addr, IPV4_LENGTH );
	aPort = port;
	ipAddressSet = true;	
}


void
NetworkAddress::setPort( uint16 iPort )
{
    aPort = iPort;
}


Data
NetworkAddress::getHostName() const
{
    Data hostName;
#if defined METRO
	
#else
    struct hostent * pxHostEnt = 0;
    initIpAddress();
    pxHostEnt = gethostbyaddr( ipAddress, IPV4_LENGTH, AF_INET );
    if ( pxHostEnt )
        hostName = pxHostEnt->h_name;
    else
        hostName = getIpName();
#endif
	return hostName;
}


Data
NetworkAddress::getIpName() const
{
    Data ipName;
#if defined METRO
#else
    struct in_addr temp;
    initIpAddress();
    memcpy( (void *)&temp.s_addr, ipAddress, IPV4_LENGTH );
    ipName = inet_ntoa( temp );
#endif
    return ipName;
}


Data
NetworkAddress::getPortName() const
{
	return Data::ToString(aPort);
}


Data
NetworkAddress::getIpPortName() const
{
	return (getIpName()+getPortName());
}


unsigned int
NetworkAddress::getIp4Address() const
{
    unsigned int uiTmp;
    initIpAddress();
    memcpy( (void *) &uiTmp, ipAddress, IPV4_LENGTH );
    return uiTmp;
}


#if !defined METRO

void
NetworkAddress::getSockAddr( struct sockaddr& socka ) const
{
    short tmp_s = htons( aPort );
    char *tmp_p;
    tmp_p = (char*)&tmp_s;
    socka.sa_family = AF_INET;
    socka.sa_data[0] = tmp_p[0];
    socka.sa_data[1] = tmp_p[1];
    initIpAddress();
    memcpy( (void *)&socka.sa_data[2], ipAddress, IPV4_LENGTH );
    return ;
}

#endif

uint16
NetworkAddress::getPort() const
{
    return aPort;
}


bool
operator < ( const NetworkAddress & xAddress, const NetworkAddress & yAddress )
{
    xAddress.initIpAddress();
    yAddress.initIpAddress();
    for (int i = 0 ; i <= IPV4_LENGTH; i++ )
    {
        if ( xAddress.ipAddress[i] < yAddress.ipAddress[i])
        {
            return true;
        }
        if ( xAddress.ipAddress[i] > yAddress.ipAddress[i])
        {
            return false;
        }
    }
    return xAddress.aPort < yAddress.aPort ;
}


bool
operator == ( const NetworkAddress& xAddress, const NetworkAddress& yAddress )
{
    xAddress.initIpAddress();
    yAddress.initIpAddress();
    for ( int i = 0 ; i < IPV4_LENGTH ; i++ )
    {
        if ( xAddress.ipAddress[i] != yAddress.ipAddress[i])
        {
            return false;
        }
    }
    return xAddress.aPort == yAddress.aPort ;
}


bool
operator != ( const NetworkAddress& xAddress, const NetworkAddress& yAddress )
{
    return !( xAddress == yAddress );
}


NetworkAddress& 
NetworkAddress::operator=( const NetworkAddress& x )
{
	aPort = x.aPort;
    memcpy( this->ipAddress, x.ipAddress, sizeof(this->ipAddress) );
    ipAddressSet = x.ipAddressSet;
    rawHostName = x.rawHostName;
    return ( *this );
}



bool
NetworkAddress::isValid()
{
	return ipAddressSet;
}

void
NetworkAddress::initIpAddress() const
{
#if defined METRO

#else
    if ( !ipAddressSet )
    {
        if ( true == is_valid_ip_addr( rawHostName, ipAddress ) )
        {
            ipAddressSet = true;
        }
        else
        {
			struct hostent * pxHostEnt = 0;
			//#if defined WIN32
// 			pxHostEnt = gethostbyname( rawHostName.c_str() );
// 			if ( pxHostEnt == NULL )
// 			{
// 				LogOut( NETWORK, LOG_ERR, "gethostbyname error: %s", rawHostName.c_str() );
// 				return;
// 			}

			for ( int i=0; i<10; i++ )
			{
				pxHostEnt = gethostbyname( rawHostName.c_str() );
				if ( pxHostEnt != NULL )
					break;
				vsleep( 300 );
			}

			if ( pxHostEnt == NULL )
			{
				LogOut( NETWORK, LOG_ERR, "gethostbyname error: %s", rawHostName.c_str() );
				return;
			}

			memcpy( (void *)&ipAddress, (void *)pxHostEnt->h_addr, pxHostEnt->h_length );
			ipAddressSet = true;
			/*
			#else
			if ( rawHostName == "localhost" )
			{
				int fd;
				struct ifreq buf[16]; 
				struct ifconf ifc; 
				if ( ( fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) >= 0 ) 
				{ 
					ifc.ifc_len = sizeof buf; 
					ifc.ifc_buf = (caddr_t) buf; 
					if ( !ioctl (fd, SIOCGIFCONF, (char *) &ifc) ) 
					{ 
						if ( !( ioctl (fd, SIOCGIFADDR, (char *) &buf[1]) ) )
						{
							memcpy( (void*)&ipAddress, (void*)(&((struct sockaddr_in*)(&buf[1].ifr_addr))->sin_addr), sizeof( in_addr ) );
							ipAddressSet = true;
						}
						else
						{
							close( fd );
							cpLog( LOG_ERR, "ioctl error" );
							return;
						}
					}
					else
					{
						close( fd );
						cpLog( LOG_ERR, "ioctl error" );
						return;
					}
					close(fd);
				}
				else
				{
					cpLog( LOG_ERR, "Create socket error" );
					return;
				}
			}	
			else
			{
				pxHostEnt = gethostbyname( rawHostName.getData() );
				if ( pxHostEnt == NULL )
				{
					cpLog( LOG_ERR, "gethostbyname error" );
					return;
				}
				memcpy( (void *)&ipAddress, (void *)pxHostEnt->h_addr, pxHostEnt->h_length );
				ipAddressSet = true;
				memcpy( (void *)&ipAddress, (void *)pxHostEnt->h_addr, pxHostEnt->h_length );
			}
			#endif
			*/
        }
    }
#endif
}


bool
NetworkAddress::is_valid_ip_addr( const Data& addr, char* retaddr )
{
	unsigned long maskcheck = ~255;
	const char *advancer = addr.c_str();
	unsigned long octet;
	char *nextchar;
	char ipAddr[ IPV4_LENGTH ];
	if ( ( *(advancer) == 0 ) || (*(advancer) == ' ') || (*(advancer) == '\t') )
	{	
		return false;
	}
	octet = strtoul( advancer, &nextchar, 10 );
	if( (*nextchar != '.') || (octet & maskcheck) || (octet == 0) )
	{
		return false;
	}
	assert( octet < 256 );
	ipAddr[0] = (char)octet;
	
	advancer = nextchar + 1;
	if( (*(advancer) == 0) || (*(advancer) == ' ') || (*(advancer) == '\t') )
	{
     	return false;
	}
	octet = strtoul( advancer, &nextchar, 10 );
	if( (*nextchar != '.') || (octet & maskcheck) )
	{
		return false;
	}
	assert( octet < 256 );
	ipAddr[1] = (char)octet;
	advancer = nextchar+1;
	if( (*(advancer) == 0) || (*(advancer) == ' ') || (*(advancer) == '\t') )
	{
		return false;
	}
	octet = strtoul(advancer, &nextchar, 10);
	if((*nextchar != '.') || (octet & maskcheck))
	{
		return false;
	}
	assert( octet < 256 );
	ipAddr[2] = (char)octet;
    advancer = nextchar+1;
    if ((*(advancer) == 0) || (*(advancer) == ' ') || (*(advancer) == '\t'))
    {
        return false;
    }
    octet = strtoul(advancer, &nextchar, 10);
    if ( (*nextchar) || (octet & maskcheck) || (octet == 0) )
    {	
        return false;
    }
	assert( octet < 256 );
    ipAddr[3] = (char)octet;
	
	if( NULL != retaddr )							//bug modify by wensdy NULL != addr
	{
		memcpy( retaddr, ipAddr, IPV4_LENGTH );
	}
    return true;
}

bool
NetworkAddress::isSameNetwork( const NetworkAddress& dtIp )
{
	unsigned int ipaddr1 = getIp4Address();
	unsigned int ipaddr2 = dtIp.getIp4Address();
	unsigned int netmask = getNetmask();
	if ( (ipaddr1 & netmask) == (ipaddr2 & netmask ) )
		return true;
	else
		return false;
}

unsigned int
NetworkAddress::getNetmask()
{
	/*
	if ( m_netmask != 0 )
		return m_netmask;
	#ifdef WINDOWS

	#else
	int fd;
	struct ifreq buf[16]; 
	struct ifconf ifc; 
	if ( ( fd = socket(AF_INET, SOCK_DGRAM, 0) ) >= 0) 
	{ 
		ifc.ifc_len = sizeof buf; 
		ifc.ifc_buf = (caddr_t) buf; 
		if ( !ioctl(fd, SIOCGIFCONF, (char *) &ifc) ) 
		{ 
			if ( !(ioctl(fd, SIOCGIFNETMASK, (char *)&buf[1])) ) 
			{ 
				memcpy( (void*)&m_netmask, (void*)(&((struct sockaddr_in*)(&buf[1].ifr_netmask))->sin_addr), sizeof( unsigned int ) );
				close(fd);
				return m_netmask;
			}
		}
		close(fd);
	} 
	#endif
	return m_netmask;
	*/
	return 0;
}

Data
NetworkAddress::getNetmaskName()
{
#if !defined METRO
	unsigned int netmask = getNetmask();
	return Data( inet_ntoa( *((struct in_addr *)&netmask )) );
#else
	return "";
#endif
}
