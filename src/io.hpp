/*
*   Copyright 2025 Kevin Exton
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
#include "streams.hpp"
#include <algorithm>
#include <chrono>
#include <poll.h>

#pragma once
#ifndef IO
#define IO
namespace io {
    namespace {
        template<class T>
        static bool shrink_to_fit(std::vector<T>& vec){
            static constexpr std::size_t THRESH = 256;
            bool resized = false;
            const std::size_t capacity = vec.capacity();
            if( capacity > THRESH &&
                vec.size() < capacity/8
            ){
                vec  = std::vector<T>(
                    std::make_move_iterator(vec.begin()),
                    std::make_move_iterator(vec.end())
                );
                resized = true;
            }
            return resized;
        }
    }
    struct poll_t {
        using native_handle_type = int;
        using event_type = struct pollfd;
        using events_type = std::vector<event_type>;
        using event_mask = short;
        static event_type mkevent(
            const native_handle_type& hnd,
            const event_mask& events,
            const event_mask& revents=0
        ){
            return event_type{hnd, events, revents};
        }
    };

    template<class PollT>
    struct poll_traits : public PollT
    {
        using Base = PollT;
        using signal_type = sigset_t;
        using size_type = std::size_t;
        using duration_type = std::chrono::milliseconds;
        static const size_type npos = -1;
    };

    template<class PollT, class Traits = poll_traits<PollT> >
    class basic_poller {
        public:
            using native_handle_type = typename Traits::native_handle_type;
            using signal_type = typename Traits::signal_type;
            using size_type = typename Traits::size_type;
            using duration_type = typename Traits::duration_type;
            using event_type = typename Traits::event_type;
            using events_type = typename Traits::events_type;
            using event_mask = typename Traits::event_mask;
            static const size_type npos = Traits::npos;

            basic_poller() = default;

            size_type operator()(duration_type timeout = duration_type(0)){ return _poll(timeout); }

            size_type add(native_handle_type handle, event_type event){ return _add(handle, _events, event); }
            size_type update(native_handle_type handle, event_type event){ return _update(handle, _events, event); }
            size_type del(native_handle_type handle) { return _del(handle, _events); }

            events_type& events() { return _events; }
            const events_type& events() const { return _events; }

            virtual ~basic_poller() = default;

            basic_poller(const basic_poller& other) = delete;
            basic_poller(basic_poller&& other) = delete;
            basic_poller& operator=(const basic_poller& other) = delete;
            basic_poller& operator=(basic_poller&& other) = delete;

        protected:
            virtual size_type _add(native_handle_type handle, events_type& events, event_type event) { return npos; }
            virtual size_type _update(native_handle_type handle, events_type& events, event_type event) { return npos; }
            virtual size_type _del(native_handle_type handle, events_type& events ) { return npos; }
            virtual size_type _poll(const duration_type& timeout) { return npos; }

        private:
            events_type _events{};
    };

    class poller: public basic_poller<poll_t> {
        public:
            using Base = basic_poller<poll_t>;
            using size_type = Base::size_type;
            using duration_type = Base::duration_type;
            using event_type = Base::event_type;
            using events_type = Base::events_type;
            using event_mask = Base::event_mask;

            poller(): Base(){}
            virtual ~poller() = default;

            poller(const poller& other) = delete;
            poller(poller&& other) = delete;
            poller& operator=(const poller& other) = delete;
            poller& operator=(poller&& other) = delete;

        protected:
            size_type _add(native_handle_type handle, events_type& events, event_type event) override;
            size_type _update(native_handle_type handle, events_type& events, event_type event) override;
            size_type _del(native_handle_type handle, events_type& events ) override;
            size_type _poll(const duration_type& timeout) override;
    };

    template<class PollT, class Traits = poll_traits<PollT> >
    class basic_trigger {
        public:
            using poller_type = basic_poller<PollT>;
            using traits_type = Traits;
            using native_handle_type = typename traits_type::native_handle_type;
            using signal_type = typename traits_type::signal_type;
            using size_type = typename traits_type::size_type;
            using duration_type = typename traits_type::duration_type;
            using event_type = typename traits_type::event_type;
            using events_type = typename traits_type::events_type;
            using event_mask = typename traits_type::event_mask;
            using trigger_type = std::uint32_t;
            using interest_type = std::tuple<native_handle_type, trigger_type>;
            using interest_list = std::vector<interest_type>;
            static const size_type npos = traits_type::npos;

            explicit basic_trigger(poller_type& poller): _poller{poller}{}

            size_type set(native_handle_type handle, trigger_type trigger) {
                auto begin = _list.begin(), end = _list.end();
                auto lb = std::lower_bound(
                    begin,
                    end,
                    handle,
                    [](const auto& lhs, const native_handle_type& rhs) {
                        return std::get<native_handle_type>(lhs) < rhs;
                    }
                );
                if(lb == end || std::get<native_handle_type>(*lb) != handle) {
                    _list.insert(lb, interest_type{handle, trigger});
                    shrink_to_fit(_list);
                    return _poller.add(handle, traits_type::mkevent(handle, trigger));
                }
                auto& trig = std::get<trigger_type>(*lb);
                if( (trig & trigger) != trigger )
                    return _poller.update(handle, traits_type::mkevent(handle, trig |= trigger));
                return _list.size();
            }

            size_type clear(native_handle_type handle, trigger_type trigger=UINT32_MAX) {
                auto begin = _list.begin(), end = _list.end();
                auto lb = std::lower_bound(
                    begin,
                    end,
                    handle,
                    [](const auto& lhs, const native_handle_type& rhs) {
                        return std::get<native_handle_type>(lhs) < rhs;
                    }
                );
                if(lb == end || std::get<native_handle_type>(*lb) != handle)
                    return npos;
                auto& trig = std::get<trigger_type>(*lb);
                if( !(trig & ~trigger) ) {
                    _list.erase(lb);
                    return _poller.del(handle);
                }
                if( (trig & ~trigger) != trig )
                    return _poller.update(handle, traits_type::mkevent(handle, trig &= ~trigger));
                return _list.size();
            }

            size_type wait(duration_type timeout = duration_type(0)){ return _poller(timeout); }
            const interest_list& list() const { return _list; }

            const events_type& events() const {
                return _poller.events();
            }

            virtual ~basic_trigger() = default;

            basic_trigger() = delete;
            basic_trigger(const basic_trigger& other) = delete;
            basic_trigger(basic_trigger&& other) = delete;
            basic_trigger& operator=(const basic_trigger& other) = delete;
            basic_trigger& operator=(const basic_trigger&& other) = delete;

        private:
            interest_list _list;
            poller_type& _poller;
    };

    class trigger: public basic_trigger<poll_t> {
        public:
            using Base = basic_trigger<poll_t>;
            using native_handle_type = Base::native_handle_type;
            using trigger_type = Base::trigger_type;
            using event_type = Base::event_type;
            using events_type = Base::events_type;
            using event_mask = Base::event_mask;
            using size_type = Base::size_type;

            trigger(): Base(_poller){}
            virtual ~trigger() = default;

            trigger(const trigger& other) = delete;
            trigger(trigger&& other) = delete;
            trigger& operator=(const trigger& other) = delete;
            trigger& operator=(trigger&& other) = delete;

        private:
            poller _poller;
    };

    template<class TriggerT>
    class basic_handler {
        public:
            using trigger_type = TriggerT;
            using event_type = typename trigger_type::event_type;
            using events_type = typename trigger_type::events_type;
            using event_mask = typename trigger_type::event_mask;
            using size_type = typename trigger_type::size_type;

            explicit basic_handler(trigger_type& triggers):
                _triggers{triggers}{}

            size_type handle(events_type& events) { return _handle(events); }
            trigger_type& triggers() { return _triggers; }

            virtual ~basic_handler() = default;

            basic_handler(const basic_handler& other) = delete;
            basic_handler(basic_handler&& other) = delete;
            basic_handler& operator=(const basic_handler& other) = delete;
            basic_handler& operator=(basic_handler&& other) = delete;

        protected:
            virtual size_type _handle(events_type& events) { return trigger_type::npos; }

        private:
            trigger_type& _triggers;
    };
}
#endif