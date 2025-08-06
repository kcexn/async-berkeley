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
#include <streambuf>
#include <memory>
#include <array>
#include <vector>
#include <tuple>
#include <sys/socket.h>

#pragma once
#ifndef IO_BUFFERS
#define IO_BUFFERS
namespace io{
    namespace buffers{
        struct socket_message {
            using address_type = std::tuple<struct sockaddr_storage, socklen_t>;
            using ancillary_buffer = std::vector<char>;
            using data_buffer = struct iovec;

            struct msghdr header;
            address_type addr;
            ancillary_buffer ancillary;
            data_buffer data;
        };
        class sockbuf : public std::streambuf {
            public:
                using Base = std::streambuf;
                using pos_type = Base::pos_type;
                using off_type = Base::off_type;
                using int_type = Base::int_type;
                using traits_type = Base::traits_type;
                using char_type = Base::char_type;
                using size_type = std::size_t;
                using buffer_type = std::shared_ptr<socket_message>;
                using buffers_type = std::vector<buffer_type>;
                using native_handle_type = int;
                static constexpr native_handle_type BAD_SOCKET = -1;
                static constexpr size_type MIN_BUFSIZE = 32*1024;

                sockbuf();
                explicit sockbuf(native_handle_type sockfd, bool connected=false, std::ios_base::openmode which=(std::ios_base::in | std::ios_base::out));
                explicit sockbuf(int domain, int type, int protocol, std::ios_base::openmode which=(std::ios_base::in | std::ios_base::out));
                buffer_type connectto(const struct sockaddr *addr, socklen_t addrlen);

                const buffer_type& recvbuf() const { return _buffers.front(); }
                const buffer_type& sendbuf() const { return _buffers.back(); }
                native_handle_type& native_handle() { return _socket; }
                int& err() { return _errno; }
                const int& err() const { return _errno; }

                ~sockbuf();

                sockbuf(const sockbuf& other) = delete;
                sockbuf& operator=(const sockbuf& other) = delete;
                sockbuf(sockbuf&& other) = delete;
                sockbuf& operator=(sockbuf&& other) = delete;

            protected:
                virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
                virtual pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
                virtual int sync() override;
                virtual std::streamsize showmanyc() override;

                virtual int_type overflow(int_type ch = traits_type::eof()) override;
                virtual int_type underflow() override;
            private:
                buffers_type _buffers;
                native_handle_type _socket;
                int _errno;
                bool _connected;
                std::ios_base::openmode _which;

                void _init_buf_ptrs();
                int _send(const buffer_type& buf);
                void _resizewbuf(const buffer_type& buf, void *iov_base=nullptr, std::size_t buflen=0);
                void _memmoverbuf();
                int _recv();
        };
    }
}
#endif
