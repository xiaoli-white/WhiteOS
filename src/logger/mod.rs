pub use log::{debug, error, info, trace, warn};

use log::{Level, LevelFilter, Log, Metadata, Record};
use polished_serial_logging::serial_println;

use crate::console::{Color, Console, with_console};
use core::fmt::Write;

struct Logger;

impl Logger {
    fn set_console_color(console: &mut Console, level: Level) {
        console.set_color(match level {
            Level::Trace => Color::Blue,
            Level::Debug => Color::Green,
            Level::Info => Color::White,
            Level::Warn => Color::Yellow,
            Level::Error => Color::Red,
        });
    }
}
impl Log for Logger {
    fn enabled(&self, _metadata: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            let file = record
                .file()
                .and_then(|s| s.rsplit('/').next())
                .unwrap_or("???");
            let line = record.line().unwrap_or(0);
            if cfg!(debug_assertions) {
                serial_println!("[{}] ({}:{}) {}", record.level(), file, line, record.args());
                with_console(|console| {
                    Self::set_console_color(console, record.level());
                    writeln!(
                        console,
                        "[{}] ({}:{}) {}",
                        record.level(),
                        file,
                        line,
                        record.args()
                    )
                    .unwrap();
                });
            } else {
                serial_println!("[{}] {}", record.level(), record.args());
                with_console(|console| {
                    Self::set_console_color(console, record.level());
                    writeln!(console, "[{}] {}", record.level(), record.args()).unwrap();
                });
            }
        }
    }

    fn flush(&self) {}
}

pub fn init_logger() {
    log::set_logger(&Logger).expect("Logger already exist");
    log::set_max_level(LevelFilter::Debug);
}
