#pragma once

#include <boost/noncopyable.hpp>
#include <muduo/net/TcpServer.h>
#include <unordered_map>

#include "dns_resolver.h"
#include "tunnel.h"

namespace zy
{
class proxy_server : boost::noncopyable
{
 public:
  // 数据转发要求： 获取到客户端的完整请求再转发， 对于服务器返回的数据，完整返回给客户端
  enum conState
  {
    kStart = 0, // 连接刚开始建立
    kGotRequest, // 获取完所有的http请求
    kResolved,  // 获取到远程服务器的ip地址
    kTransport_http, // 和远程服务器建立http连接, 正在执行转发过程(转发需要修改header)
    kTransport_https, // 和远程服务器建立https连接，正在执行转发过程(这是一个简单的隧道转发)
  };

  // 代理类型
  enum Type
  {
    kHttp = 0,
    kHttps
  };

  proxy_server(muduo::net::EventLoop* loop, const muduo::net::InetAddress& addr);

  void onConnection(const muduo::net::TcpConnectionPtr& con);

  void onMessage(const muduo::net::TcpConnectionPtr& con, muduo::net::Buffer* buf, muduo::Timestamp);

  void onResolve(const muduo::net::InetAddress &addr,
            const boost::weak_ptr<muduo::net::TcpConnection> wkCon,
            uint16_t port, const std::string& request);

  void onResolve(const muduo::net::InetAddress &addr,
                 const boost::weak_ptr<muduo::net::TcpConnection> wkCon,
                 uint16_t port);

  void start() { server_.start(); }


  void set_con_state(const muduo::string& con_name, conState state);

 private:

  // is valid address ?
  static bool is_valid_addr(const muduo::net::InetAddress& addr);

  void onResolveError(const muduo::net::TcpConnectionPtr& con);

  void onHeaderError(const muduo::net::TcpConnectionPtr& con);

  void clean_from_container(const muduo::string& con_name);

  muduo::net::EventLoop* loop_;
  muduo::net::TcpServer server_;
  dns_resolver resolver_;
  std::unordered_map<muduo::string, conState> con_states_;
  std::unordered_map<muduo::string, TunnelPtr> tunnels_;
};
}