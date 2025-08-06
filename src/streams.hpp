/* Copyright 2025 Kevin Exton
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "buffers.hpp"
#include <iostream>
#pragma once
#ifndef IO_STREAMS
#define IO_STREAMS
namespace iosched {
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
