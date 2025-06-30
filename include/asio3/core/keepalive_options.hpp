/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <tuple>

#include <asio3/core/predef.h>
#include <asio3/core/error.hpp>
#include <asio3/core/netutil.hpp>

#if ASIO3_OS_WINDOWS
#	if __has_include(<Mstcpip.h>)
#		include <Mstcpip.h> // tcp_keepalive struct
#	endif
#endif

#ifdef ASIO_STANDALONE
namespace asio::detail
#else
namespace boost::asio::detail
#endif
{
	/**
	 * @brief set tcp socket keep alive options
	 * @param onoff    - Turn keepalive on or off.
	 * @param idle     - How many seconds after the connection is idle, start sending keepalives.
	 * @param interval - How many seconds later to send again when no reply is received.
	 * @param count    - How many times to resend when no reply is received.
	 * @li on macOS Catalina 10.15.5 (19F101), the default value is:
	 * onoff - false, idle - 7200, interval - 75, count - 8
	 */
	template<class SocketT>
	requires is_tcp_socket<SocketT>
	asio::error_code set_keepalive_options(
		SocketT&     socket,
		bool         onoff = true,
		unsigned int idle = 60,
		unsigned int interval = 3,
		unsigned int count = 3
	) noexcept
	{
		if (!socket.is_open())
		{
			return asio::error::not_connected;
		}

		error_code ec;

		asio::socket_base::keep_alive option(onoff);
		socket.set_option(option, ec);

		if (ec)
		{
			return ec;
		}

		auto native_fd = socket.native_handle();

		asio::ignore_unused(onoff, idle, interval, count, native_fd);

	#if ASIO3_OS_LINUX
		// For *n*x systems
		int ret_keepidle  = setsockopt(native_fd, SOL_TCP, TCP_KEEPIDLE , (void*)&idle    , sizeof(unsigned int));
		if (ret_keepidle)
		{
			return asio::error_code{ errno, asio::error::get_system_category() };
		}

		int ret_keepintvl = setsockopt(native_fd, SOL_TCP, TCP_KEEPINTVL, (void*)&interval, sizeof(unsigned int));
		if (ret_keepintvl)
		{
			return asio::error_code{ errno, asio::error::get_system_category() };
		}

		int ret_keepcount = setsockopt(native_fd, SOL_TCP, TCP_KEEPCNT  , (void*)&count   , sizeof(unsigned int));
		if (ret_keepcount)
		{
			return asio::error_code{ errno, asio::error::get_system_category() };
		}
	#elif ASIO3_OS_UNIX
		// be pending
	#elif ASIO3_OS_MACOS
		int ret_keepalive = setsockopt(native_fd, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&idle    , sizeof(unsigned int));
		if (ret_keepalive)
		{
			return asio::error_code{ errno, asio::error::get_system_category() };
		}

		int ret_keepintvl = setsockopt(native_fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&interval, sizeof(unsigned int));
		if (ret_keepintvl)
		{
			return asio::error_code{ errno, asio::error::get_system_category() };
		}

		int ret_keepcount = setsockopt(native_fd, IPPROTO_TCP, TCP_KEEPCNT  , (void*)&count   , sizeof(unsigned int));
		if (ret_keepcount)
		{
			return asio::error_code{ errno, asio::error::get_system_category() };
		}
	#elif ASIO3_OS_IOS
		// be pending
	#elif ASIO3_OS_WINDOWS
		// on WSL ubuntu, the ASIO3_OS_WINDOWS is true, but can't find the <Mstcpip.h> file.
		#if __has_include(<Mstcpip.h>)
			// Partially supported on windows
			tcp_keepalive keepalive_options;
			keepalive_options.onoff = onoff;
			keepalive_options.keepalivetime = idle * 1000; // Keep Alive in milliseconds.
			keepalive_options.keepaliveinterval = interval * 1000; // Resend if No-Reply 

			DWORD bytes_returned = 0;

			if (SOCKET_ERROR == ::WSAIoctl(native_fd, SIO_KEEPALIVE_VALS, (LPVOID)&keepalive_options,
				(DWORD)sizeof(keepalive_options), nullptr, 0, (LPDWORD)&bytes_returned, nullptr, nullptr))
			{
				if (::WSAGetLastError() != WSAEWOULDBLOCK)
				{
					return asio::error_code{ ::WSAGetLastError(), asio::error::get_system_category() };
				}
			}
		#endif
	#endif
		return asio::error_code{};
	}

	template<class SocketT>
	requires (!is_tcp_socket<SocketT>)
	asio::error_code set_keepalive_options(
		SocketT&     socket,
		bool         onoff = true,
		unsigned int idle = 60,
		unsigned int interval = 3,
		unsigned int count = 3
	) noexcept
	{
		asio::ignore_unused(socket, onoff, idle, interval, count);

		return asio::error_code{};
	}
}
