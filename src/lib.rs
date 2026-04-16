#![no_std]
#![feature(abi_x86_interrupt)]

extern crate alloc;

pub mod console;
pub mod gdt;
pub mod interrupt;
pub mod logger;
pub mod memory;
pub mod task;

use core::fmt::Write;
use core::panic::PanicInfo;

use x86_64::instructions;

use crate::console::with_console;
use crate::gdt::init_gdt;
use crate::interrupt::{init_idt, pics::init_pcis};

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
pub fn init_kernel() {
    init_gdt();
    init_idt();
    init_pcis();
    instructions::interrupts::enable();
}

pub fn hcf() -> ! {
    loop {
        #[cfg(target_arch = "x86_64")]
        x86_64::instructions::hlt();
    }
}

#[panic_handler]
pub fn panic(info: &PanicInfo) -> ! {
    with_console(|console| writeln!(console, "{}", info).unwrap());
    hcf()
}
