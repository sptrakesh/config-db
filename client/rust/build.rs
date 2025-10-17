fn main() {
  if cfg!(target_os = "macos")
  {
    cxx_build::bridge("src/lib.rs")
        .cpp(true)
        .include("/usr/local/spt/include")
        .include("/usr/local/boost/include")
        .file("src/configdb.cpp")
        .std("c++23")
        .compile("configdb");

    println!("cargo::rustc-link-search=/usr/local/spt/lib");
    println!("cargo::rustc-link-search=/opt/homebrew/opt/openssl@3/lib");
  }
  else
  {
    cxx_build::bridge("src/lib.rs")
        .cpp(true)
        .include("/opt/spt/include")
        .include("/opt/local/include")
        .file("src/configdb.cpp")
        .std("c++23")
        .compile("configdb");
    println!("cargo::rustc-link-search=/opt/spt/lib");
  }

  if cfg!(target_os = "linux") { println!("cargo::rustc-link-lib=gcc_eh"); }

  println!("cargo::rustc-link-lib=nanolog");
  println!("cargo::rustc-link-lib=configcommon");
  println!("cargo::rustc-link-lib=configapi");
  println!("cargo::rustc-link-lib=crypto");
  println!("cargo::rustc-link-lib=ssl");
  println!("cargo:rerun-if-changed=src/configdb.cpp");
  println!("cargo:rerun-if-changed=include/configdb.hpp");
}