//
// Created by Rakesh on 29/12/2021.
//

#include "client.h"
#include "../../api/api.h"
#include "../log/NanoLog.h"

#include <cctype>
#include <cstring>
#include <iostream>
#include <string_view>

#include <readline/readline.h>
#include <readline/history.h>

//https://stackoverflow.com/questions/2924697/how-does-one-output-bold-text-in-bash

namespace spt::configdb::client::pclient
{
  void help()
  {
    std::cout << "\033[1mAvailable commands\033[0m" << '\n';
    std::cout << "  \033[1mls\033[0m \033[3m<path>\033[0m - To list child node names.  Eg. [ls /]" << '\n';
    std::cout << "  \033[1mget\033[0m \033[3m<key>\033[0m - To get configured value for key.  Eg. [get /key1/key2/key3]" << '\n';
    std::cout << "  \033[1mset\033[0m \033[3m<key> <value>\033[0m - To set value for key.  Eg. [set /key1/key2/key3 Some long value. Note no surrounding quotes]" << '\n';
    std::cout << "  \033[1mmv\033[0m \033[3m<key> <destination>\033[0m - To move value for key to destination.  Eg. [mv /key1/key2/key3 /key/key2/key3]" << '\n';
    std::cout << "  \033[1mrm\033[0m \033[3m<key>\033[0m - To remove configured key.  Eg. [rm /key1/key2/key3]" << '\n';
    std::cout << "  \033[1mimport\033[0m \033[3m<path to file>\033[0m - To bulk import \033[3mkey-values\033[0m from file.  Eg. [import /tmp/kvps.txt]" << '\n';
  }

  std::string_view trim( std::string_view in )
  {
    auto left = in.begin();
    for ( ;; ++left )
    {
      if ( left == in.end() ) return in;
      if ( !std::isspace( *left ) ) break;
    }
    auto right = in.end() - 1;
    for ( ; right > left && std::isspace( *right ); --right );
    return { left, static_cast<std::size_t>(std::distance( left, right ) + 1) };
  }

  std::tuple<std::string_view, std::size_t> command( std::string_view line )
  {
    auto idx = line.find( ' ', 0 );
    if ( idx == std::string_view::npos ) return { line, idx };
    return { line.substr( 0, idx ), idx };
  }

  std::tuple<std::string_view, std::size_t> key( std::string_view line, std::size_t begin )
  {
    auto idx = line.find( ' ', begin + 1 );
    while ( idx != std::string::npos && line.substr( begin + 1, idx - begin - 1 ).empty() )
    {
      ++begin;
      idx = line.find( ' ', begin + 1 );
    }

    return { line.substr( begin + 1, idx - begin - 1 ), idx };
  }

  void processList( std::string_view line, std::size_t idx )
  {
    auto [key, end] = pclient::key( line, idx );
    if ( end <= idx )
    {
      LOG_WARN << "Cannot parse key from " << line;
      std::cout << "Cannot parse key from " << line << '\n';
      return;
    }

    auto response = api::list( key );
    if ( !response || response->empty() )
    {
      LOG_WARN << "Error retrieving path " << key;
      std::cout << "Error retrieving path " << key << '\n';
      return;
    }

    LOG_INFO << "Retrieved children for path " << key;
    for ( auto&& v : *response ) std::cout << v << '\n';
  }

  void processGet( std::string_view line, std::size_t idx )
  {
    auto [key, end] = pclient::key( line, idx );
    if ( end <= idx )
    {
      std::cout << "Cannot parse key from " << line << '\n';
      return;
    }

    auto response = api::get( key );
    if ( !response )
    {
      LOG_WARN << "Error retrieving value for key " << key;
      std::cout << "Error retrieving value for key " << key << '\n';
      return;
    }

    LOG_INFO << "Retrieved value for key " << key;
    std::cout << *response << '\n';
  }

  void processSet( std::string_view line, std::size_t idx )
  {
    auto [key, end] = pclient::key( line, idx );
    if ( end == std::string_view::npos )
    {
      std::cout << "Cannot parse key from " << line << '\n';
      return;
    }

    auto vidx = line.find( ' ', end + 1 );
    while ( vidx != std::string::npos && line.substr( end + 1, vidx - end - 1 ).empty() )
    {
      ++end;
      vidx = line.find( ' ', end + 1 );
    }

    if ( const auto response = api::set( key, line.substr( end + 1 ) ); !response )
    {
      std::cout << "Error setting key " << key << '\n';
      return;
    }

    std::cout << "Set key " << key << '\n';
  }

  void processMove( std::string_view line, std::size_t idx )
  {
    auto [key, end] = pclient::key( line, idx );
    if ( end == std::string_view::npos )
    {
      std::cout << "Cannot parse key from " << line << '\n';
      return;
    }

    auto vidx = line.find( ' ', end + 1 );
    while ( vidx != std::string::npos && line.substr( end + 1, vidx - end - 1 ).empty() )
    {
      ++end;
      vidx = line.find( ' ', end + 1 );
    }

    auto dest = line.substr( end + 1 );
    if ( const auto response = api::move( key, dest ); !response )
    {
      std::cout << "Error moving key " << key << " to " << dest << '\n';
      return;
    }

    std::cout << "Moved key " << key << " to " << dest << '\n';
  }

  void processRemove( std::string_view line, std::size_t idx )
  {
    auto [key, end] = pclient::key( line, idx );
    if ( end <= idx )
    {
      std::cout << "Cannot parse key from " << line << '\n';
      return;
    }

    if ( const auto response = api::remove( key ); !response )
    {
      LOG_WARN << "Error removing key " << key;
      std::cout << "Error removing key " << key << '\n';
      return;
    }

    std::cout << "Removed key " << key << '\n';
  }

  void processImport( std::string_view line, std::size_t idx )
  {
    auto [key, end] = pclient::key( line, idx );
    if ( end <= idx )
    {
      std::cout << "Cannot parse file to import from " << line << '\n';
      return;
    }

    const auto& [response, size, count] = api::import( std::string{ key } );
    if ( !response )
    {
      LOG_WARN << "Error importing (" << int(size) << '/' << count << ") keys from file " << key;
      std::cout << "Error importing (" << size << '/' << count << ") keys from file " << key << '\n';
      return;
    }

    LOG_INFO << "Imported (" << int(size) << '/' << count << ") keys from file " << key;
    std::cout << "Imported (" << size << '/' << count << ") keys from file " << key << '\n';
  }
}

int spt::configdb::client::run( std::string_view server, std::string_view port, bool ssl )
{
  api::init( server, port, ssl );

  using namespace std::literals;
  std::cout << "Enter commands followed by <ENTER>" << '\n';
  std::cout << "Enter \033[1mhelp\033[0m for help about commands" << '\n';
  std::cout << "Enter \033[1mexit\033[0m or \033[1mquit\033[0m to exit shell\n";

  // Disable tab completion
  rl_bind_key( '\t', rl_insert );

  std::string previous;
  previous.reserve( 128 );

  char* buf;
  while ( ( buf = readline("configdb> " ) ) != nullptr )
  {
#ifdef __STDC_LIB_EXT1__
    auto len = strnlen_s( buf, 64*1024 );
#else
    auto len = strlen( buf ); // flawfinder: ignore
#endif
    if ( len == 0 )
    {
      std::free( buf );
      continue;
    }

    if ( previous != std::string{ buf } ) add_history( buf );

    auto line = std::string_view{ buf, len };
    line = pclient::trim( line );

    if ( line == "exit"sv || line == "quit"sv )
    {
      std::cout << "Bye\n";
      break;
    }
    else if ( line == "help"sv ) pclient::help();
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
      else if ( "mv"sv == command )
      {
        pclient::processMove( line, idx );
      }
      else if ( "rm"sv == command )
      {
        pclient::processRemove( line, idx );
      }
      else if ( "import"sv == command )
      {
        pclient::processImport( line, idx );
      }
      else
      {
        std::cout << "Unknown command " << command << '\n';
      }
    }

    previous.clear();
    previous.append( buf, len );
    std::free( buf );
  }

  return 0;
}
