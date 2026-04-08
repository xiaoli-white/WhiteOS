#![no_std]

pub mod console;
pub mod logger;

use core::arch::asm;
use core::panic::PanicInfo;

pub fn hcf() -> ! {
    loop {
        unsafe {
            #[cfg(target_arch = "x86_64")]
            asm!("hlt");
        }
    }
}

#[panic_handler]
pub fn panic(_info: &PanicInfo) -> ! {
    hcf()
}
