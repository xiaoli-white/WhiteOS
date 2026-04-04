fn main() {
    let arch =
        std::env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    let linker_script = format!("linker-{arch}.ld");
    println!("cargo:rustc-link-arg=-T{}", linker_script);
    println!("cargo:rerun-if-changed={}", linker_script);
}
