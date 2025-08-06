/*
*   Copyright 2024 Kevin Exton
*   This file is part of Cloudbus.
*
*   Cloudbus is free software: you can redistribute it and/or modify it under the
*   terms of the GNU General Public License as published by the Free Software
*   Foundation, either version 3 of the License, or any later version.
*
*   Cloudbus is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
*   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*   See the GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along with Cloudbus.
*   If not, see <https://www.gnu.org/licenses/>.
*/
#include "buffers.hpp"
#include <iostream>
#pragma once
#ifndef IO_STREAMS
#define IO_STREAMS
namespace io {
    namespace streams {
        class sockstream: public std::iostream {
            using Base = std::iostream;
            using sockbuf = buffers::sockbuf;
            sockbuf _buf;

            public:
                using native_handle_type = sockbuf::native_handle_type;
                static constexpr native_handle_type BAD_SOCKET = sockbuf::BAD_SOCKET;
                
                sockstream():
                    Base(&_buf), _buf{}
                {}
                explicit sockstream(native_handle_type sockfd, bool connected=false, std::ios_base::openmode which=(std::ios_base::in | std::ios_base::out)):
                    Base(&_buf), _buf(sockfd, connected, which)
                {}
                explicit sockstream(int domain, int type, int protocol, std::ios_base::openmode which=(std::ios_base::in | std::ios_base::out)):
                    Base(&_buf), _buf(domain, type, protocol, which)
                {}

                const sockbuf::buffer_type& recvbuf() const { return _buf.recvbuf(); }
                const sockbuf::buffer_type& sendbuf() const { return _buf.sendbuf(); }
                native_handle_type& native_handle() { return _buf.native_handle(); }
                int& err() { return _buf.err(); }
                const int& err() const { return _buf.err(); }
                sockbuf::buffer_type connectto(const struct sockaddr* addr, socklen_t len) { return _buf.connectto(addr, len); }

                ~sockstream() = default;

                sockstream(const sockstream& other) = delete;
                sockstream& operator=(const sockstream& other) = delete;
                sockstream(sockstream&& other) = delete;
                sockstream& operator=(sockstream&& other) = delete;
        };
    }
}
#endif
