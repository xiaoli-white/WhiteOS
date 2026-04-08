use limine::memmap;
use limine::request::{HhdmRequest, MemmapRequest};
use x86_64::PhysAddr;
use x86_64::registers::control::Cr3;
use x86_64::structures::paging::{FrameAllocator, PhysFrame, Size4KiB};
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

pub struct PhysicalFrameAllocator {
    entries: &'static [&'static memmap::Entry],
    next: usize,
}

impl PhysicalFrameAllocator {
    fn usable_frames(&self) -> impl Iterator<Item = PhysFrame> {
        let usable_regions = self
            .entries
            .iter()
            .filter(|entry| entry.type_ == memmap::MEMMAP_USABLE);
        let addr_ranges = usable_regions.map(|entry| entry.base..(entry.base + entry.length));
        let frame_addresses = addr_ranges.flat_map(|r| r.step_by(4096));
        frame_addresses.map(|addr| PhysFrame::containing_address(PhysAddr::new(addr)))
    }
}
impl Default for PhysicalFrameAllocator {
    fn default() -> Self {
        PhysicalFrameAllocator {
            entries: MMAP_REQUEST.response().unwrap().entries(),
            next: 0,
        }
    }
}
unsafe impl FrameAllocator<Size4KiB> for PhysicalFrameAllocator {
    fn allocate_frame(&mut self) -> Option<PhysFrame> {
        let frame = self.usable_frames().nth(self.next);
        self.next += 1;
        frame
    }
}
