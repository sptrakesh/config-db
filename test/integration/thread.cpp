//
// Created by Rakesh on 25/06/2024.
//

#include <format>
#include <mutex>
#include <ranges>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "../../src/api/api.hpp"

using spt::configdb::model::RequestData;
using std::operator ""s;
using std::operator ""sv;

namespace
{
  namespace itest::pthread
  {
    struct Result
    {
      bool create{ false };
      bool get{ false };
      bool list{ false };
      bool update{ false };
      bool move{ false };
      bool getDest{ false };
      bool listAfterMove{ false };
      bool ttl{ false };
      bool remove{ false };
    };

    struct Crud
    {
      Result run()
      {
        auto res = Result{};
        create( res );
        get( res );
        list( res );
        update( res );
        move( res );
        ttl( res );
        remove( res );
        return res;
      }

    private:
      void create( Result& result )
      {
        const auto data = RequestData{ key, "value"sv };
        result.create = spt::configdb::api::set( data );
      }

      void get( Result& result )
      {
        const auto value = spt::configdb::api::get( key );
        result.get = value && *value == "value";
      }

      void list( Result& result )
      {
        const auto children = spt::configdb::api::list( "/"sv );
        result.list = children && !children->empty();
        if ( !result.list ) return;

        result.list = std::ranges::find( *children, key ) != std::end( *children );
      }

      void update( Result& result )
      {
        result.update = spt::configdb::api::set( RequestData{ key, "value modified"sv } );
        if ( result.update )
        {
          const auto value = spt::configdb::api::get( key );
          result.update = value && *value == "value modified";
        }
      }

      void move( Result& result )
      {
        result.move = spt::configdb::api::move(
          RequestData{ key, dest, RequestData::Options{ 60, true, false } } );
        const auto value = spt::configdb::api::get( dest );
        result.getDest = value && *value == "value modified";
        const auto children = spt::configdb::api::list( "/"sv );
        result.listAfterMove = children && !children->empty();
        if ( !result.listAfterMove ) return;

        result.listAfterMove = std::ranges::find( *children, dest ) != std::end( *children );
      }

      void ttl( Result& result )
      {
        result.ttl = spt::configdb::api::ttl( dest ).count() > 0;
      }

      void remove( Result& result )
      {
        result.remove = spt::configdb::api::remove( dest );
      }

      std::string key{ std::format( "key1{}", std::this_thread::get_id() ) };
      std::string dest{ std::format( "key2{}", std::this_thread::get_id() ) };
    };
  };
}

SCENARIO( "Multi-threaded test", "thread" )
{
  GIVEN( "Threads performing crud operations" )
  {
    WHEN( "Spawning threaded requests to database" )
    {
      const std::size_t nthreads = 8;
      std::mutex mutex;
      std::vector<std::thread> threads;
      threads.reserve( nthreads );
      std::vector<itest::pthread::Result> results;
      results.reserve( nthreads );

      for ( auto i = 0ul; i < nthreads; ++i )
      {
        threads.emplace_back( [&results, &mutex]()
        {
          auto runner = itest::pthread::Crud{};
          auto result = runner.run();
          auto lg = std::lock_guard{ mutex };
          results.push_back( result );
        } );
      }

      for ( auto& thread : threads ) if ( thread.joinable() ) thread.join();

      for ( auto i = 0ul; i < nthreads; ++i )
      {
        INFO( std::format( "Checking thread {}", i ) );
        const auto& result = results[i];
        CHECK( result.create );
        CHECK( result.get );
        CHECK( result.list );
        CHECK( result.update );
        CHECK( result.move );
        CHECK( result.getDest );
        CHECK( result.listAfterMove );
        //CHECK( result.ttl );
        CHECK( result.remove );
      }
    }
  }
}
