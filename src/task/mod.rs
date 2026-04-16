use core::arch::asm;

use x86_64::registers::read_rip;

#[derive(Debug, Default)]
pub struct TaskContext {
    pub rip: u64,
    pub rbx: u64,
    pub rbp: u64,
    pub rsp: u64,
    pub r12: u64,
    pub r13: u64,
    pub r14: u64,
    pub r15: u64,
}
impl TaskContext {
    pub fn current() -> Self {
        let mut context = Self {
            rip: read_rip().as_u64(),
            ..Default::default()
        };
        unsafe {
            asm!(
                "mov {0}, rbx",
                "mov {1}, rbp",
                "mov {2}, rsp",
                "mov {3}, r12",
                "mov {4}, r13",
                "mov {5}, r14",
                "mov {6}, r15",
                out(reg) context.rbx,
                out(reg) context.rbp,
                out(reg) context.rsp,
                out(reg) context.r12,
                out(reg) context.r13,
                out(reg) context.r14,
                out(reg) context.r15
            );
        }
        context
    }
    pub fn load(&self) -> ! {
        unsafe {
            asm!(
                "mov rbx, {0}",
                "mov rbp, {1}",
                "mov rsp, {2}",
                "mov r12, {3}",
                "mov r13, {4}",
                "mov r14, {5}",
                "mov r15, {6}",
                "jmp {7}",
                in(reg) self.rbx,
                in(reg) self.rbp,
                in(reg) self.rsp,
                in(reg) self.r12,
                in(reg) self.r13,
                in(reg) self.r14,
                in(reg) self.r15,
                in(reg) self.rip,
                options(noreturn)
            );
        }
    }
}

#[derive(Debug)]
pub struct Task {
    pub id: usize,
    pub context: TaskContext,
    pub state: TaskState,
}

#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(u8)]
pub enum TaskState {
    Ready = 0,
    Running,
    Blocked,
}
