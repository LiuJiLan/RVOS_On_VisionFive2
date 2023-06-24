#include "os.h"

#define VIRTIO0 0x10001000UL
#define R(r) ((volatile uint32_t *)(VIRTIO0 + (r)))

#define QUEUE_SIZE 4
//  Queue Size value is always a power of 2. The maximum Queue Size value is 32768.
//  power of 2应该是有且仅有一个1
//  我们给了4, 是因为我们现在只能使用Legacy Interface, 且不支持VIRTIO_F_ANY_LAYOUT
//  意味着我们对blk发送一条命令要3个desc

//  手册:https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.html
//  宏命名来自xv6-riscv, 和virtio有些许不同


/*
    Virtio, transport分类下MMIO, Device分类下Block Device
*/

//  4 Virtio Transport Options > Virtio Over MMIO > Legacy interface
#define VIRTIO_MMIO_MAGIC_VALUE		    0x000 // 0x74726976
#define VIRTIO_MMIO_VERSION		        0x004 // version; 1 is legacy
#define VIRTIO_MMIO_DEVICE_ID		    0x008 // device type; 1 is net, 2 is disk
#define VIRTIO_MMIO_VENDOR_ID		    0x00c // 0x554d4551
#define VIRTIO_MMIO_DEVICE_FEATURES	    0x010
#define VIRTIO_MMIO_DRIVER_FEATURES	    0x020
#define VIRTIO_MMIO_GUEST_PAGE_SIZE	    0x028 // page size for PFN, write-only
#define VIRTIO_MMIO_QUEUE_SEL		    0x030 // select queue, write-only
#define VIRTIO_MMIO_QUEUE_NUM_MAX	    0x034 // max size of current queue, read-only
#define VIRTIO_MMIO_QUEUE_NUM		    0x038 // size of current queue, write-only
#define VIRTIO_MMIO_QUEUE_ALIGN		    0x03c // used ring alignment, write-only
#define VIRTIO_MMIO_QUEUE_PFN		    0x040 // physical page number for queue, read/write
#define VIRTIO_MMIO_QUEUE_READY		    0x044 // ready bit
#define VIRTIO_MMIO_QUEUE_NOTIFY	    0x050 // write-only
#define VIRTIO_MMIO_INTERRUPT_STATUS	0x060 // read-only
#define VIRTIO_MMIO_INTERRUPT_ACK	    0x064 // write-only
#define VIRTIO_MMIO_STATUS		        0x070 // read/write
#define VIRTIO_MMIO_CONFIG                      0x100

//  3.1 Device Initialization (virtio 通用status)
#define VIRTIO_CONFIG_S_ACKNOWLEDGE	1
#define VIRTIO_CONFIG_S_DRIVER		2
#define VIRTIO_CONFIG_S_DRIVER_OK	4
#define VIRTIO_CONFIG_S_FEATURES_OK	8

// device feature bits
//  5.2.3 (Block Device) Feature bits
#define VIRTIO_BLK_F_RO              5	/* Disk is read-only */
#define VIRTIO_BLK_F_SCSI            7	/* Supports scsi command passthru */
#define VIRTIO_BLK_F_CONFIG_WCE     11	/* Writeback mode available in config */
#define VIRTIO_BLK_F_MQ             12	/* support more than one vq */
// 6.3 Legacy Interface: Reserved Feature Bits
#define VIRTIO_F_ANY_LAYOUT         27
// 2.7.5.3.1 Driver Requirements: Indirect Descriptors
#define VIRTIO_RING_F_INDIRECT_DESC 28
#define VIRTIO_RING_F_EVENT_IDX     29

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint16_t le16;
typedef uint32_t le32;
typedef uint64_t le64;

struct virtq_desc { 
        /* Address (guest-physical). */ 
        le64 addr; 
        /* Length. */ 
        le32 len; 
 
/* This marks a buffer as continuing via the next field. */ 
#define VIRTQ_DESC_F_NEXT   1 
/* This marks a buffer as device write-only (otherwise device read-only). */ 
#define VIRTQ_DESC_F_WRITE     2 
/* This means the buffer contains a list of buffer descriptors. */ 
#define VIRTQ_DESC_F_INDIRECT   4 
        /* The flags as indicated above. */ 
        le16 flags; 
        /* Next field if flags & NEXT */ 
        le16 next; 
};

struct virtq_avail { 
#define VIRTQ_AVAIL_F_NO_INTERRUPT      1 
        le16 flags; 
        le16 idx; 
        le16 ring[QUEUE_SIZE]; 
        le16 used_event; /* Only if VIRTIO_F_EVENT_IDX */ 
};

/* le32 is used here for ids for padding reasons. */ 
struct virtq_used_elem { 
        /* Index of start of used descriptor chain. */ 
        le32 id; 
        /* 
         * The number of bytes written into the device writable portion of 
         * the buffer described by the descriptor chain. 
         */ 
        le32 len; 
};

struct virtq_used { 
#define VIRTQ_USED_F_NO_NOTIFY  1 
        le16 flags; 
        le16 idx; 
        struct virtq_used_elem ring[QUEUE_SIZE]; 
        le16 avail_event; /* Only if VIRTIO_F_EVENT_IDX */ 
}; 
 
struct virtq { 
        // The actual descriptors (16 bytes each) 
        struct virtq_desc desc[QUEUE_SIZE]; 
 
        // A ring of available descriptor heads with free-running index. 
        struct virtq_avail avail; 
 
        // Padding to the next Queue Align boundary. 
        u8 pad[PGSIZE - 16*QUEUE_SIZE - (6+2*QUEUE_SIZE)]; 
 
        // A ring of used descriptor heads with free-running index. 
        struct virtq_used used; 
};

struct virtio_blk_config { 
        le64 capacity; 
        le32 size_max; 
        le32 seg_max; 
        struct virtio_blk_geometry { 
                le16 cylinders; 
                u8 heads; 
                u8 sectors; 
        } geometry; 
        le32 blk_size; 
        struct virtio_blk_topology { 
                // # of logical blocks per physical block (log2) 
                u8 physical_block_exp; 
                // offset of first aligned logical block 
                u8 alignment_offset; 
                // suggested minimum I/O size in blocks 
                le16 min_io_size; 
                // optimal (suggested maximum) I/O size in blocks 
                le32 opt_io_size; 
        } topology; 
        u8 writeback; 
        u8 unused0; 
        u16 num_queues; 
        le32 max_discard_sectors; 
        le32 max_discard_seg; 
        le32 discard_sector_alignment; 
        le32 max_write_zeroes_sectors; 
        le32 max_write_zeroes_seg; 
        u8 write_zeroes_may_unmap; 
        u8 unused1[3]; 
        le32 max_secure_erase_sectors; 
        le32 max_secure_erase_seg; 
        le32 secure_erase_sector_alignment; 
};

#define VIRTIO_BLK_T_IN  0 // read the disk
#define VIRTIO_BLK_T_OUT 1 // write the disk


struct virtio_blk_req {
  uint32_t type; // VIRTIO_BLK_T_IN or ..._OUT
  uint32_t reserved;
  uint64_t sector;
};

#define VIRTIO_BLK_S_OK        0
#define VIRTIO_BLK_S_IOERR     1
#define VIRTIO_BLK_S_UNSUPP    2