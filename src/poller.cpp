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
#include "io.hpp"
namespace iosched{
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
