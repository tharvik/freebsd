/*-
 * Copyright (c) 2012-2015 Solarflare Communications Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the FreeBSD Project.
 *
 * $FreeBSD$
 */

/* These structures define the layouts for the TLV items stored in static and
 * dynamic configuration partitions in NVRAM for EF10 (Huntington etc.).
 *
 * They contain the same sort of information that was kept in the
 * siena_mc_static_config_hdr_t and siena_mc_dynamic_config_hdr_t structures
 * (defined in <ci/mgmt/mc_flash_layout.h> and <ci/mgmt/mc_dynamic_cfg.h>) for
 * Siena.
 *
 * These are used directly by the MC and should also be usable directly on host
 * systems which are little-endian and do not do strange things with structure
 * padding.  (Big-endian host systems will require some byte-swapping.)
 *
 *                                    -----
 *
 * Please refer to SF-108797-SW for a general overview of the TLV partition
 * format.
 *
 *                                    -----
 *
 * The current tag IDs have a general structure: with the exception of the
 * special values defined in the document, they are of the form 0xLTTTNNNN,
 * where:
 *
 *   -  L is a location, indicating where this tag is expected to be found:
 *      0 for static configuration, or 1 for dynamic configuration.   Other
 *      values are reserved.
 *
 *   -  TTT is a type, which is just a unique value.  The same type value
 *      might appear in both locations, indicating a relationship between
 *      the items (e.g. static and dynamic VPD below).
 *
 *   -  NNNN is an index of some form.  Some item types are per-port, some
 *      are per-PF, some are per-partition-type.
 *
 *                                    -----
 *
 * As with the previous Siena structures, each structure here is laid out
 * carefully: values are aligned to their natural boundary, with explicit
 * padding fields added where necessary.  (No, technically this does not
 * absolutely guarantee portability.  But, in practice, compilers are generally
 * sensible enough not to introduce completely pointless padding, and it works
 * well enough.)
 */


#ifndef CI_MGMT_TLV_LAYOUT_H
#define CI_MGMT_TLV_LAYOUT_H


/* ----------------------------------------------------------------------------
 *  General structure (defined by SF-108797-SW)
 * ----------------------------------------------------------------------------
 */


/* The "end" tag.
 *
 * (Note that this is *not* followed by length or value fields: anything after
 * the tag itself is irrelevant.)
 */

#define TLV_TAG_END                     (0xEEEEEEEE)


/* Other special reserved tag values.
 */

#define TLV_TAG_SKIP                    (0x00000000)
#define TLV_TAG_INVALID                 (0xFFFFFFFF)


/* TLV partition header.
 *
 * In a TLV partition, this must be the first item in the sequence, at offset
 * 0.
 */

#define TLV_TAG_PARTITION_HEADER        (0xEF10DA7A)

struct tlv_partition_header {
  uint32_t tag;
  uint32_t length;
  uint16_t type_id;
  uint16_t reserved;
  uint32_t generation;
  uint32_t total_length;
};


/* TLV partition trailer.
 *
 * In a TLV partition, this must be the last item in the sequence, immediately
 * preceding the TLV_TAG_END word.
 */

#define TLV_TAG_PARTITION_TRAILER       (0xEF101A57)

struct tlv_partition_trailer {
  uint32_t tag;
  uint32_t length;
  uint32_t generation;
  uint32_t checksum;
};


/* Appendable TLV partition header.
 *
 * In an appendable TLV partition, this must be the first item in the sequence,
 * at offset 0.  (Note that, unlike the configuration partitions, there is no
 * trailer before the TLV_TAG_END word.)
 */

#define TLV_TAG_APPENDABLE_PARTITION_HEADER (0xEF10ADA7)

struct tlv_appendable_partition_header {
  uint32_t tag;
  uint32_t length;
  uint16_t type_id;
  uint16_t reserved;
};


/* ----------------------------------------------------------------------------
 *  Configuration items
 * ----------------------------------------------------------------------------
 */


/* NIC global capabilities.
 */

#define TLV_TAG_GLOBAL_CAPABILITIES     (0x00010000)

struct tlv_global_capabilities {
  uint32_t tag;
  uint32_t length;
  uint32_t flags;
};


/* Siena-style per-port MAC address allocation.
 *
 * There are <count> addresses, starting at <base_address> and incrementing
 * by adding <stride> to the low-order byte(s).
 *
 * (See also TLV_TAG_GLOBAL_MAC for an alternative, specifying a global pool
 * of contiguous MAC addresses for the firmware to allocate as it sees fit.)
 */

#define TLV_TAG_PORT_MAC(port)          (0x00020000 + (port))

struct tlv_port_mac {
  uint32_t tag;
  uint32_t length;
  uint8_t  base_address[6];
  uint16_t reserved;
  uint16_t count;
  uint16_t stride;
};


/* Static VPD.
 *
 * This is the portion of VPD which is set at manufacturing time and not
 * expected to change.  It is formatted as a standard PCI VPD block.
 */

#define TLV_TAG_PF_STATIC_VPD(pf)       (0x00030000 + (pf))

struct tlv_pf_static_vpd {
  uint32_t tag;
  uint32_t length;
  uint8_t  bytes[];
};


/* Dynamic VPD.
 *
 * This is the portion of VPD which may be changed (e.g. by firmware updates).
 * It is formatted as a standard PCI VPD block.
 */

#define TLV_TAG_PF_DYNAMIC_VPD(pf)      (0x10030000 + (pf))

struct tlv_pf_dynamic_vpd {
  uint32_t tag;
  uint32_t length;
  uint8_t  bytes[];
};


/* "DBI" PCI config space changes.
 *
 * This is a set of edits made to the default PCI config space values before
 * the device is allowed to enumerate.
 */

#define TLV_TAG_PF_DBI(pf)              (0x00040000 + (pf))

struct tlv_pf_dbi {
  uint32_t tag;
  uint32_t length;
  struct {
    uint16_t addr;
    uint16_t byte_enables;
    uint32_t value;
  } items[];
};


/* Partition subtype codes.
 *
 * A subtype may optionally be stored for each type of partition present in
 * the NVRAM.  For example, this may be used to allow a generic firmware update
 * utility to select a specific variant of firmware for a specific variant of
 * board.
 *
 * The description[] field is an optional string which is returned in the
 * MC_CMD_NVRAM_METADATA response if present.
 */

#define TLV_TAG_PARTITION_SUBTYPE(type) (0x00050000 + (type))

struct tlv_partition_subtype {
  uint32_t tag;
  uint32_t length;
  uint32_t subtype;
  uint8_t  description[];
};


/* Partition version codes.
 *
 * A version may optionally be stored for each type of partition present in
 * the NVRAM.  This provides a standard way of tracking the currently stored
 * version of each of the various component images.
 */

#define TLV_TAG_PARTITION_VERSION(type) (0x10060000 + (type))

struct tlv_partition_version {
  uint32_t tag;
  uint32_t length;
  uint16_t version_w;
  uint16_t version_x;
  uint16_t version_y;
  uint16_t version_z;
};

/* Global PCIe configuration */

#define TLV_TAG_GLOBAL_PCIE_CONFIG (0x10070000)

struct tlv_pcie_config {
  uint32_t tag;
  uint32_t length;
  int16_t max_pf_number;                        /**< Largest PF RID (lower PFs may be hidden) */
  uint16_t pf_aper;                             /**< BIU aperture for PF BAR2 */
  uint16_t vf_aper;                             /**< BIU aperture for VF BAR0 */
  uint16_t int_aper;                            /**< BIU aperture for PF BAR4 and VF BAR2 */  
#define TLV_MAX_PF_DEFAULT (-1)                 /* Use FW default for largest PF RID  */
#define TLV_APER_DEFAULT (0xFFFF)               /* Use FW default for a given aperture */
};

/* Per-PF configuration. Note that not all these fields are necessarily useful
 * as the apertures are constrained by the BIU settings (the one case we do
 * use is to make BAR2 bigger than the BIU thinks to reserve space), but we can
 * tidy things up later */

#define TLV_TAG_PF_PCIE_CONFIG(pf)  (0x10080000 + (pf))

struct tlv_per_pf_pcie_config {
  uint32_t tag;
  uint32_t length;
  uint8_t vfs_total;
  uint8_t port_allocation;  
  uint16_t vectors_per_pf;
  uint16_t vectors_per_vf;
  uint8_t pf_bar0_aperture;
  uint8_t pf_bar2_aperture;
  uint8_t vf_bar0_aperture;
  uint8_t vf_base;  
  uint16_t supp_pagesz;
  uint16_t msix_vec_base;
};


/* Development ONLY. This is a single TLV tag for all the gubbins
 * that can be set through the MC command-line other than the PCIe
 * settings. This is a temporary measure. */
#define TLV_TAG_TMP_GUBBINS (0x10090000)

struct tlv_tmp_gubbins {
  uint32_t tag;
  uint32_t length;
  /* Consumed by dpcpu.c */
  uint64_t tx0_tags;     /* Bitmap */
  uint64_t tx1_tags;     /* Bitmap */
  uint64_t dl_tags;      /* Bitmap */
  uint32_t flags;
#define TLV_DPCPU_TX_STRIPE (1) /* TX striping is on */
#define TLV_DPCPU_BIU_TAGS  (2) /* Use BIU tag manager */
#define TLV_DPCPU_TX0_TAGS  (4) /* tx0_tags is valid */
#define TLV_DPCPU_TX1_TAGS  (8) /* tx1_tags is valid */
#define TLV_DPCPU_DL_TAGS  (16) /* dl_tags is valid */
  /* Consumed by features.c */
  uint32_t dut_features;        /* All 1s -> leave alone */
  int8_t with_rmon;             /* 0 -> off, 1 -> on, -1 -> leave alone */
  /* Consumed by clocks_hunt.c */
  int8_t clk_mode;             /* 0 -> off, 1 -> on, -1 -> leave alone */
  /* Consumed by sram.c */
  int8_t rx_dc_size;           /* -1 -> leave alone */
  int8_t tx_dc_size;
  int16_t num_q_allocs;
};

/* Global port configuration
 *
 * This is now deprecated in favour of a platform-provided default
 * and dynamic config override via tlv_global_port_options.
 */
#define TLV_TAG_GLOBAL_PORT_CONFIG      (0x000a0000)

struct tlv_global_port_config {
  uint32_t tag;
  uint32_t length;
  uint32_t ports_per_core;
  uint32_t max_port_speed;
};


/* Firmware options.
 *
 * This is intended for user-configurable selection of optional firmware
 * features and variants.
 *
 * Initially, this consists only of the satellite CPU firmware variant
 * selection, but this tag could be extended in the future (using the
 * tag length to determine whether additional fields are present).
 */

#define TLV_TAG_FIRMWARE_OPTIONS        (0x100b0000)

struct tlv_firmware_options {
  uint32_t tag;
  uint32_t length;
  uint32_t firmware_variant;
#define TLV_FIRMWARE_VARIANT_DRIVER_SELECTED (0xffffffff)

/* These are the values for overriding the driver's choice; the definitions
 * are taken from MCDI so that they don't get out of step.  Include
 * <ci/mgmt/mc_driver_pcol.h> or the equivalent from your driver's tree if
 * you need to use these constants.
 */
#define TLV_FIRMWARE_VARIANT_FULL_FEATURED   MC_CMD_FW_FULL_FEATURED
#define TLV_FIRMWARE_VARIANT_LOW_LATENCY     MC_CMD_FW_LOW_LATENCY
#define TLV_FIRMWARE_VARIANT_PACKED_STREAM   MC_CMD_FW_PACKED_STREAM
#define TLV_FIRMWARE_VARIANT_HIGH_TX_RATE    MC_CMD_FW_HIGH_TX_RATE
#define TLV_FIRMWARE_VARIANT_PACKED_STREAM_HASH_MODE_1 \
                                             MC_CMD_FW_PACKED_STREAM_HASH_MODE_1
};

/* Voltage settings
 * 
 * Intended for boards with A0 silicon where the core voltage may
 * need tweaking. Most likely set once when the pass voltage is 
 * determined. */

#define TLV_TAG_0V9_SETTINGS (0x000c0000)

struct tlv_0v9_settings {
  uint32_t tag;
  uint32_t length;  
  uint16_t flags; /* Boards with high 0v9 settings may need active cooling */
#define TLV_TAG_0V9_REQUIRES_FAN (1)
  uint16_t target_voltage; /* In millivolts */
  /* Since the limits are meant to be centred to the target (and must at least
   * contain it) they need setting as well. */
  uint16_t warn_low;       /* In millivolts */
  uint16_t warn_high;      /* In millivolts */
  uint16_t panic_low;      /* In millivolts */
  uint16_t panic_high;     /* In millivolts */   
};


/* Clock configuration */

#define TLV_TAG_CLOCK_CONFIG            (0x000d0000)

struct tlv_clock_config {
  uint32_t tag;
  uint32_t length;  
  uint16_t clk_sys;        /* MHz */
  uint16_t clk_dpcpu;      /* MHz */
  uint16_t clk_icore;      /* MHz */
  uint16_t clk_pcs;        /* MHz */
};

#define TLV_TAG_CLOCK_CONFIG_MEDFORD      (0x00100000)

struct tlv_clock_config_medford {
  uint32_t tag;
  uint32_t length;
  uint16_t clk_sys;        /* MHz */
  uint16_t clk_mc;         /* MHz */
  uint16_t clk_rmon;       /* MHz */
  uint16_t clk_vswitch;    /* MHz */
  uint16_t clk_dpcpu;      /* MHz */
  uint16_t clk_pcs;        /* MHz */
};


/* EF10-style global pool of MAC addresses.
 *
 * There are <count> addresses, starting at <base_address>, which are
 * contiguous.  Firmware is responsible for allocating addresses from this
 * pool to ports / PFs as appropriate.
 */

#define TLV_TAG_GLOBAL_MAC              (0x000e0000)

struct tlv_global_mac {
  uint32_t tag;
  uint32_t length;
  uint8_t  base_address[6];
  uint16_t reserved1;
  uint16_t count;
  uint16_t reserved2;
};

#define TLV_TAG_ATB_0V9_TARGET           (0x000f0000)

/* The target value for the 0v9 power rail measured on-chip at the
 * analogue test bus */
struct tlv_0v9_atb_target {
  uint32_t tag;
  uint32_t length;
  uint16_t millivolts;
  uint16_t reserved;
};

/* Global PCIe configuration, second revision. This represents the visible PFs
 * by a bitmap rather than having the number of the highest visible one. As such
 * it can (for a 16-PF chip) represent a superset of what TLV_TAG_GLOBAL_PCIE_CONFIG
 * can and it should be used in place of that tag in future (but compatibility with
 * the old tag will be left in the firmware indefinitely).  */

#define TLV_TAG_GLOBAL_PCIE_CONFIG_R2 (0x10100000)

struct tlv_pcie_config_r2 {
  uint32_t tag;
  uint32_t length;
  uint16_t visible_pfs;                         /**< Bitmap of visible PFs */
  uint16_t pf_aper;                             /**< BIU aperture for PF BAR2 */
  uint16_t vf_aper;                             /**< BIU aperture for VF BAR0 */
  uint16_t int_aper;                            /**< BIU aperture for PF BAR4 and VF BAR2 */  
};

/* Dynamic port mode.
 *
 * Allows selecting alternate port configuration for platforms that support it
 * (e.g. 1x40G vs 2x10G on Milano, 1x40G vs 4x10G on Medford). This affects the
 * number of externally visible ports (and, hence, PF to port mapping), so must
 * be done at boot time.
 *
 * This tag supercedes tlv_global_port_config.
 */

#define TLV_TAG_GLOBAL_PORT_MODE         (0x10110000)

struct tlv_global_port_mode {
  uint32_t tag;
  uint32_t length;
  uint32_t port_mode;
#define TLV_PORT_MODE_DEFAULT           (0xffffffff) /* Default for given platform */
#define TLV_PORT_MODE_10G                        (0) /* 10G, single SFP/10G-KR */
#define TLV_PORT_MODE_40G                        (1) /* 40G, single QSFP/40G-KR */
#define TLV_PORT_MODE_10G_10G                    (2) /* 2x10G, dual SFP/10G-KR or single QSFP */
#define TLV_PORT_MODE_40G_40G                    (3) /* 40G + 40G, dual QSFP/40G-KR (Greenport, Medford) */
#define TLV_PORT_MODE_10G_10G_10G_10G            (4) /* 2x10G + 2x10G, quad SFP/10G-KR or dual QSFP (Greenport, Medford) */
#define TLV_PORT_MODE_10G_10G_10G_10G_Q          (5) /* 4x10G, single QSFP, cage 0 (Medford) */
#define TLV_PORT_MODE_40G_10G_10G                (6) /* 1x40G + 2x10G, dual QSFP (Greenport, Medford) */
#define TLV_PORT_MODE_10G_10G_40G                (7) /* 2x10G + 1x40G, dual QSFP (Greenport, Medford) */
#define TLV_PORT_MODE_10G_10G_10G_10G_Q2         (8) /* 4x10G, single QSFP, cage 1 (Medford) */
#define TLV_PORT_MODE_MAX TLV_PORT_MODE_10G_10G_10G_10G_Q2
};

/* Type of the v-switch created implicitly by the firmware */

#define TLV_TAG_VSWITCH_TYPE(port)       (0x10120000 + (port))

struct tlv_vswitch_type {
  uint32_t tag;
  uint32_t length;
  uint32_t vswitch_type;
#define TLV_VSWITCH_TYPE_DEFAULT        (0xffffffff) /* Firmware default; equivalent to no TLV present for a given port */
#define TLV_VSWITCH_TYPE_NONE                    (0)
#define TLV_VSWITCH_TYPE_VLAN                    (1)
#define TLV_VSWITCH_TYPE_VEB                     (2)
#define TLV_VSWITCH_TYPE_VEPA                    (3)
#define TLV_VSWITCH_TYPE_MUX                     (4)
#define TLV_VSWITCH_TYPE_TEST                    (5)
};

/* A VLAN tag for the v-port created implicitly by the firmware */

#define TLV_TAG_VPORT_VLAN_TAG(pf)               (0x10130000 + (pf))

struct tlv_vport_vlan_tag {
  uint32_t tag;
  uint32_t length;
  uint32_t vlan_tag;
#define TLV_VPORT_NO_VLAN_TAG                    (0xFFFFFFFF) /* Default in the absence of TLV for a given PF */
};

/* Offset to be applied to the 0v9 setting, wherever it came from */

#define TLV_TAG_ATB_0V9_OFFSET           (0x10140000)

struct tlv_0v9_atb_offset {
  uint32_t tag;
  uint32_t length;
  int16_t  offset_millivolts;
  uint16_t reserved;
};

/* A privilege mask given on reset to all non-admin PCIe functions (that is other than first-PF-per-port).
 * The meaning of particular bits is defined in mcdi_ef10.yml under MC_CMD_PRIVILEGE_MASK, see also bug 44583.
 * TLV_TAG_PRIVILEGE_MASK_ADD specifies bits that should be added (ORed) to firmware default while
 * TLV_TAG_PRIVILEGE_MASK_REM specifies bits that should be removed (ANDed) from firmware default:
 * Initial_privilege_mask = (firmware_default_mask | privilege_mask_add) & ~privilege_mask_rem */

#define TLV_TAG_PRIVILEGE_MASK          (0x10150000) /* legacy symbol - do not use */

struct tlv_privilege_mask {                          /* legacy structure - do not use */
  uint32_t tag;
  uint32_t length;
  uint32_t privilege_mask;
};

#define TLV_TAG_PRIVILEGE_MASK_ADD      (0x10150000)

struct tlv_privilege_mask_add {
  uint32_t tag;
  uint32_t length;
  uint32_t privilege_mask_add;
};

#define TLV_TAG_PRIVILEGE_MASK_REM      (0x10160000)

struct tlv_privilege_mask_rem {
  uint32_t tag;
  uint32_t length;
  uint32_t privilege_mask_rem;
};

/* Additional privileges given to all PFs.
 * This tag takes precedence over TLV_TAG_PRIVILEGE_MASK_REM. */

#define TLV_TAG_PRIVILEGE_MASK_ADD_ALL_PFS         (0x10190000)

struct tlv_privilege_mask_add_all_pfs {
  uint32_t tag;
  uint32_t length;
  uint32_t privilege_mask_add;
};

/* Additional privileges given to a selected PF.
 * This tag takes precedence over TLV_TAG_PRIVILEGE_MASK_REM. */

#define TLV_TAG_PRIVILEGE_MASK_ADD_SINGLE_PF(pf)   (0x101A0000 + (pf))

struct tlv_privilege_mask_add_single_pf {
  uint32_t tag;
  uint32_t length;
  uint32_t privilege_mask_add;
};

/* Turning on/off the PFIOV mode.
 * This tag only takes effect if TLV_TAG_VSWITCH_TYPE is missing or set to DEFAULT. */

#define TLV_TAG_PFIOV(port)             (0x10170000 + (port))

struct tlv_pfiov {
  uint32_t tag;
  uint32_t length;
  uint32_t pfiov;
#define TLV_PFIOV_OFF                    (0) /* Default */
#define TLV_PFIOV_ON                     (1)
};

/* Multicast filter chaining mode selection.
 *
 * When enabled, multicast packets are delivered to all recipients of all
 * matching multicast filters, with the exception that IP multicast filters
 * will steal traffic from MAC multicast filters on a per-function basis.
 * (New behaviour.)
 *
 * When disabled, multicast packets will always be delivered only to the
 * recipients of the highest priority matching multicast filter.
 * (Legacy behaviour.)
 *
 * The DEFAULT mode (which is the same as the tag not being present at all)
 * is equivalent to ENABLED in production builds, and DISABLED in eftest
 * builds.
 *
 * This option is intended to provide run-time control over this feature
 * while it is being stabilised and may be withdrawn at some point in the
 * future; the new behaviour is intended to become the standard behaviour.
 */

#define TLV_TAG_MCAST_FILTER_CHAINING   (0x10180000)

struct tlv_mcast_filter_chaining {
  uint32_t tag;
  uint32_t length;
  uint32_t mode;
#define TLV_MCAST_FILTER_CHAINING_DEFAULT  (0xffffffff)
#define TLV_MCAST_FILTER_CHAINING_DISABLED (0)
#define TLV_MCAST_FILTER_CHAINING_ENABLED  (1)
};


/* Pacer rate limit per PF */
#define TLV_TAG_RATE_LIMIT(pf)    (0x101b0000 + (pf))

struct tlv_rate_limit {
  uint32_t tag;
  uint32_t length;
  uint32_t rate_mbps;
};


/* OCSD Enable/Disable
 *
 * This setting allows OCSD to be disabled. This is a requirement for HP
 * servers to support PCI passthrough for virtualization.
 *
 * The DEFAULT mode (which is the same as the tag not being present) is
 * equivalent to ENABLED.
 *
 * This option is not used by the MCFW, and is entirely handled by the various
 * drivers that support OCSD, by reading the setting before they attempt
 * to enable OCSD.
 *
 * bit0: OCSD Disabled/Enabled
 */

#define TLV_TAG_OCSD (0x101C0000)

struct tlv_ocsd {
  uint32_t tag;
  uint32_t length;
  uint32_t mode;
#define TLV_OCSD_DISABLED 0
#define TLV_OCSD_ENABLED 1 /* Default */
};

#endif /* CI_MGMT_TLV_LAYOUT_H */
