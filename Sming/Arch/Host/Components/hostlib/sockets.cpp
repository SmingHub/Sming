/**
 * sockets.cpp
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the Sming Framework Project
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with SHEM.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "sockets.h"
#include "hostmsg.h"
#include <string.h>

#ifndef __WIN32
// For FIONREAD
#include <sys/ioctl.h>
#include <poll.h>
#endif

void sockets_initialise()
{
#ifdef __WIN32
	WORD versionWanted = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(versionWanted, &wsaData);
#endif
}

void sockets_finalise()
{
#ifdef __WIN32
	WSACleanup();
#endif
}

static void socket_close(int fd)
{
	if(fd <= 0) {
		return;
	}

	::shutdown(fd, SHUT_RDWR);
#ifdef __WIN32
	::closesocket(fd);
#else
	::close(fd);
#endif
}

static bool socket_blocking(int fd, bool block)
{
#ifdef __WIN32
	u_long iMode = block ? 0 : 1;
	return ioctlsocket(fd, FIONBIO, &iMode) == 0;
#else
	int fl = fcntl(fd, F_GETFL);
	int fl_new = block ? (fl & ~O_NONBLOCK) : (fl | O_NONBLOCK);
	if(fl_new == fl)
		return true;
	return fcntl(fd, F_SETFL, fl_new) == 0;
#endif
}

/*
 * CSockAddr
 */

std::string CSockAddr::text() const
{
	if(m_addr.sin_family != AF_INET) {
		return "(not ip4: " + std::to_string(m_addr.sin_family) + ")";
	}

	char* addr = inet_ntoa(m_addr.in4.sin_addr);
	if(addr == nullptr) {
		return "";
	}

	return std::string(addr) + ':' + std::to_string(ntohs(m_addr.in4.sin_port));
}

bool CSockAddr::assign(const char* addr, unsigned port)
{
	if(port > 0xFFFF) {
		return false;
	}
	m_addr.in4.sin_port = htons(port);
	m_addr.in4.sin_family = AF_INET;
	uint32_t ina = (addr == nullptr || *addr == '\0') ? 0 : inet_addr(addr);
	m_addr.in4.sin_addr.s_addr = ina;
	return ina != INADDR_NONE;
}

bool CSockAddr::get_host(int fd)
{
	clear();
	socklen_t len = sizeof(m_addr.sa);
	return (getsockname(fd, &m_addr.sa, &len) == 0);
}

bool CSockAddr::get_peer(int fd)
{
	clear();
	socklen_t len = sizeof(m_addr.sa);
	return (getpeername(fd, &m_addr.sa, &len) == 0);
}

/*
 * CSocket
 */

std::string socket_strerror()
{
	char buf[256];
	buf[0] = '\0';
	int ErrorCode;
#ifdef __WIN32
	ErrorCode = WSAGetLastError();
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY, nullptr,
				   ErrorCode, 0, buf, sizeof(buf), nullptr);
#else
	ErrorCode = errno;
	char* res = strerror_r(ErrorCode, buf, sizeof(buf));
	if(res == nullptr) {
		strcpy(buf, "Unknown");
	}
#endif
	return buf[0] ? buf : std::string("Error #" + std::to_string(ErrorCode));
}

bool CSocket::create()
{
	if(m_fd > 0) {
		return true;
	}

	// creation of the socket
	m_fd = ::socket(AF_INET, m_type, 0);
	if(m_fd <= 0) {
		hostmsg("%s", socket_strerror().c_str());
		return false;
	}

	int reuse = 1;
	if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (sock_ptr_t)&reuse, sizeof(reuse)) < 0) {
		hostmsg("REUSEADDR: %s", socket_strerror().c_str());
		close();
		return false;
	}

#ifdef SO_REUSEPORT
	if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, (sock_ptr_t)&reuse, sizeof(reuse)) < 0) {
		hostmsg("REUSEPORT: %s", socket_strerror().c_str());
		close();
		return false;
	}
#endif

	return true;
}

bool CSocket::setblocking(bool block)
{
	return socket_blocking(m_fd, block);
}

bool CSocket::bind(const CSockAddr& sa)
{
	if(!create()) {
		return false;
	}

	m_addr = sa;
	return ::bind(m_fd, &sa.addr(), sizeof(struct sockaddr)) == 0;
}

bool CSocket::connect(const CSockAddr& sa)
{
	bool res = create();

	if(!res) {
		return false;
	}

	m_addr = sa;
	if(::connect(m_fd, &sa.addr(), sizeof(struct sockaddr))) {
		//!! TODO: Implement event notifications for everything (connect, close, data ready, etc.)
#ifdef __WIN32
		if(WSAGetLastError() != WSAEWOULDBLOCK)
#else
		if(errno != EAGAIN)
#endif
			res = false;
	}

	return res;
}

void CSocket::close()
{
	if(m_fd <= 0) {
		return;
	}

	hostmsg("%s", addr().text().c_str());

	socket_close(m_fd);
	m_fd = 0;
}

// Returns available data to receive
int CSocket::available()
{
	u_long av = 0;
#ifdef __WIN32
	int res = ioctlsocket(m_fd, FIONREAD, &av);
#else
	int res = ioctl(m_fd, FIONREAD, &av);
#endif
	return (res == 0) ? int(av) : -1;
}

/*
 * If we rebuffer then the receiver has to deal with fragmented data packets.
 * For command use data is small so that will never happen, but with video it's likely.
 */
int CSocket::send(const void* data, size_t n, int flags)
{
	int sent = 0;
	while(n != 0) {
		int ret = ::send(m_fd, static_cast<const char*>(data) + sent, n, flags);
		if(ret < 0) {
			return ret;
		}
		sent += ret;
		if(size_t(ret) > n) {
			break;
		}
		n -= ret;
	}
	return sent;
}

bool CSocket::wait(unsigned timeout_ms, unsigned flags)
{
#ifdef __WIN32
	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_fd, &set);

	struct timeval tv;
	tv.tv_sec = long(timeout_ms) / 1000;
	tv.tv_usec = (timeout_ms - tv.tv_sec * 1000) * 1000;

	auto rdSet = (flags & SOCKET_WAIT_READ) ? &set : nullptr;
	auto wrSet = (flags & SOCKET_WAIT_WRITE) ? &set : nullptr;
	return select(m_fd + 1, rdSet, wrSet, nullptr, &tv) == 1;

#else
	short events = 0;
	if(flags & SOCKET_WAIT_READ) {
		events |= POLLIN;
	}
	if(flags & SOCKET_WAIT_WRITE) {
		events |= POLLOUT;
	}
	struct pollfd pfd = {m_fd, events, 0};
	return poll(&pfd, 1, timeout_ms) == 1;
#endif
}

int CSocket::recv(void* buf, size_t n, int flags)
{
#ifdef __WIN32
	// Emulate MSG_NOWAIT behaviour
	if(flags & MSG_DONTWAIT) {
		int av = available();
		if(av < 0) {
			return av;
		}
		if(av == 0) {
			WSASetLastError(WSAEWOULDBLOCK);
			return -1;
		}
		if(size_t(av) < n) {
			n = size_t(av);
		}
		flags &= ~MSG_DONTWAIT;
	}
#endif

	return ::recv(m_fd, (char*)buf, n, flags);
}

/*
 * CSocketList
 */

CSocketList::~CSocketList()
{
	for(auto& s : *this) {
		delete s;
		s = nullptr;
	}
}

void CSocketList::add(CSocket* skt)
{
	lock();
	for(auto& s : *this) {
		if(!s->active()) {
			delete s;
			s = skt;
			skt = nullptr;
			break;
		}
	}

	if(skt != nullptr) {
		push_back(skt);
	}
	unlock();
}

void CSocketList::send(const void* data, size_t n, int flags)
{
	lock();
	for(auto& skt : *this) {
		if(!skt->active()) {
			continue;
		}

		int num_sent = skt->send(data, n, flags);
		if(num_sent != (int)n) {
#ifndef __WIN32
			if(errno != EPIPE)
				perror(__FUNCTION__);
#endif
			skt->close();
		}
	}
	unlock();
}

CSocket* CSocketList::recv(void* buf, size_t& n)
{
	lock();
	for(auto& skt : *this) {
		if(!skt->active()) {
			continue;
		}

		int num_recv = skt->recv(buf, n, MSG_DONTWAIT);
		if(num_recv > 0) {
			n = num_recv;
			unlock();
			return skt;
		}

		if(num_recv < 0)
#ifdef __WIN32
			if(WSAGetLastError() != WSAEWOULDBLOCK) {
#else
			if(errno != EAGAIN) {
#endif
				if(errno != EPIPE) {
					// Broken pipe
					hostmsg("%s", socket_strerror().c_str());
				}
				skt->close();
			}
	}
	unlock();
	return nullptr;
}

unsigned CSocketList::active()
{
	lock();
	unsigned n = 0;
	for(auto& skt : *this) {
		if(skt->active())
			++n;
	}
	unlock();
	return n;
}

void CSocketList::closeall()
{
	for(auto& s : *this) {
		s->close();
	}
}

/*
 * CServerSocket
 */

bool CServerSocket::listen(const CSockAddr& addr, unsigned max_connections)
{
	if(!create()) {
		return false;
	}

	if(!bind(addr)) {
		return false;
	}

	if(::listen(m_fd, 3)) {
		return false;
	}

	m_max_connections = max_connections;

	return setblocking(false);
}

CSocket* CServerSocket::try_connect()
{
	// Initialised?
	if(m_fd <= 0) {
		return nullptr;
	}

	// Ignore further attempts until we're ready
	if(m_clients.active() >= m_max_connections) {
		return nullptr;
	}

	struct sockaddr sa;
	socklen_t len = sizeof(sa);
	int fd = ::accept(m_fd, &sa, &len);
	if(fd <= 0) {
		return nullptr;
	}

	if(socket_blocking(fd, false)) {
		CSocket* skt = new_connection(fd, CSockAddr(sa));
		if(skt) {
			m_clients.add(skt);
			return skt;
		}
	}

	socket_close(fd);
	return nullptr;
}
