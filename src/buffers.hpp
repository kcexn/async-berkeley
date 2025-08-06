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
#pragma once
#ifndef IOSCHED_BUFFERS
#define IOSCHED_BUFFERS
#include "socket.hpp"

#include <memory>
#include <streambuf>

namespace iosched::buffers {
    using socket_message = ::iosched::socket::socket_message;
    class sockbuf : public std::streambuf {
        public:
            using Base = std::streambuf;
            using size_type = std::size_t;
            using buffer_type = std::shared_ptr<socket_message>;
            using buffers_type = std::vector<buffer_type>;
            using native_handle_type = int;
            static constexpr native_handle_type BAD_SOCKET = -1;
            static constexpr size_type MIN_BUFSIZE = 32UL*1024;

            sockbuf();
            explicit sockbuf(native_handle_type sockfd, bool connected=false, std::ios_base::openmode which=(std::ios_base::in | std::ios_base::out));
            explicit sockbuf(int domain, int type, int protocol, std::ios_base::openmode which=(std::ios_base::in | std::ios_base::out));
            auto connectto(const struct sockaddr *addr, socklen_t addrlen) -> buffer_type;

            [[nodiscard]] auto recvbuf() const -> const buffer_type& { return _buffers.front(); }
            [[nodiscard]] auto sendbuf() const -> const buffer_type& { return _buffers.back(); }
            auto native_handle() -> native_handle_type& { return _socket; }
            auto err() -> int& { return _errno; }
            [[nodiscard]] auto err() const -> const int& { return _errno; }

            ~sockbuf();

            sockbuf(const sockbuf& other) = delete;
            auto operator=(const sockbuf& other) -> sockbuf& = delete;
            sockbuf(sockbuf&& other) = delete;
            auto operator=(sockbuf&& other) -> sockbuf& = delete;

        protected:
            auto seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) -> pos_type override;
            auto seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) -> pos_type override;
            auto sync() -> int override;
            auto showmanyc() -> std::streamsize override;

            auto overflow(int_type character = traits_type::eof()) -> int_type override;
            auto underflow() -> int_type override;
        private:
            buffers_type _buffers;
            native_handle_type _socket;
            int _errno;
            bool _connected;
            std::ios_base::openmode _which;

            void _init_buf_ptrs();
            auto _send(const buffer_type& buf) -> int;
            void _resizewbuf(const buffer_type& buf, void *iov_base=nullptr, std::size_t buflen=0);
            void _memmoverbuf();
            auto _recv() -> int;
    };
}

#endif
