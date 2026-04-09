#![no_std]
#![no_main]

extern crate alloc;

use limine::BaseRevision;

use WhiteOS::{
    console::{init_console, with_console},
    gdt::init_gdt,
    hcf,
    interrupt::init_idt,
    logger::init_logger,
    memory::{self, HHDM_REQUEST, active_level_4_table, frame::PhysicalFrameAllocator, heap},
};
use alloc::boxed::Box;
use core::fmt::Write;
use x86_64::{VirtAddr, structures::paging::PageTable};

#[used]
static BASE_REVISION: BaseRevision = BaseRevision::new();

#[unsafe(no_mangle)]
pub extern "C" fn kernel_main() -> ! {
    assert!(BASE_REVISION.is_supported());

    init_console();
    init_logger();
    init_gdt();
    init_idt();

    with_console(|console| writeln!(console, "Hello, kernel!").unwrap());
    let l4_table = active_level_4_table();

    for (i, entry) in l4_table.iter().enumerate() {
        if !entry.is_unused() {
            with_console(|console| writeln!(console, "L4 Entry {}: {:?}", i, entry).unwrap());
            let phys = entry.frame().unwrap().start_address();
            let virt = phys.as_u64() + HHDM_REQUEST.response().unwrap().offset;
            let ptr = VirtAddr::new(virt).as_mut_ptr();
            let l3_table: &PageTable = unsafe { &*ptr };

            for (i, entry) in l3_table.iter().enumerate() {
                if !entry.is_unused() {
                    with_console(|console| {
                        writeln!(console, "  L3 Entry {}: {:?}", i, entry).unwrap()
                    });
                }
            }
        }
    }
    let mut mapper = memory::init();
    let mut allocator = PhysicalFrameAllocator::default();
    heap::init_heap(&mut mapper, &mut allocator).expect("heap initialization failed");

    let x = Box::new(41);
    with_console(|console| writeln!(console, "{}", x).unwrap());
    /*
    if let Some(framebuffer) = FRAMEBUFFER_REQUEST
        .response()
        .and_then(|resp| resp.framebuffers().first())
    {
        let width = framebuffer.width;
        let height = framebuffer.height;
        let pitch = framebuffer.pitch;

        let fb_ptr = framebuffer.address() as *mut u32;

        let pitch_pixels = (pitch / 4) as usize;

        for y in 0..height {
            for x in 0..width {
                let n_x = (x as u32) * 255 / (width as u32);
                let n_y = (y as u32) * 255 / (height as u32);

                let color = (n_y << 8) | n_x;

                let offset = (y as usize) * pitch_pixels + (x as usize);

                unsafe {
                    fb_ptr.add(offset).write_volatile(color);
                }
            }
        }
    }
    */

    hcf();
}
