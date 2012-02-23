/*
 * =====================================================================================
 *
 *       Filename:  mm64_struct.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10.11.2011 10:31:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef MM_STRUCT_H
#define MM_STRUCT_H

#if __x86_64__

typedef union {
    uint64_t u64;
    /* structure for a reference to pd2/pdpe (bit 7, ps: must be 0), Intel 3A, Table 4-14 */
    struct {
        uint64_t p     :  1;    /* bit 0 - Present          - must be 1 to reference a pd2/pdpe */
        uint64_t rw    :  1;    /* bit 1 - Read/Write       - 0: read-only (depends on CPL and CR0l.WP) */
        uint64_t us    :  1;    /* bit 2 - User/Supervisor  - 0: no access for CPL=3 for 512 GB region */
        uint64_t pwt   :  1;    /* bit 3 - Page lvl Write-through  */
        uint64_t pcd   :  1;    /* bit 4 - Page lvl Cache Disable  */
        uint64_t a     :  1;    /* bit 5 - Accessed         - indicates if this entry has been used */
        uint64_t ign   :  1;    /* bit 6 - ignored */
        uint64_t ps    :  1;    /* bit 7 - Page Size        - must be 0 (i.e. reference to pd2/pdpe) */
        uint64_t ign2  :  1;    /* bit 8 - ignored */
        uint64_t avl1  :  3;    /* 9,10,11 - ignored        - AMD: available */
        uint64_t frame : 40;    /* bit 12..51 - Phys adr    - reference to 4kB aligned pd2/pdpe */
        uint64_t avl2  : 11;    /* bit 52..62 - ign.        - ignored/available */
        uint64_t nx    :  1;    /* bit 63 - eXecute Disable - if IA32_EFER.NXE=1 : 1: no execute from 512 GB region */
    } dir;
} pd1_entry_t;     /*  pd1 (page directory 1st level): pml4 */

typedef union {
    uint64_t u64;
    /* structure for a reference to pd3/pde (bit 7, ps: must be 0), Intel 3A, Table 4-16 */
    struct {
        uint64_t p     :  1;    /* bit 0 - Present          - must be 1 to map a 1-GB page */
        uint64_t rw    :  1;    /* bit 1 - Read/Write       - 0: read-only (depends on CPL and CR0l.WP) */
        uint64_t us    :  1;    /* bit 2 - User/Supervisor  - 0: no access for CPL=3 for 1 GB region */
        uint64_t pwt   :  1;    /* bit 3 - Page lvl Write-through  */
        uint64_t pcd   :  1;    /* bit 4 - Page lvl Cache Disable  */
        uint64_t a     :  1;    /* bit 5 - Accessed         - indicates if this entry has been used */
        uint64_t ign   :  1;    /* bit 6 - ignored */
        uint64_t ps    :  1;    /* bit 7 - Page Size        - must be 0: reference to pd3/pde */
        uint64_t mbz   :  1;    /* bit 8 - ignored */
        uint64_t avl1  :  3;    /* 9,10,11 - ignored        - AMD: available*/
        uint64_t frame : 40;    /* 12..51 - Phys adr        - reference to 4kB aligned pd3/pde */
        uint64_t avl2  : 11;    /* 52..62 - ignored */
        uint64_t nx    :  1;    /* bit 63 - eXecute Disable - 1: no execute from 1 GB region */
    } dir;
    /* structure for an 1 GB huge page (bit 7, ps: must be 1), Intel 3A, Table 4-15 */
    struct {
        uint64_t p     :  1;    /* bit 0 - Present          - must be 1 to map a 1-GB page */
        uint64_t rw    :  1;    /* bit 1 - Read/Write       - 0: read-only */
        uint64_t us    :  1;    /* bit 2 - User/Supervisor  - 0: read-only for CPL=3 for 1 GB region */
        uint64_t pwt   :  1;    /* bit 3 - Page lvl Write-through */
        uint64_t pcd   :  1;    /* bit 4 - Page lvl Cache Disable */
        uint64_t a     :  1;    /* bit 5 - Accessed */
        uint64_t d     :  1;    /* BIT 6 - Dirty */
        uint64_t ps    :  1;    /* bit 7 - Page Size        - must be 1: abort table walk, use 1GB very huge page */
        uint64_t g     :  1;    /* bit 8 - Global           - translation is global (if CR4.PGE=1) */
        uint64_t avl1  :  3;    /* 9,10,11 - ignored */
        uint64_t pat   :  1;    /* bit 12 */
        uint64_t res   : 17;    /* 13..29 - reserved        - must be 0 (upper bits of 1GB offset) */
        uint64_t frame1G : 22;    /* 30..51 - Phys adr        - reference to a 1 GB-page */
        uint64_t avl2  : 11;    /* 52..62 - ignored */
        uint64_t nx    :  1;    /* bit 63 - eXecute Disable - 1: no execute from 1 GB page */
    } page;
} pd2_entry_t;     /* pd2 (page directory 2nd level): pdpe */

typedef union {
    uint64_t u64;
    /* structure for a reference to pt/pte (bit 7, ps: must be 0), Intel 3A, Table 4-18 */
    struct {
        uint64_t p     :  1;    /* bit 0 */
        uint64_t rw    :  1;    /* bit 1 */
        uint64_t us    :  1;    /* bit 2 */
        uint64_t pwt   :  1;    /* bit 3 */
        uint64_t pcd   :  1;    /* bit 4 */
        uint64_t a     :  1;    /* bit 5 */
        uint64_t ign   :  1;    /* bit 6 */
        uint64_t ps    :  1;    /* bit 7 - Page Size - 0: 4kB regular page, 1: abort table walk, use 2MB huge page */
        uint64_t ign2  :  1;    /* bit 8 */
        uint64_t avl1  :  3;    /* 9,10,11 */
        uint64_t frame : 40;    /* 12..51  */
        uint64_t avl2  : 11;    /* 52..62 */
        uint64_t nx    :  1;    /* bit 62 */
    } dir;
    /* structure for a 2 MB huge page (bit 7, ps: must be 1), Intel 3A, Table 4-17 */
    struct {
        uint64_t p     :  1;    /* bit 0 */
        uint64_t rw    :  1;    /* bit 1 */
        uint64_t us    :  1;    /* bit 2 */
        uint64_t pwt   :  1;    /* bit 3 */
        uint64_t pcd   :  1;    /* bit 4 */
        uint64_t a     :  1;    /* bit 5 */
        uint64_t d     :  1;    /* bit 6 */
        uint64_t ps    :  1;    /* bit 7 - Page Size - 0: 4kB regular page, 1: abort table walk, use 2MB huge page */
        uint64_t g     :  1;    /* bit 8 */
        uint64_t avl1  :  3;    /* 9,10,11 */
        uint64_t pat   :  1;    /* bit 12 */
        uint64_t res   :  8;    /* 13..20 */
        uint64_t frame2M : 31;    /* 21..51 */
        uint64_t avl2  : 11;    /* 52..62 */
        uint64_t nx    :  1;    /* bit 63 */
    } page;
} pd3_entry_t;      /* pd3 (page directory 3rd level): pde */

typedef union {
    uint64_t u64;
    /* structure for a 6 kB page (pat moves from bit 12 to bit 7), Intel 3A, Table 4-19 */
    struct {
        uint64_t p     :  1;    /* bit 0 */
        uint64_t rw    :  1;    /* bit 1 */
        uint64_t us    :  1;    /* bit 2 */
        uint64_t pwt   :  1;    /* bit 3 */
        uint64_t pcd   :  1;    /* bit 4 */
        uint64_t a     :  1;    /* bit 5 */
        uint64_t d     :  1;    /* bit 6 */
        uint64_t pat   :  1;    /* bit 7 - memory type */
        uint64_t g     :  1;    /* bit 8 */
        uint64_t avl1  :  3;    /* 9..11 */
        uint64_t frame : 40;    /* 12..51 */
        uint64_t avl2  : 11;    /* 52..62 */
        uint64_t nx    :  1;    /* bit 63 */
    } page;
} pt_entry_t;      /* pt (page table): pte */

#else // __x86_64__

typedef union {
    uint32_t u32;
    struct {
        uint32_t p     :  1;    /* bit 0 - Present          - must be 1 to map a 4-MB page */
        uint32_t rw    :  1;    /* bit 1 */
        uint32_t us    :  1;    /* bit 2 */
        uint32_t pwt   :  1;    /* bit 3 */
        uint32_t pcd   :  1;    /* bit 4 */
        uint32_t a     :  1;    /* bit 5 */
        uint32_t ign   :  1;    /* bit 6 */
        uint32_t ps    :  1;    /* bit 7                    - Page Size - must be 0: reference to pd3/pde */
        uint32_t avl1  :  4;    /* 8..11 */
        uint32_t frame : 20;    /* 12..31 */
    } dir;
    struct {
        uint32_t p     :  1;    /* bit 0 - Present          - must be 1 to map a 4-MB page */
        uint32_t rw    :  1;    /* bit 1 - Read/Write       - 0: read-only */
        uint32_t us    :  1;    /* bit 2 - User/Supervisor  - 0: read-only for CPL=3 for 1 GB region */
        uint32_t pwt   :  1;    /* bit 3 - Page lvl Write-through */
        uint32_t pcd   :  1;    /* bit 4 - Page lvl Cache Disable */
        uint32_t a     :  1;    /* bit 5 - Accessed */
        uint32_t d     :  1;    /* BIT 6 - Dirty */
        uint32_t ps    :  1;    /* bit 7 - Page Size        - must be 1: abort table walk, use 4 MB huge page */
        uint32_t g     :  1;    /* bit 8 - Global           - translation is global (if CR4.PGE=1) */
        uint32_t avl1  :  3;    /* 9,10,11 */
        uint32_t pat   :  1;    /* bit 12 - PAT             -  */
        uint32_t ign   :  9;    /* 13..21 */
        uint32_t frame4M : 10;    /* 22..31 */
    } page;
} pd1_entry_t;     /* pd1 (page directory 1st level): pde  */


typedef union {
    uint32_t u32;
    struct {
        uint32_t p     :  1;    /* bit 0 */
        uint32_t rw    :  1;    /* bit 1 */
        uint32_t us    :  1;    /* bit 2 */
        uint32_t pwt   :  1;    /* bit 3 */
        uint32_t pcd   :  1;    /* bit 4 */
        uint32_t a     :  1;    /* bit 5 */
        uint32_t d     :  1;    /* bit 6 */
        uint32_t pat   :  1;    /* bit 7 - memory type */
        uint32_t g     :  1;    /* bit 8 */
        uint32_t avl1  :  3;    /* 9,10,11 */
        uint32_t frame : 20;    /* 12..31 */
    } page;
} pt_entry_t;      /* pt (page table)  */

#endif // __x86_64__

#endif // MM_STRUCT_H

