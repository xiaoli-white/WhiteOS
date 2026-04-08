use limine::request::{HhdmRequest, MemmapRequest};
use x86_64::registers::control::Cr3;
use x86_64::{VirtAddr, structures::paging::PageTable};

#[used]
#[unsafe(link_section = ".limine_requests")]
pub static MMAP_REQUEST: MemmapRequest = MemmapRequest::new();

#[used]
#[unsafe(link_section = ".limine_requests")]
pub static HHDM_REQUEST: HhdmRequest = HhdmRequest::new();

pub fn active_level_4_table() -> &'static mut PageTable {
    let (level_4_table_frame, _) = Cr3::read();
    let phys = level_4_table_frame.start_address();
    let virt = VirtAddr::new(HHDM_REQUEST.response().unwrap().offset + phys.as_u64());
    let page_table_ptr: *mut PageTable = virt.as_mut_ptr();

    unsafe { &mut *page_table_ptr }
}
