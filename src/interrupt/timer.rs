use core::fmt::Write;
use x86_64::structures::idt::InterruptStackFrame;

use super::{InterruptIndex, pics::PICS};
use crate::console::with_console;

pub extern "x86-interrupt" fn timer_interrupt_handler(_stack_frame: InterruptStackFrame) {
    with_console(|console| write!(console, ".").unwrap());

    unsafe {
        PICS.lock()
            .notify_end_of_interrupt(InterruptIndex::Timer.as_u8());
    }
}