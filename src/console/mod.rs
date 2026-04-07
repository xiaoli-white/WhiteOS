use core::fmt;
use limine::{framebuffer::Framebuffer, request::FramebufferRequest};
use spin::{Mutex, Once};

const FONT_FILE: &[u8] = include_bytes!("../../fonts/Lat7-Terminus16.psf");

pub const FONT_WIDTH: usize = 8;
pub const FONT_HEIGHT: usize = FONT_FILE[3] as usize;

const FONT_DATA_OFFSET: usize = 4;

#[used]
#[unsafe(link_section = ".limine_requests")]
pub static FRAMEBUFFER_REQUEST: FramebufferRequest = FramebufferRequest::new();

#[derive(Debug, Clone, Copy)]
pub enum Color {
    Black,
    Red,
    Green,
    Blue,
    White,
    Yellow,
    Cyan,
    Magenta,
    Rgb(u8, u8, u8),
}

impl Color {
    pub fn to_u32(self) -> u32 {
        match self {
            Color::Black => 0x000000,
            Color::Red => 0xFF0000,
            Color::Green => 0x00FF00,
            Color::Blue => 0x0000FF,
            Color::White => 0xFFFFFF,
            Color::Yellow => 0xFFFF00,
            Color::Cyan => 0x00FFFF,
            Color::Magenta => 0xFF00FF,
            Color::Rgb(r, g, b) => ((r as u32) << 16) | ((g as u32) << 8) | (b as u32),
        }
    }
}

pub struct Console {
    x: usize,
    y: usize,
    color: u32,
    bg_color: u32,
    width: usize,
    height: usize,
    pitch: usize,
}

pub fn get_framebuffer() -> &'static Framebuffer {
    FRAMEBUFFER_REQUEST
        .response()
        .and_then(|resp| resp.framebuffers().first())
        .expect("Failed to get framebuffer")
}

impl Console {
    fn new() -> Self {
        let framebuffer = get_framebuffer();
        Self {
            x: 0,
            y: 0,
            color: 0xFFFFFF,
            bg_color: 0x000000,
            width: framebuffer.width as usize,
            height: framebuffer.height as usize,
            pitch: framebuffer.pitch as usize,
        }
    }

    fn put_pixel(&mut self, x: usize, y: usize, color: u32) {
        if x >= self.width || y >= self.height {
            return;
        }

        let framebuffer = get_framebuffer();
        let ptr = framebuffer.address() as *mut u32;

        let offset = (y * self.pitch / 4) + x;
        unsafe {
            *ptr.add(offset) = color;
        }
    }

    fn draw_char(&mut self, c: char, x: usize, y: usize) {
        let char_code = c as usize;

        if char_code >= 256 {
            return;
        }

        for row in 0..FONT_HEIGHT {
            for col in 0..FONT_WIDTH {
                self.put_pixel(x + col, y + row, self.bg_color);
            }
        }

        let start = FONT_DATA_OFFSET + (char_code * FONT_HEIGHT);

        for (row, line) in (0..FONT_HEIGHT).enumerate() {
            let byte = FONT_FILE[start + row];
            for col in 0..FONT_WIDTH {
                if (byte >> (7 - col)) & 1 == 1 {
                    self.put_pixel(x + col, y + line, self.color);
                }
            }
        }
    }

    fn scroll(&mut self) {
        if self.y + FONT_HEIGHT > self.height {
            self.clear();
            self.y = 0;
        }
    }

    pub fn clear(&mut self) {
        let framebuffer = get_framebuffer();
        let ptr = framebuffer.address() as *mut u32;
        let size = self.width * self.height;
        unsafe {
            core::ptr::write_bytes(ptr, 0, size);
        }
    }

    pub fn write_char(&mut self, c: char) {
        match c {
            '\n' => {
                self.x = 0;
                self.y += FONT_HEIGHT;
            }
            '\r' => {
                self.x = 0;
            }
            '\t' => {
                self.x += 4 * FONT_WIDTH;
            }
            _ => {
                self.draw_char(c, self.x, self.y);
                self.x += FONT_WIDTH;
            }
        }
        self.scroll();
    }
    pub fn set_color(&mut self, color: Color) {
        self.color = color.to_u32();
    }
    pub fn set_bg_color(&mut self, bg_color: Color) {
        self.bg_color = bg_color.to_u32();
    }
}

impl fmt::Write for Console {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        for c in s.chars() {
            self.write_char(c);
        }
        Ok(())
    }
}

static CONSOLE: Once<Mutex<Console>> = Once::new();

pub fn init_console() {
    CONSOLE.call_once(|| Mutex::new(Console::new()));
}
pub fn with_console<F, R>(f: F) -> R
where
    F: FnOnce(&mut Console) -> R,
{
    let console = CONSOLE.get().expect("Console not initialized");
    let mut guard = console.lock();
    f(&mut guard)
}
