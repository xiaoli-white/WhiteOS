#![no_std]
#![no_main]

use limine::BaseRevision;

use WhiteOS::{
    console::{init_console, with_console},
    hcf,
    logger::init_logger,
};
use core::fmt::Write;

#[used]
static BASE_REVISION: BaseRevision = BaseRevision::new();

#[unsafe(no_mangle)]
pub extern "C" fn kernel_main() -> ! {
    assert!(BASE_REVISION.is_supported());

    init_console();
    init_logger();

    with_console(|console| {
        writeln!(console, "Hello, kernel!");
    });
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
