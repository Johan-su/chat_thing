static const char *str_from_wsaerror(int wsa_error)
{
    switch (wsa_error)
    {
        case 10004:
        return "WSAEINTR A blocking operation was interrupted by a call to WSACancelBlockingCall.";

        case 10009:
        return "WSAEBADF The file handle supplied is not valid.";

        case 10013:
        return "WSAEACCES An attempt was made to access a socket in a way forbidden by its access permissions.";

        case 10014:
        return "WSAEFAULT The system detected an invalid pointer address in attempting to use a pointer argument in a call.";

        case 10022:
        return "WSAEINVAL An invalid argument was supplied.";

        case 10024:
        return "WSAEMFILE Too many open sockets.";

        case 10035:
        return "WSAEWOULDBLOCK A non-blocking socket operation could not be completed immediately.";

        case 10036:
        return "WSAEINPROGRESS A blocking operation is currently executing.";

        case 10037:
        return "WSAEALREADY An operation was attempted on a non-blocking socket that already had an operation in progress.";

        case 10038:
        return "WSAENOTSOCK An operation was attempted on something that is not a socket.";

        case 10039:
        return "WSAEDESTADDRREQ A required address was omitted from an operation on a socket.";

        case 10040:
        return "WSAEMSGSIZE A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram into was smaller than the datagram itself.";

        case 10041:
        return "WSAEPROTOTYPE A protocol was specified in the socket function call that does not support the semantics of the socket type requested.";

        case 10042:
        return "WSAENOPROTOOPT An unknown, invalid, or unsupported option or level was specified in a getsockopt or setsockopt call.";

        case 10043:
        return "WSAEPROTONOSUPPORT The requested protocol has not been configured into the system, or no implementation for it exists.";

        case 10044:
        return "WSAESOCKTNOSUPPORT The support for the specified socket type does not exist in this address family.";

        case 10045:
        return "WSAEOPNOTSUPP The attempted operation is not supported for the type of object referenced.";

        case 10046:
        return "WSAEPFNOSUPPORT The protocol family has not been configured into the system or no implementation for it exists.";

        case 10047:
        return "WSAEAFNOSUPPORT An address incompatible with the requested protocol was used.";

        case 10048:
        return "WSAEADDRINUSE Only one usage of each socket address (protocol/network address/port) is normally permitted.";

        case 10049:
        return "WSAEADDRNOTAVAIL The requested address is not valid in its context.";

        case 10050:
        return "WSAENETDOWN A socket operation encountered a dead network.";

        case 10051:
        return "WSAENETUNREACH A socket operation was attempted to an unreachable network.";

        case 10052:
        return "WSAENETRESET The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.";

        case 10053:
        return "WSAECONNABORTED An established connection was aborted by the software in your host machine.";

        case 10054:
        return "WSAECONNRESET An existing connection was forcibly closed by the remote host.";

        case 10055:
        return "WSAENOBUFS An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.";

        case 10056:
        return "WSAEISCONN A connect request was made on an already connected socket.";

        case 10057:
        return "WSAENOTCONN A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied.";

        case 10058:
        return "WSAESHUTDOWN A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.";

        case 10059:
        return "WSAETOOMANYREFS Too many references to some kernel object.";

        case 10060:
        return "WSAETIMEDOUT A connection attempt failed because the connected party did not properly respond after a period of time, or established connection failed because connected host has failed to respond.";

        case 10061:
        return "WSAECONNREFUSED No connection could be made because the target machine actively refused it.";

        case 10062:
        return "WSAELOOP Cannot translate name.";

        case 10063:
        return "WSAENAMETOOLONG Name component or name was too long.";

        case 10064:
        return "WSAEHOSTDOWN A socket operation failed because the destination host was down.";

        case 10065:
        return "WSAEHOSTUNREACH A socket operation was attempted to an unreachable host.";

        case 10066:
        return "WSAENOTEMPTY Cannot remove a directory that is not empty.";

        case 10067:
        return "WSAEPROCLIM A Windows Sockets implementation may have a limit on the number of applications that may use it simultaneously.";

        case 10068:
        return "WSAEUSERS Ran out of quota.";

        case 10069:
        return "WSAEDQUOT Ran out of disk quota.";

        case 10070:
        return "WSAESTALE File handle reference is no longer available.";

        case 10071:
        return "WSAEREMOTE Item is not available locally.";

        case 10091:
        return "WSASYSNOTREADY WSAStartup cannot function at this time because the underlying system it uses to provide network services is currently unavailable.";

        case 10092:
        return "WSAVERNOTSUPPORTED The Windows Sockets version requested is not supported.";

        case 10093:
        return "WSANOTINITIALISED Either the application has not called WSAStartup, or WSAStartup failed.";

        case 10101:
        return "WSAEDISCON Returned by WSARecv or WSARecvFrom to indicate the remote party has initiated a graceful shutdown sequence.";

        case 10102:
        return "WSAENOMORE No more results can be returned by WSALookupServiceNext.";

        case 10103:
        return "WSAECANCELLED A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled.";

        case 10104:
        return "WSAEINVALIDPROCTABLE The procedure call table is invalid.";

        case 10105:
        return "WSAEINVALIDPROVIDER The requested service provider is invalid.";

        case 10106:
        return "WSAEPROVIDERFAILEDINIT The requested service provider could not be loaded or initialized.";

        case 10107:
        return "WSASYSCALLFAILURE A system call has failed.";

        case 10108:
        return "WSASERVICE_NOT_FOUND No such service is known. The service cannot be found in the specified name space.";

        case 10109:
        return "WSATYPE_NOT_FOUND The specified class was not found.";

        case 10110:
        return "WSA_E_NO_MORE No more results can be returned by WSALookupServiceNext.";

        case 10111:
        return "WSA_E_CANCELLED A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled.";

        case 10112:
        return "WSAEREFUSED A database query failed because it was actively refused.";

        case 11001:
        return "WSAHOST_NOT_FOUND No such host is known.";

        case 11002:
        return "WSATRY_AGAIN This is usually a temporary error during hostname resolution and means that the local server did not receive a response from an authoritative server.";

        case 11003:
        return "WSANO_RECOVERY A non-recoverable error occurred during a database lookup.";

        case 11004:
        return "WSANO_DATA The requested name is valid, but no data of the requested type was found.";

        case 11005:
        return "WSA_QOS_RECEIVERS At least one reserve has arrived.";

        case 11006:
        return "WSA_QOS_SENDERS At least one path has arrived.";

        case 11007:
        return "WSA_QOS_NO_SENDERS There are no senders.";

        case 11008:
        return "WSA_QOS_NO_RECEIVERS There are no receivers.";

        case 11009:
        return "WSA_QOS_REQUEST_CONFIRMED Reserve has been confirmed.";

        case 11010:
        return "WSA_QOS_ADMISSION_FAILURE Error due to lack of resources.";

        case 11011:
        return "WSA_QOS_POLICY_FAILURE Rejected for administrative reasons - bad credentials.";

        case 11012:
        return "WSA_QOS_BAD_STYLE Unknown or conflicting style.";

        case 11013:
        return "WSA_QOS_BAD_OBJECT Problem with some part of the filterspec or providerspecific buffer in general.";

        case 11014:
        return "WSA_QOS_TRAFFIC_CTRL_ERROR Problem with some part of the flowspec.";

        case 11015:
        return "WSA_QOS_GENERIC_ERROR General QOS error.";

        case 11016:
        return "WSA_QOS_ESERVICETYPE An invalid or unrecognized service type was found in the flowspec.";

        case 11017:
        return "WSA_QOS_EFLOWSPEC An invalid or inconsistent flowspec was found in the QOS structure.";

        case 11018:
        return "WSA_QOS_EPROVSPECBUF Invalid QOS provider-specific buffer.";

        case 11019:
        return "WSA_QOS_EFILTERSTYLE An invalid QOS filter style was used.";

        case 11020:
        return "WSA_QOS_EFILTERTYPE An invalid QOS filter type was used.";

        case 11021:
        return "WSA_QOS_EFILTERCOUNT An incorrect number of QOS FILTERSPECs were specified in the FLOWDESCRIPTOR.";

        case 11022:
        return "WSA_QOS_EOBJLENGTH An object with an invalid ObjectLength field was specified in the QOS provider-specific buffer.";

        case 11023:
        return "WSA_QOS_EFLOWCOUNT An incorrect number of flow descriptors was specified in the QOS structure.";

        case 11024:
        return "WSA_QOS_EUNKOWNPSOBJ An unrecognized object was found in the QOS provider-specific buffer.";

        case 11025:
        return "WSA_QOS_EPOLICYOBJ An invalid policy object was found in the QOS provider-specific buffer.";

        case 11026:
        return "WSA_QOS_EFLOWDESC An invalid QOS flow descriptor was found in the flow descriptor list.";

        case 11027:
        return "WSA_QOS_EPSFLOWSPEC An invalid or inconsistent flowspec was found in the QOS provider specific buffer.";

        case 11028:
        return "WSA_QOS_EPSFILTERSPEC An invalid FILTERSPEC was found in the QOS provider-specific buffer.";

        case 11029:
        return "WSA_QOS_ESDMODEOBJ An invalid shape discard mode object was found in the QOS provider specific buffer.";

        case 11030:
        return "WSA_QOS_ESHAPERATEOBJ An invalid shaping rate object was found in the QOS provider-specific buffer.";

        case 11031:
        return "WSA_QOS_RESERVED_PETYPE A reserved policy element was found in the QOS provider-specific buffer.";

        case 11032:
        return "WSA_SECURE_HOST_NOT_FOUND No such host is known securely.";

        case 11033:
        return "WSA_IPSEC_NAME_POLICY_ERROR Name based IPSEC policy could not be added.";

        default:
        return "Unknown wsa error code";
    }
}