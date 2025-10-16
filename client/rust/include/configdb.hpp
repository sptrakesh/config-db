//
// Created by Rakesh on 15/10/2025.
//

#pragma once

#include <cstdint>

#include "rust/cxx.h"
#include "configdb/src/lib.rs.h"

void init_logger( Logger conf );
void init( Configuration conf );
rust::String get( rust::Str key );
rust::Vec<KeyValue> get_multiple( const rust::Vec<rust::String>& keys );
bool set( const RequestData& data );
bool set_multiple( const rust::Vec<RequestData>& keys );
bool rename( rust::Str key, rust::Str dest );
bool rename_multiple( const rust::Vec<KeyValue>& keys );
bool remove( rust::Str key );
bool remove_multiple( const rust::Vec<rust::String>& keys );
rust::Vec<rust::String> list( rust::Str key );
uint32_t ttl( rust::Str key );
rust::Vec<TTL> ttl_multiple( const rust::Vec<rust::String>& keys );
