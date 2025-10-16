# Rust Client
A simple Rust wrapper around the C++ API using [cxx.rs](https://cxx.rs/).

See [build.rs](build.rs) for setting include search paths and library search paths and libraries.

## Use
Include the checked out directory into your project workspace, or copy the sources into your
project and use as appropriate.

```rust
use configdb::*;

#[test]
fn operations()
{
  let mut logger = Logger::new("/tmp/", "configdb-rust");
  logger.level = LogLevel::DEBUG;
  init_logger(logger);

  let mut conf = Configuration::new("localhost", 2022);
  conf.ssl = false;
  init(conf);
  
  let mut rd = RequestData::new("key", "value");
  rd.expirationInSeconds = 60;
  let result = set(&rd);
  assert_eq!(result, true);

  let result = get(rd.key.as_str());
  assert!(result.is_ok());
  assert_eq!(result.unwrap(), rd.value);

  let result = ttl(format!("/{}", rd.key).as_str());
  assert!(result > 0);

  let result = remove(rd.key.as_str());
  assert_eq!(result, true);
}
```