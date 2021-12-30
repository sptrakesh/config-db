//
// Created by Rakesh on 29/12/2021.
//

#include "client.h"
#include "connection.h"
#include "../lib/pool/pool.h"

#include <iostream>
#include <string>
#include <string_view>

#include <boost/algorithm/string/trim.hpp>

//https://stackoverflow.com/questions/2924697/how-does-one-output-bold-text-in-bash
//Down Arrow  0x1B 0x5B 0x42
//Left Arrow  0x1B 0x5B 0x44
//Right Arrow 0x1B 0x5B 0x43
//Up Arrow    0x1B 0x5B 0x41

namespace spt::configdb::client::pclient
{
  void help()
  {
    std::cout << "\033[1mAvailable commands\033[0m" << '\n';
    std::cout << "  \033[1mls\033[0m \033[3m<path>\033[0m - To list child node names.  Eg. [ls /]" << '\n';
    std::cout << "  \033[1mget\033[0m \033[3m<key>\033[0m - To get configured value for key.  Eg. [get /key1/key2/key3]" << '\n';
    std::cout << "  \033[1mset\033[0m \033[3m<key> <value>\033[0m - To set value for key.  Eg. [set /key1/key2/key3 Some long value. Note no surrounding quotes]" << '\n';
    std::cout << "  \033[1mrm\033[0m \033[3m<key>\033[0m - To remove configured key.  Eg. [rm /key1/key2/key3]" << '\n';
  }

  void prompt()
  {
    std::cout << "configdb> ";
  }

  struct PoolHolder
  {
    static PoolHolder& instance()
    {
      static PoolHolder h;
      return h;
    }

    ~PoolHolder() = default;
    PoolHolder(const PoolHolder&) = delete;
    PoolHolder& operator=(const PoolHolder&) = delete;

    spt::configdb::pool::Pool<Connection> pool{ spt::configdb::client::create, pool::Configuration{} };

  private:
    PoolHolder() = default;
  };

  std::tuple<std::string_view, std::size_t> command( std::string_view line )
  {
    auto idx = line.find( ' ', 0 );
    if ( idx == std::string_view::npos ) return { line, idx };
    return { line.substr( 0, idx ), idx };
  }

  std::tuple<std::string_view, std::size_t> key( std::string_view line, std::size_t begin )
  {
    auto idx = line.find( ' ', begin + 1 );
    if ( idx == std::string_view::npos ) return { line, idx };
    return { line.substr( begin + 1, idx - begin - 1 ), idx };
  }

  void processList( std::string_view line, std::size_t idx )
  {
    auto key = line.substr( idx + 1 );

    auto popt = pclient::PoolHolder::instance().pool.acquire();
    if ( !popt )
    {
      std::cout << "Error acquiring connection from pool\n";
      return;
    }

    auto response = (*popt)->list( key );
    if ( response->value_type() != model::ResponseValue::Children )
    {
      std::cout << "Error retrieving path " << key << '\n';
      return;
    }

    auto resp = response->value_as<model::Children>();
    for ( auto&& v : *resp->value() )
    {
      std::cout << v->string_view() << '\n';
    }
  }

  void processGet( std::string_view line, std::size_t idx )
  {
    auto key = line.substr( idx + 1 );

    auto popt = pclient::PoolHolder::instance().pool.acquire();
    if ( !popt )
    {
      std::cout << "Error acquiring connection from pool\n";
      return;
    }

    auto response = (*popt)->get( key );
    if ( !response )
    {
      std::cout << "Error retrieving value for key " << key << '\n';
      return;
    }
    if ( response->value_type() != model::ResponseValue::Value )
    {
      std::cout << "Error retrieving key " << key << '\n';
      return;
    }

    auto resp = response->value_as<model::Value>();
    std::cout << resp->value()->string_view() << '\n';
  }

  void processSet( std::string_view line, std::size_t idx )
  {
    auto [key, end] = pclient::key( line, idx );
    if ( end == std::string_view::npos )
    {
      std::cout << "Cannot parse key " << line.substr( idx + 1, line.size() - 1 ) << '\n';
      return;
    }

    auto popt = pclient::PoolHolder::instance().pool.acquire();
    if ( !popt )
    {
      std::cout << "Error acquiring connection from pool\n";
      return;
    }

    auto response = (*popt)->set( key, line.substr( end + 1 ) );
    if ( !response )
    {
      std::cout << "Unable to set key " << key << '\n';
      return;
    }

    if ( response->value_type() != model::ResponseValue::Success ||
        !response->value_as<model::Success>()->value() )
    {
      std::cout << "Error setting key " << key << '\n';
    }
    else
    {
      std::cout << "Set key " << key << '\n';
    }
  }

  void processRemove( std::string_view line, std::size_t idx )
  {
    auto key = line.substr( idx + 1 );

    auto popt = pclient::PoolHolder::instance().pool.acquire();
    if ( !popt )
    {
      std::cout << "Error acquiring connection from pool\n";
      return;
    }

    auto response = ( *popt )->remove( key );
    if ( response->value_type() != model::ResponseValue::Success )
    {
      std::cout << "Error removing key " << key << '\n';
      return;
    }

    auto resp = response->value_as<model::Success>();
    if ( !resp->value())
    {
      std::cout << "Error removing key " << key << '\n';
    }
    else
    {
      std::cout << "Removed key " << key << '\n';
    }
  }
}

int spt::configdb::client::run( std::string_view server, std::string_view port )
{
  init( server, port );

  using namespace std::literals;
  std::cout << "Enter commands followed by <ENTER>" << '\n';
  std::cout << "Enter \033[1mhelp\033[0m for help about commands" << '\n';
  std::cout << "Enter \033[1mexit\033[0m or \033[1mquit\033[0m to exit shell\n";
  pclient::prompt();

  std::string line;
  while ( true )
  {
    std::getline( std::cin, line );
    boost::algorithm::trim( line );

    if ( line == "exit"s || line == "quit"s )
    {
      std::cout << "Bye\n";
      break;
    }
    else if ( line == "help"s ) pclient::help();
    else if ( line.empty() ) { /* noop */ }
    else
    {
      auto [command, idx] = pclient::command( line );

      if ( "ls"sv == command )
      {
        pclient::processList( line, idx );
      }
      else if ( "get"sv == command )
      {
        pclient::processGet( line, idx );
      }
      else if ( "set"sv == command )
      {
        pclient::processSet( line, idx );
      }
      else if ( "rm"sv == command )
      {
        pclient::processRemove( line, idx );
      }
      else
      {
        std::cout << "Unknown command " << command << '\n';
      }
    }

    pclient::prompt();
  }

  return 0;
}

