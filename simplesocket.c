
/* vim: set filetype=c ts=8 noexpandtab: */

#include "vendor/SDK/amx/amx.h"
#include "vendor/SDK/plugincommon.h"
#include <string.h>

#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
	#include "windows.h"
	#include "io.h"
	#pragma comment(lib, "ws2_32.lib")
#else
	#include "sys/types.h"
	#include "sys/socket.h"
	#include "arpa/inet.h"
	#include "fcntl.h"
	#include "unistd.h"
#endif

typedef void (*logprintf_t)(char* format, ...);

#define amx_GetUString(dest, source, size) \
	amx_GetString(dest, source, 0, size)
#define amx_SetUString(dest, source, size) \
	amx_SetString(dest, source, 0, 0, size)

static logprintf_t logprintf;
extern void *pAMXFunctions;

static int wait, waitvalue;

struct SOCKET {
	int handle;
	AMX *amx;
	int callback;
};

struct SOCKET sockets[10];
static const struct SOCKET *sockets_end =
	sockets + sizeof(sockets)/sizeof(struct SOCKET);

static int _ssocket_set_nonblocking(int fd)
{
	int flags;
#ifdef WIN32
	flags = 1;
	return ioctlsocket(fd, FIONBIO, (DWORD*) &flags);
#else
	/*grabbed from http://dwise1.net/pgm/sockets/blocking.html*/
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
		flags = 0;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}

static void _ssocket_close(int *handle)
{
#ifdef WIN32
	closesocket(*handle);
#else
	close(*handle);
#endif
	*handle = -1;
}

/*native ssocket_connect(ssocket:handle, address[], port)*/
static cell AMX_NATIVE_CALL ssocket_connect(AMX *amx, cell *params)
{
	const int handle = params[1], port = params[3];
	struct SOCKET *s = sockets + handle;
	struct sockaddr_in addr;
	struct sockaddr *saddr = (struct sockaddr*) &addr;
	char address[50];
	cell *phys;

	if (s->handle == -1) {
		logprintf("ssocket_connect(): incorrect handle: %d", handle);
		return 0;
	}
	amx_GetAddr(amx, params[2], &phys);
	amx_GetUString(address, phys, sizeof(address));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(address);
	addr.sin_port = htons(port);

	if (connect(s->handle, saddr, sizeof(struct sockaddr)) == -1) {
		logprintf("ssocket_connect(): %d failed", handle);
		return 0;
	}
	_ssocket_set_nonblocking(s->handle);
	return 1;
}

/*native ssocket:ssocket_create()*/
static cell AMX_NATIVE_CALL ssocket_create(AMX *amx, cell *params)
{
	struct SOCKET *s = sockets;

	while (s != sockets_end) {
		if (s->handle == -1) {
			s->handle = socket(AF_INET, SOCK_DGRAM, 0);
			if (s->handle == -1) {
				logprintf("ssocket_create(): failed");
			}
			s->amx = amx;
			return s - sockets;
		}
		s++;
	}
	logprintf("ssocket_create(): no free slot");
	return -1;
}

/*native ssocket_destroy(ssocket:handle)*/
static cell AMX_NATIVE_CALL ssocket_destroy(AMX *amx, cell *params)
{
	struct SOCKET *s = sockets + params[1];

	if (s->handle != -1) {
		_ssocket_close(&s->handle);
		return 1;
	}
	return 0;
}

/*native ssocket_listen(ssocket:handle, port)*/
static cell AMX_NATIVE_CALL ssocket_listen(AMX *amx, cell *params)
{
	const int handle = params[1], port = params[2];
	struct SOCKET *s = sockets + handle;
	struct sockaddr_in addr;

	if (s->handle == -1) {
		logprintf("ssocket_listen(): incorrect handle: %d", handle);
		return 0;
	}
	if (amx_FindPublic(amx, "SSocket_OnRecv", &s->callback)) {
		logprintf("ssocket_listen(): no SSocket_OnRecv callback");
		return 0;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s->handle, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		logprintf("ssocket_listen(): cannot listen on port %d", port);
		return 0;
	}
	_ssocket_set_nonblocking(s->handle);
	return 0;
}

/*native ssocket_send(ssocket:handle, data[], len)*/
static cell AMX_NATIVE_CALL ssocket_send(AMX *amx, cell *params)
{
	struct SOCKET *s = sockets + params[1];
	const int len = params[3];
	cell *phys;

	if (s->handle != -1) {
		amx_GetAddr(amx, params[2], &phys);
		return send(s->handle, (char*) phys, len, 0);
	}
	return 0;
}

/*native ssocket_set_recv_wait(wait)*/
static cell AMX_NATIVE_CALL ssocket_set_recv_wait(AMX *amx, cell *params)
{
	wait = params[1];

	if (wait > 1000) {
		wait = 1000;
	}
	if (wait < 0) {
		wait = 0;
	}
	waitvalue = 1001;

	return 1;
}

/*native ssocket_strunpack(dest[], source[], maxlength=sizeof(dest))*/
static cell AMX_NATIVE_CALL ssocket_strunpack(AMX *amx, cell *params)
{
	char *src;
	cell *dest;
	int maxlength = params[3];

	amx_GetAddr(amx, params[2], &dest);
	src = (char*) dest;
	amx_GetAddr(amx, params[1], &dest);
	while (maxlength--) {
		*dest = *(src++);
		if (*dest == 0) {
			return 1;
		}
		dest++;
	}
	*(dest - 1) = 0;
	return 1;
}

static AMX_NATIVE_INFO PluginNatives[] =
{
	{ "ssocket_connect", ssocket_connect },
	{ "ssocket_create", ssocket_create },
	{ "ssocket_destroy", ssocket_destroy },
	{ "ssocket_listen", ssocket_listen },
	{ "ssocket_send", ssocket_send },
	{ "ssocket_set_recv_wait", ssocket_set_recv_wait },
	{ "ssocket_strunpack", ssocket_strunpack },
	{0, 0}
};

PLUGIN_EXPORT int PLUGIN_CALL ProcessTick()
{
	struct SOCKET *s;
	static struct sockaddr_in remote_client;
	static
#ifdef WIN32
        int
#else
	size_t
#endif
	client_len = sizeof(remote_client);
	static char buf[2048];
	int recvsize;
	cell data, result;

	waitvalue++;
	if (waitvalue > wait) {
		waitvalue = 0;
		s = sockets;
		while (s != sockets_end) {
			if (s->handle != -1) {
				recvsize = recvfrom(
					s->handle, buf, 2048,
#ifdef WIN32
					0,
#else
					MSG_DONTWAIT,
#endif
					(struct sockaddr*) &remote_client,
					&client_len);
				if (recvsize > 0) {
					amx_Push(s->amx, recvsize);
					amx_PushArray(
						s->amx, &data, NULL,
						(cell*) buf,
						sizeof(buf)/sizeof(cell));
					amx_Push(s->amx, s - sockets);
					amx_Exec(s->amx, &result, s->callback);
					amx_Release(s->amx, data);
				}
			}
			s++;
		}
	}
	return 1;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
	struct SOCKET *s = sockets;

#ifdef WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) ||
		LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		logprintf("cannot initialize winsock: %d\n", WSAGetLastError());
		return 0;
	}
#endif

	wait = waitvalue = 0;
	while (s != sockets_end) {
		s->handle = -1;
		s++;
	}

	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t) ppData[PLUGIN_DATA_LOGPRINTF];
	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	struct SOCKET *s = sockets;
	int closed = 0;

	while (s != sockets_end) {
		if (s->handle != -1) {
			_ssocket_close(&s->handle);
			closed++;
		}
		s++;
	}
	if (closed) {
		logprintf("simplesocket: closed %d unclosed sockets", closed);
	}
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	return amx_Register(amx, PluginNatives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	struct SOCKET *s = sockets;
	int closed = 0;

	while (s != sockets_end) {
		if (s->handle != -1 && s->amx == amx) {
			_ssocket_close(&s->handle);
			closed++;
		}
		s++;
	}
	if (closed) {
		logprintf(
			"simplesocket: closed %d opened sockets registered "
			"by unloaded script", closed);
	}
	return AMX_ERR_NONE;
}

