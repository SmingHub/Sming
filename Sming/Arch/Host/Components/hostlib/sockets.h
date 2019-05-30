/**
 * sockets.h - C++ socket support for Windows / Linux
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

#pragma once

#include "hostlib.h"
#include "threads.h"
#include <vector>
#include <string>

#ifdef __WIN32
#include <ws2tcpip.h>

#define MSG_NOSIGNAL 0
#define MSG_DONTWAIT 0x40
#define SHUT_RDWR SD_BOTH
typedef char* sock_ptr_t;
typedef int socklen_t;

#else

#include <arpa/inet.h>
typedef void* sock_ptr_t;

#endif

// CSocket::wait flags
#define SOCKET_WAIT_READ 0x01
#define SOCKET_WAIT_WRITE 0x02

// Call exactly once for the application
void sockets_initialise();
void sockets_finalise();
std::string socket_strerror();

class CSockAddr
{
private:
	union {
		short sin_family;
		struct sockaddr sa;
		struct sockaddr_in in4;  // AF_INET
		struct sockaddr_in6 in6; // AF_INET6
	} m_addr;

public:
	CSockAddr()
	{
		clear();
	}

	CSockAddr(const char* addr, unsigned port)
	{
		assign(addr, port);
	}

	CSockAddr(const struct sockaddr& addr)
	{
		m_addr.sa = addr;
	}

	CSockAddr(const struct sockaddr_in& addr)
	{
		m_addr.in4 = addr;
	}

	bool assign(const char* addr, unsigned port);

	bool get_host(int fd);
	bool get_peer(int fd);

	void clear()
	{
		m_addr = {};
	}

	std::string text() const;

	const struct sockaddr& addr() const
	{
		return m_addr.sa;
	}
};

class CSocketList;

class CSocket
{
public:
	CSocket(int type = SOCK_STREAM, int protocol = 0) : m_type(type), m_protocol(protocol), m_fd(0)
	{
	}

	CSocket(int fd, const CSockAddr& addr) : CSocket()
	{
		assign(fd, addr);
	}

	virtual ~CSocket()
	{
		close();
	}

	virtual void close();

	bool setblocking(bool block);
	bool bind(const CSockAddr& sa);
	bool connect(const CSockAddr& sa);
	bool active() const
	{
		return m_fd > 0;
	}

	void assign(int fd, const CSockAddr& addr)
	{
		if(fd != m_fd) {
			close();
			m_fd = fd;
		}
		m_addr = addr;
	}

	CSockAddr get_host() const
	{
		CSockAddr addr;
		addr.get_host(m_fd);
		return addr;
	}

	CSockAddr get_peer() const
	{
		CSockAddr addr;
		addr.get_peer(m_fd);
		return addr;
	}

	const CSockAddr& addr() const
	{
		return m_addr;
	}

	int send(const void* data, size_t n, int flags = MSG_NOSIGNAL);

	int available();
	bool wait(unsigned timeout_ms, unsigned flags = SOCKET_WAIT_READ);
	int recv(void* buf, size_t n, int flags = 0);

	// If we take a copy of this socket this ensures it doesn't get closed
	void release()
	{
		m_fd = 0;
	}

protected:
	bool create();

protected:
	// Type of socket: SOCK_STREAM, SOCK_DGRAM, etc.
	int m_type;
	// Typically 0, or some other value if type is SOCK_RAW
	int m_protocol;
	//
	int m_fd;
	CSockAddr m_addr;
};

class CSocketList : public std::vector<CSocket*>
{
public:
	~CSocketList();

	void lock()
	{
		m_mutex.lock();
	}

	void unlock()
	{
		m_mutex.unlock();
	}

	void add(CSocket* skt);

	void send(const void* data, size_t n, int flags = MSG_NOSIGNAL);

	CSocket* recv(void* buf, size_t& n);

	unsigned active();

	void closeall();

private:
	CMutex m_mutex;
};

class CServerSocket : public CSocket
{
public:
	CServerSocket(int type = SOCK_STREAM) : CSocket(type), m_max_connections(1)
	{
	}

	void close() override
	{
		m_clients.closeall();
		CSocket::close();
	}

	bool listen(const CSockAddr& addr, unsigned max_connections);

	CSocket* try_connect();

protected:
	// Inherited classes override this - return false to reject connection
	virtual CSocket* new_connection(int& fd, const CSockAddr& addr)
	{
		return new CSocket(fd, addr);
	}

private:
	unsigned m_max_connections;
	CSocketList m_clients;
};
