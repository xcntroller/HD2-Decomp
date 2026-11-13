#ifndef HD2_HOST_CONTROL_CONNECTION_PLATFORM_H
#define HD2_HOST_CONTROL_CONNECTION_PLATFORM_H
#include "frontend/control_connection_socket.h"
#include <winsock2.h>
#include <windows.h>
#include <cstdio>

namespace hd2::host {
class ControlConnectionPlatform {
    class Thread final:public frontend::ControlConnectionThread {
    public:
        explicit Thread(std::function<void()> body):body_(std::move(body)){}
        ~Thread()override{if(handle_){(void)WaitForSingleObject(handle_,INFINITE);CloseHandle(handle_);}}
        bool launch(std::uint32_t& identity){DWORD id=0;handle_=CreateThread(nullptr,0,entry,this,0,&id);identity=id;return handle_!=nullptr;}
        bool join(std::uint32_t milliseconds)override{return WaitForSingleObject(handle_,milliseconds)==WAIT_OBJECT_0;}
        std::exception_ptr failure()const override{return done_.load(std::memory_order_acquire)?failure_:std::exception_ptr{};}
    private:
        static DWORD WINAPI entry(void* context)noexcept{
            auto& self=*static_cast<Thread*>(context);
            try{self.body_();}catch(...){self.failure_=std::current_exception();}
            self.done_.store(true,std::memory_order_release);return 0;
        }
        std::function<void()> body_;HANDLE handle_=nullptr;std::exception_ptr failure_;std::atomic<bool> done_{false};
    };
public:
    std::unique_ptr<frontend::ControlConnectionThread> create_control_thread(std::function<void()> body,std::uint32_t& id){
        auto owner=std::make_unique<Thread>(std::move(body)); //All source allocation precedes CreateThread.
        if(!owner->launch(id))return {};
        return owner;
    }
    std::uintptr_t create_control_event(){return reinterpret_cast<std::uintptr_t>(CreateEventA(nullptr,TRUE,FALSE,nullptr));}
    void signal_control_event(std::uintptr_t event){(void)SetEvent(reinterpret_cast<HANDLE>(event));}
    void reset_control_event(std::uintptr_t event){(void)ResetEvent(reinterpret_cast<HANDLE>(event));}
    std::uint32_t wait_control_event(std::uintptr_t event,std::uint32_t milliseconds){return WaitForSingleObject(reinterpret_cast<HANDLE>(event),milliseconds);}
    void close_control_event(std::uintptr_t event){(void)CloseHandle(reinterpret_cast<HANDLE>(event));}
    void control_thread_priority(int value){(void)SetThreadPriority(GetCurrentThread(),value);}
    void control_wsa_startup(){WSADATA data{};(void)WSAStartup(0x202,&data);}
    void control_wsa_cleanup(){(void)WSACleanup();}
    std::uintptr_t control_udp_create(){return static_cast<std::uintptr_t>(socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP));}
    void control_udp_bind(std::uintptr_t value,std::uint16_t port){
        sockaddr_in address{};address.sin_family=AF_INET;address.sin_port=htons(port);
        (void)bind(static_cast<SOCKET>(value),reinterpret_cast<const sockaddr*>(&address),sizeof(address));
    }
    void control_udp_close(std::uintptr_t value){(void)closesocket(static_cast<SOCKET>(value));}
    std::optional<frontend::ControlDatagram> control_udp_receive(std::uintptr_t value,unsigned char* buffer,unsigned capacity,unsigned microseconds){
        fd_set input{};FD_SET(static_cast<SOCKET>(value),&input);timeval timeout{};timeout.tv_usec=static_cast<long>(microseconds);
        const auto result=select(static_cast<int>(value+1),&input,nullptr,nullptr,&timeout);
        if(result>=0&&!FD_ISSET(static_cast<SOCKET>(value),&input))return {};
        sockaddr_in sender{};int size=sizeof(sender);
        const auto received=recvfrom(static_cast<SOCKET>(value),reinterpret_cast<char*>(buffer),static_cast<int>(capacity),0,reinterpret_cast<sockaddr*>(&sender),&size);
        if(received<=0)return {};
        frontend::ControlDatagram packet;packet.bytes.assign(buffer,buffer+received);packet.address=sender.sin_addr.s_addr;packet.port=ntohs(sender.sin_port);return packet;
    }
    void control_udp_send(std::uintptr_t value,const std::vector<unsigned char>& bytes,std::uint32_t target,std::uint16_t port){
        sockaddr_in address{};address.sin_family=AF_INET;address.sin_port=htons(port);address.sin_addr.s_addr=target;
        (void)sendto(static_cast<SOCKET>(value),reinterpret_cast<const char*>(bytes.data()),static_cast<int>(bytes.size()),0,reinterpret_cast<const sockaddr*>(&address),sizeof(address));
    }
    void record_control_worker_failure(std::exception_ptr error)noexcept{
        try{if(error)std::rethrow_exception(error);}catch(const std::exception& e){std::fprintf(stderr,"Control worker cleanup failed: %s\n",e.what());}catch(...){std::fputs("Control worker cleanup failed\n",stderr);}
    }
};
} //namespace hd2::host
#endif
