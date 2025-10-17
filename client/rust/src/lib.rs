//! Wrapper around config-db C++ API
#![allow(dead_code)]

#[cxx::bridge]
mod configdb
{
  /// C++ API logging level options
  enum LogLevel
  {
    /// Log messages at the `critical` level.
    CRIT,
    /// Log messages at the `warning` level.
    WARN,
    /// Log messages at the `information` level.
    INFO,
    /// Log messages at the `debug` level.
    DEBUG
  }

  /// C++ API global logger configuration.
  struct Logger
  {
    /// The directory to which log files are written.  **Must** exist.  Trailing `/` is mandatory.
    path: String,
    /// The base/root name for the log files.  Logger will append day to the base name each day.
    name: String,
    /// The level to set for the logger.  Default is `LogLevel::INFO`.
    level: LogLevel,
    /// Set to `true` to also echo logs to `stdout`.  Default `false`.
    console: bool
  }

  /// Use to configure the C++ API and logging
  struct Configuration
  {
    /// The host on which the service is running.
    host: String,
    /// The port on which the service is listening.
    port: u16,
    /// Set to `true` if the service is running with TLS on.
    ssl: bool
  }

  /// Structure for saving value(s) into config-db
  struct RequestData
  {
    /// The key for the value being stored.  Use `/` as separator.
    key: String,
    /// The value being stored.  Any string data, base64 encoded binary data, ...
    value: String,
    /// If a TTL is to be associated with this data.
    expirationInSeconds: u32,
    /// Only create if `key` does not already exist.  No updates are made.
    ifNotExists: bool,
    /// Indicate the value is a cache value.  Requires non-zero `expirationInSeconds` value.
    cache: bool
  }

  /// Structure used when retrieving a batch of keys.  Also used to move a batch of keys.
  struct KeyValue
  {
    /// The key for the stored data.
    key: String,
    /// The value associated with the `key`.  For `move`, this is the destination `key`.
    value: String
  }

  /// Structure for retrieving a batch of TTL values.
  struct TTL
  {
    /// The key for which the TTL was set.
    key: String,
    /// The remaining time till the key is automatically removed.
    expirationInSeconds: u32
  }

  unsafe extern "C++"
  {
    include!("configdb/include/configdb.hpp");

    /// Initialise the C++ global logger.
    ///
    /// The logger is global for all SPT APIs, and should be initialised exactly once
    /// before initialising any clients APIs.  Ensure this if using the Rust wrapper alongside
    /// other wrappers (e.g. [encrypter](https://github.com/sptrakesh/encrypter/tree/master/client/rust))
    ///
    /// # Arguments
    ///
    /// * `config` - The logging configuration structure
    pub fn init_logger(config: Logger);

    /// Initialise the C++ client API.  **Should** only be invoked **once**.
    ///
    /// # Arguments
    ///
    /// * `config` - The client configuration structure
    pub fn init(config: Configuration);

    /// Retrieve the stored value from the service.
    ///
    /// # Arguments
    ///
    /// * `key` - The key to retrieve from the database service.
    ///
    /// # Returns
    /// - The value if found, or an error.
    pub fn get(key: &str) -> Result<String>;

    /// Retrieve a batch of values.  Results will omit any keys which do not exist.
    ///
    /// # Arguments
    ///
    /// * `keys` - The batch of keys to retrieve from the database service.
    ///
    /// # Returns
    /// - The `key->value` pairs that were found.
    pub fn get_multiple(keys: &Vec<String>) -> Vec<KeyValue>;

    /// Store information to the database.
    ///
    /// # Arguments
    ///
    /// * `data` - The `key->value` and additional specifications for storing the data.
    pub fn set(data: &RequestData) -> bool;

    /// Store a batch of information to the database.
    ///
    /// # Arguments
    ///
    /// * `data` - The batch of `key->value` pairs with additional specifications as appropriate.
    pub fn set_multiple(data: &Vec<RequestData>) -> bool;

    /// Rename a `key` to the specified `dest`.  If `key` does not exists, or other errors were
    /// encountered, returns `false`.
    ///
    /// # Arguments
    ///
    /// * `key` - The key to rename.
    /// * `dest` - The new key for the `value`.
    ///
    /// # Returns
    /// - `true` if the `key` existed and was renamed.
    pub fn rename(key: &str, dest: &str) -> bool;

    /// Rename a batch of keys.
    ///
    /// # Arguments
    ///
    /// * `keys` - The batch of `key` and `dest` values.
    ///
    /// # Returns
    /// - `true` if all the `key`s were renamed successfully.
    pub fn rename_multiple(keys: &Vec<KeyValue>) -> bool;

    /// Remove the specified key from the database.
    ///
    /// # Arguments
    ///
    /// * `key` - The key to remove.
    ///
    /// # Returns
    /// - `true` if the `key` was removed.
    pub fn remove(key: &str) -> bool;

    /// Remove a batch of keys from the database.
    ///
    /// # Arguments
    ///
    /// * `keys` - The batch of keys to remove.
    ///
    /// # Returns
    /// - `true` if all the `key`s were removed.
    pub fn remove_multiple(keys: &Vec<String>) -> bool;

    /// List child nodes (single level only) for the specified *root* path.
    ///
    ///  **Note:** paths are split on the `/` character.  See [documentation](https://sptrakesh.github.io/config-db.html#keys).
    ///
    /// # Arguments
    ///
    /// * `path` - The base path whose immediate children are to be returned.
    ///
    /// # Returns
    /// - The names of the immediate child nodes.
    pub fn list(path: &str) -> Result<Vec<String>>;

    /// Retrieve the time till expiration in seconds for the specified key.
    ///
    /// **Note:** The returned value is not the original TTL value that was set,
    /// but the number of seconds till expiration at this instant.
    ///
    /// # Arguments
    ///
    /// * `key` - The key whose TTL value is to be retrieved.
    ///
    /// # Returns
    /// - The seconds till expiration from *now*, or `0` if no TTL was set
    pub fn ttl(key: &str) -> u32;

    /// Retrieve the TTL values for a batch of keys.
    ///
    /// # Arguments
    ///
    /// * `keys` - The batch of `key`s whose TTL values are to be retrieved.
    ///
    /// # Returns
    /// - The `key->ttl` pairs that was requested.
    pub fn ttl_multiple(keys: &Vec<String>) -> Vec<TTL>;
  }
}

impl configdb::Logger
{
  /// Create a new logging configuration with default values for `level` and `console`.
  ///
  /// # Arguments
  ///
  /// * `path` - The path under which the log files are to be written.
  /// * `name` - The base name for the log files.
  pub fn new(dir: &str, name: &str) -> Self
  {
    configdb::Logger{path: dir.to_string(), name: name.to_string(), level: configdb::LogLevel::INFO, console: false}
  }
}

impl configdb::Configuration
{
  /// Create a new instance of the API configuration using the specified values.
  /// `ssl` is set to `true` by default.  Ensure that the service is also running with
  /// TLS option.
  ///
  /// # Arguments
  ///
  /// * `host` - The host on which the encrypter service is running.
  /// * `port` - The prot on which the encrypter service is listening.
  pub fn new(host: &str, port: u16) -> Self
  {
    configdb::Configuration{host: host.to_string(), port, ssl: true}  }
}

impl configdb::RequestData
{
  pub fn new(key: &str, value: &str) -> Self
  {
    configdb::RequestData{key: key.to_string(), value: value.to_string(),
      expirationInSeconds: 0, ifNotExists: false, cache: false}
  }
}

#[cfg(test)]
mod tests
{
  use crate::configdb::*;

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

    let keys = vec![rd.key.clone()];
    let result = get_multiple(&keys);
    assert_eq!(result.len(), 1);
    assert_eq!(result.get(0).unwrap().value, rd.value);

    let result = ttl(format!("/{}", rd.key).as_str());
    assert_ne!(result, 0);
    let result = ttl(rd.key.as_str());
    assert_ne!(result, 0);

    let result = ttl_multiple(&keys);
    assert_eq!(result.len(), 1);
    assert_ne!(result.get(0).unwrap().expirationInSeconds, 0);

    let result = list("/");
    assert!(result.is_ok());
    assert!(result.unwrap().len() > 0);

    let nkey = "moved";
    let result = rename(rd.key.as_str(), nkey);
    assert_eq!(result, true);

    let result = get(nkey);
    assert!(result.is_ok());
    assert_eq!(result.unwrap(), rd.value);

    let result = get(rd.key.as_str());
    assert!(result.is_err());

    let result = remove(nkey);
    assert_eq!(result, true);

    let mut data : Vec<RequestData> = Vec::new();
    let mut keys : Vec<String> = Vec::new();
    for i in 0..5
    {
      let key = format!("key{}", i);
      data.push(RequestData::new(key.as_str(), format!("value{}", i).as_str()));
      keys.push(key);
    }

    let result = set_multiple(&data);
    assert_eq!(result, true);

    for key in &keys
    {
      let result = get(key.as_str());
      assert!(result.is_ok());
    }

    let result = get_multiple(&keys);
    assert_eq!(result.len(), keys.len());

    let result = list("/");
    assert!(result.is_ok());
    assert_eq!(result.unwrap().len(), keys.len());

    let result = remove_multiple(&keys);
    assert_eq!(result, true);
  }
}