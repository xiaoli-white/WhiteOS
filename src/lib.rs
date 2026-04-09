#![no_std]

extern crate alloc;

pub mod console;
pub mod logger;
pub mod memory;

use core::arch::asm;
use core::fmt::Write;
use core::panic::PanicInfo;

use crate::console::with_console;

pub struct Locked<A> {
    inner: spin::Mutex<A>,
}

impl<A> Locked<A> {
    pub const fn new(inner: A) -> Self {
        Locked {
            inner: spin::Mutex::new(inner),
        }
    }

    pub fn lock(&self) -> spin::MutexGuard<A> {
        self.inner.lock()
    }
}

pub fn hcf() -> ! {
    loop {
        unsafe {
            #[cfg(target_arch = "x86_64")]
            asm!("hlt");
        }
    }
}

#[panic_handler]
pub fn panic(info: &PanicInfo) -> ! {
    with_console(|console| writeln!(console, "{}", info).unwrap());
    hcf()
}
