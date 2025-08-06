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
#include "io.hpp"
namespace io{
    poller::size_type poller::_add(
        native_handle_type handle,
        events_type& events,
        event_type event
    ){
        auto begin = events.begin(), end = events.end();
        auto lb = std::lower_bound(begin, end, event,
            [](const auto& lhs, const auto& rhs) {
                return lhs.fd < rhs.fd;
            }
        );
        if(lb == end || lb->fd != handle) {
            events.insert(lb, event);
            shrink_to_fit(events);
            return events.size();
        }
        return npos;
    }

    poller::size_type poller::_update(
        native_handle_type handle,
        events_type& events,
        event_type event
    ){
        auto begin = events.begin(), end = events.end();
        auto lb = std::lower_bound(
            begin,
            end,
            event,
            [](const auto& lhs, const auto& rhs) {
                return lhs.fd < rhs.fd;
            }
        );
        if(lb == end || lb->fd != handle)
            return npos;
        lb->events = event.events;
        return events.size();
    }

    poller::size_type poller::_del(native_handle_type handle, events_type& events) {
        auto begin = events.begin(), end = events.end();
        auto lb = std::lower_bound(
            begin,
            end,
            handle,
            [](const auto& lhs, const native_handle_type& rhs) {
                return lhs.fd < rhs;
            }
        );
        if(lb == end || lb->fd != handle)
            return npos;
        events.erase(lb);
        return events.size();
    }

    poller::size_type poller::_poll(const duration_type& timeout)
    {
        int nfds = 0;
        if( (nfds=poll(events().data(), events().size(), timeout.count())) < 0 )
        {
            switch(errno)
            {
                case EAGAIN:
                case EINTR:
                    return 0;
                default:
                    return npos;
            }
        }
        return nfds;
    }
}
