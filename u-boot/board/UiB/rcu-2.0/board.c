/*
 * board/UiB/rcu-2.0/board.c
 *
 * Board specific code for the SmartFusion2 SoC on the ALICE Time Projection Chamber RCU2 
 *
 ** This file is property of and copyright by the Experimental Nuclear 
 ** Physics Group, Dep. of Physics and Technology
 ** University of Bergen, Norway, 2014
 ** This file has been written by Lars Bratrud,
 ** Lars.Bratrud@cern.ch
 **
 ** Permission to use, copy, modify and distribute this software and its  
 ** documentation strictly for non-commercial purposes is hereby granted  
 ** without fee, provided that the above copyright notice appears in all  
 ** copies and that both the copyright notice and this permission notice  
 ** appear in the supporting documentation. The authors make no claims    
 ** about the suitability of this software for any purpose. It is         
 ** provided "as is" without express or implied warranty.   
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/ddr.h>
#include <asm/arch/comblk.h>

//#include <asm/arch-m2s/m2s.h>

//const uint8_t frame_size = 16u;

#define XMK_STR(x)      #x
#define MK_STR(x)       XMK_STR(x)

extern void exec_comblk_get_serial_number(void);
extern u8 uboot_pars_from_flash;

/*------------------------------------------------------------------------------
 * seriai number, mac address, serverip                                                                          
 */
typedef struct
{
  char *name;
  u32  serial[4];
  char *macaddr;
  char *ipaddr;
} cfg_serial_mac_ipaddr_value_t;

#define NUMBER_OF_RCU2_BOARD   231
const cfg_serial_mac_ipaddr_value_t g_m2s_serial_mac_config[] = {
  {"122C22001", {0x17000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:01:B0", "10.160.129.20"},
  {"122C22002", {0x29001b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:02:B0", "10.160.129.21"},
  {"122C22003", {0x28001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:03:B0", "10.160.129.22"},
  {"122C22004", {0x1d001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:04:B0", "10.160.129.23"},
  {"122C22005", {0x28001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:05:B0", "10.160.129.24"},
  {"122C22006", {0x21001c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:06:B0", "10.160.129.25"},
  {"122C22007", {0x20000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:07:B0", "10.160.129.30"},
  {"122C22008", {0x16001000, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:08:B0", "10.160.129.31"},
  {"122C22009", {0x2e001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:09:B0", "10.160.129.32"},
  {"122C22010", {0x20001e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:0A:B0", "10.160.129.33"},
  {"122C22011", {0x26001200, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:0B:B0", "10.160.129.34"},
  {"122C22012", {0x23000a00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:0C:B0", "10.160.129.35"},
  {"122C22013", {0x19000d00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:0D:B0", "10.160.129.40"},
  {"122C22014", {0x2b000a00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:0E:B0", "10.160.129.41"},
  {"122C22015", {0x16000c00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:0F:B0", "10.160.129.42"},
  {"122C22016", {0x27000c00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:10:B0", "10.160.129.43"},
  {"122C22017", {0x19000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:11:B0", "10.160.129.44"},
  {"122C22018", {0x28001d00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:12:B0", "10.160.129.45"},
  {"122C22019", {0x21001b00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:13:B0", "10.160.129.50"},
  {"122C22020", {0x1d000600, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:14:B0", "10.160.129.51"},
  {"122C22021", {0x1f001100, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:15:B0", "10.160.129.52"},
  {"122C22022", {0x26001900, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:16:B0", "10.160.129.53"},
  {"122C22023", {0x21001200, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:17:B0", "10.160.129.54"},
  {"122C22024", {0x1e000800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:18:B0", "10.160.129.55"},
  {"122C22025", {0x27001200, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:19:B0", "10.160.129.60"},
  {"122C22026", {0x12001500, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:1A:B0", "10.160.129.61"},
  {"122C22027", {0x20001100, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:1B:B0", "10.160.129.62"},
  {"122C22028", {0x17000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:1C:B0", "10.160.129.63"},
  {"122C22029", {0x27001600, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:1D:B0", "10.160.129.64"},
  {"122C22030", {0x17000a00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:1E:B0", "10.160.129.65"},
  {"122C22031", {0x1d000800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:1F:B0", "10.160.129.70"},
  {"122C22032", {0x17001400, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:20:B0", "10.160.129.71"},
  {"122C22033", {0x17000600, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:21:B0", "10.160.129.72"},
  {"122C22034", {0x1f001900, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:22:B0", "10.160.129.73"},
  {"122C22035", {0x13000a00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:23:B0", "10.160.129.74"},
  {"122C22036", {0x18001900, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:24:B0", "10.160.129.75"},
  {"122C22037", {0x13001900, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:25:B0", "10.160.129.80"},
  {"122C22038", {0x2a000c00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:26:B0", "10.160.129.81"},
  {"122C22039", {0x1a000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:27:B0", "10.160.129.82"},
  {"122C22040", {0x14001800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:28:B0", "10.160.129.83"},
  {"122C22041", {0x25001700, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:29:B0", "10.160.129.84"},
  {"122C22042", {0x12001400, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:2A:B0", "10.160.129.85"},
  {"122C22043", {0x16001400, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:2B:B0", "10.160.129.90"},
  {"122C22044", {0x1a000800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:2C:B0", "10.160.129.91"},
  {"122C22045", {0x1c000800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:2D:B0", "10.160.129.92"},
  {"122C22046", {0x15000800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:2E:B0", "10.160.129.93"},
  {"122C22047", {0x20000400, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:2F:B0", "10.160.129.94"},
  {"122C22048", {0x1a001c00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:30:B0", "10.160.129.95"},
  {"122C22049", {0x24000500, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:31:B0", "10.160.129.100"},
  {"122C22050", {0x19001e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:32:B0", "10.160.129.101"},
  {"122C22051", {0x1f000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:33:B0", "10.160.129.102"},
  {"122C22052", {0x2b001800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:34:B0", "10.160.129.103"},
  {"122C22053", {0x13001400, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:35:B0", "10.160.129.104"},
  {"122C22054", {0x2e001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:36:B0", "10.160.129.105"},
  {"122C22055", {0x1b000600, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:37:B0", "10.160.129.110"},
  {"122C22056", {0x27001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:38:B0", "10.160.129.111"},
  {"122C22057", {0x16000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:39:B0", "10.160.129.112"},
  {"122C22058", {0x18000b00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:3A:B0", "10.160.129.113"},
  {"122C22059", {0x23000a00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:3B:B0", "10.160.129.114"},
  {"122C22060", {0x25000500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:3C:B0", "10.160.129.115"},
  {"122C22061", {0x14001900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:3D:B0", "10.160.129.120"},
  {"122C22062", {0x11000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:3E:B0", "10.160.129.121"},
  {"122C22063", {0x13000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:3F:B0", "10.160.129.122"},
  {"122C22064", {0x21001900, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:40:B0", "10.160.129.123"},
  {"122C22065", {0x2b001700, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:41:B0", "10.160.129.124"},
  {"122C22067", {0x20001c00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:43:B0", "10.160.129.130"},
  {"122C22068", {0x18000e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:44:B0", "10.160.129.131"},
  {"122C22069", {0x22001900, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:45:B0", "10.160.129.132"},
  {"122C22070", {0x16000b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:46:B0", "10.160.129.133"},
  {"122C22071", {0x17001100, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:47:B0", "10.160.129.134"},
  {"122C22072", {0x14000a00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:48:B0", "10.160.129.135"},
  {"122C22073", {0x24001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:49:B0", "10.160.129.140"},
  {"122C22074", {0x14000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:4A:B0", "10.160.129.141"},
  {"122C22075", {0x22000e00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:4B:B0", "10.160.129.142"},
  {"122C22076", {0x15000c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:4C:B0", "10.160.129.143"},
  {"122C22077", {0x29000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:4D:B0", "10.160.129.144"},
  {"122C22078", {0x16000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:4E:B0", "10.160.129.145"},
  {"122C22079", {0x1c001100, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:4F:B0", "10.160.129.150"},
  {"122C22080", {0x1b000600, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:50:B0", "10.160.129.151"},
  {"122C22081", {0x1c001c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:51:B0", "10.160.129.152"},
  {"122C22082", {0x26000700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:52:B0", "10.160.129.153"},
  {"122C22083", {0x20001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:53:B0", "10.160.129.154"},
  {"122C22084", {0x11000f00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:54:B0", "10.160.129.155"},
  {"122C22085", {0x1b001b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:55:B0", "10.160.129.160"},
  {"122C22086", {0x2d001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:56:B0", "10.160.129.161"},
  {"122C22087", {0x17001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:57:B0", "10.160.129.162"},
  {"122C22088", {0x1a001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:58:B0", "10.160.129.163"},
  {"122C22089", {0x1d000700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:59:B0", "10.160.129.164"},
  {"122C22090", {0x26001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:5A:B0", "10.160.129.165"},
  {"122C22091", {0x1a000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:5B:B0", "10.160.129.170"},
  {"122C22092", {0x12000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:5C:B0", "10.160.129.171"},
  {"122C22093", {0x12000c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:5D:B0", "10.160.129.172"},
  {"122C22094", {0x1a001100, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:5E:B0", "10.160.129.173"},
  {"122C22095", {0x13000e00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:5F:B0", "10.160.129.174"},
  {"122C22096", {0x1f001b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:60:B0", "10.160.129.175"},
  {"122C22097", {0x1a001b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:61:B0", "10.160.129.180"},
  {"122C22098", {0x1d001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:62:B0", "10.160.129.181"},
  {"122C22099", {0x2c000b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:63:B0", "10.160.129.182"},
  {"122C22100", {0x22001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:64:B0", "10.160.129.183"},
  {"122C22101", {0x2d000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:65:B0", "10.160.129.184"},
  {"122C22102", {0x20000600, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:66:B0", "10.160.129.185"},
  {"122C22103", {0x2c001600, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:67:B0", "10.160.129.190"},
  {"122C22104", {0x19000600, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:68:B0", "10.160.129.191"},
  {"122C22105", {0x1c000c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:69:B0", "10.160.129.192"},
  {"122C22106", {0x24001b00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:6A:B0", "10.160.129.193"},
  {"122C22107", {0x12000c00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:6B:B0", "10.160.129.194"},
  {"122C22108", {0x26000a00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:6C:B0", "10.160.129.195"},
  {"122C22109", {0x1a000500, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:6D:B0", "10.160.142.20"},
  {"122C22110", {0x1c000a00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:6E:B0", "10.160.142.21"},
  {"122C22111", {0x1c000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:6F:B0", "10.160.142.22"},
  {"122C22112", {0x12001700, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:70:B0", "10.160.142.23"},
  {"122C22113", {0x22001c00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:71:B0", "10.160.142.24"},
  {"122C22114", {0x1a001e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:72:B0", "10.160.142.25"},
  {"122C22115", {0x29001d00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:73:B0", "10.160.142.30"},
  {"122C22116", {0x2a001300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:74:B0", "10.160.142.31"},
  {"122C22117", {0x27000700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:75:B0", "10.160.142.32"},
  {"122C22118", {0x13001500, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:76:B0", "10.160.142.33"},
  {"122C22119", {0x1f000800, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:77:B0", "10.160.142.34"},
  {"122C22120", {0x1e000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:78:B0", "10.160.142.35"},
  {"122C22121", {0x2b000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:79:B0", "10.160.142.40"},
  {"122C22122", {0x1d000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:7A:B0", "10.160.142.41"},
  {"122C22123", {0x1d001900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:7B:B0", "10.160.142.42"},
  {"122C22124", {0x23001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:7C:B0", "10.160.142.43"},
  {"122C22125", {0x15001100, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:7D:B0", "10.160.142.44"},
  {"122C22126", {0x17000b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:7E:B0", "10.160.142.45"},
  {"122C22127", {0x26000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:7F:B0", "10.160.142.50"},
  {"122C22128", {0x25000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:80:B0", "10.160.142.51"},
  {"122C22129", {0x1e001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:81:B0", "10.160.142.52"},
  {"122C22130", {0x19000a00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:82:B0", "10.160.142.53"},
  {"122C22131", {0x2f001300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:83:B0", "10.160.142.54"},
  {"122C22132", {0x13000a00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:84:B0", "10.160.142.55"},
  {"122C22133", {0x18000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:85:B0", "10.160.142.60"},
  {"122C22134", {0x1c000300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:86:B0", "10.160.142.61"},
  {"122C22135", {0x2f001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:87:B0", "10.160.142.62"},
  {"122C22136", {0x21000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:88:B0", "10.160.142.63"},
  {"122C22137", {0x2a001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:89:B0", "10.160.142.64"},
  {"122C22138", {0x2b000b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:8A:B0", "10.160.142.65"},
  {"122C22139", {0x1e001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:8B:B0", "10.160.142.70"},
  {"122C22140", {0x14001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:8C:B0", "10.160.142.71"},
  {"122C22141", {0x29001300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:8D:B0", "10.160.142.72"},
  {"122C22142", {0x11001000, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:8E:B0", "10.160.142.73"},
  {"122C22143", {0x28001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:8F:B0", "10.160.142.74"},
  {"122C22144", {0x29001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:90:B0", "10.160.142.75"},
  {"122C22145", {0x1e000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:91:B0", "10.160.142.80"},
  {"122C22146", {0x22001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:92:B0", "10.160.142.81"},
  {"122C22147", {0x29000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:93:B0", "10.160.142.82"},
  {"122C22148", {0x2b001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:94:B0", "10.160.142.83"},
  {"122C22149", {0x2e001300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:95:B0", "10.160.142.84"},
  {"122C22150", {0x1b000300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:96:B0", "10.160.142.85"},
  {"122C22151", {0x1c001300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:97:B0", "10.160.142.90"},
  {"122C22152", {0x16000900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:98:B0", "10.160.142.91"},
  {"122C22153", {0x16000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:99:B0", "10.160.142.92"},
  {"122C22154", {0x24000300, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:9A:B0", "10.160.142.93"},
  {"122C22155", {0x1e001900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:9B:B0", "10.160.142.94"},
  {"122C22156", {0x12001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:9C:B0", "10.160.142.95"},
  {"122C22157", {0x13001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:9D:B0", "10.160.142.100"},
  {"122C22158", {0x22001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:9E:B0", "10.160.142.101"},
  {"122C22159", {0x14001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:9F:B0", "10.160.142.102"},
  {"122C22160", {0x10000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A0:B0", "10.160.142.103"},
  {"122C22161", {0x11000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A1:B0", "10.160.142.104"},
  {"122C22162", {0x21001900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A2:B0", "10.160.142.105"},
  {"122C22163", {0x25000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A3:B0", "10.160.142.110"},
  {"122C22164", {0x2b000c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A4:B0", "10.160.142.111"},
  {"122C22165", {0x2b000900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A5:B0", "10.160.142.112"},
  {"122C22166", {0x2a000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A6:B0", "10.160.142.113"},
  {"122C22167", {0x14000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A7:B0", "10.160.142.114"},
  {"122C22168", {0x1c000900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A8:B0", "10.160.142.115"},
  {"122C22169", {0x1a001000, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:A9:B0", "10.160.142.120"},
  {"122C22170", {0x23000700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:AA:B0", "10.160.142.121"},
  {"122C22171", {0x1a000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:AB:B0", "10.160.142.122"},
  {"122C22172", {0x1e000b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:AC:B0", "10.160.142.123"},
  {"122C22173", {0x16001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:AD:B0", "10.160.142.124"},
  {"122C22174", {0x1d000a00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:AE:B0", "10.160.142.125"},
  {"122C22175", {0x21001f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:AF:B0", "10.160.142.130"},
  {"122C22176", {0x20001000, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B0:B0", "10.160.142.131"},
  {"122C22177", {0x17000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B1:B0", "10.160.142.132"},
  {"122C22178", {0x24000500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B2:B0", "10.160.142.133"},
  {"122C22179", {0x1b000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B3:B0", "10.160.142.134"},
  {"122C22180", {0x23000b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B4:B0", "10.160.142.135"},
  {"122C22181", {0x1d001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B5:B0", "10.160.142.140"},
  {"122C22182", {0x20001c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B6:B0", "10.160.142.141"},
  {"122C22183", {0x27001b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B7:B0", "10.160.142.142"},
  {"122C22184", {0x13001400, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B8:B0", "10.160.142.143"},
  {"122C22185", {0x27001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:B9:B0", "10.160.142.144"},
  {"122C22186", {0x2e000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:BA:B0", "10.160.142.145"},
  {"122C22187", {0x15001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:BB:B0", "10.160.142.150"},
  {"122C22188", {0x1c001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:BC:B0", "10.160.142.151"},
  {"122C22189", {0x15000b00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:BD:B0", "10.160.142.152"},
  {"122C22190", {0x24000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:BE:B0", "10.160.142.153"},
  {"122C22191", {0x22001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:BF:B0", "10.160.142.154"},
  {"122C22192", {0x21001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C0:B0", "10.160.142.155"},
  {"122C22193", {0x23001600, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C1:B0", "10.160.142.160"},
  {"122C22194", {0x16001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C2:B0", "10.160.142.161"},
  {"122C22195", {0x1a001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C3:B0", "10.160.142.162"},
  {"122C22196", {0x1b000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C4:B0", "10.160.142.163"},
  {"122C22197", {0x20000e00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C5:B0", "10.160.142.164"},
  {"122C22198", {0x25000900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C6:B0", "10.160.142.165"},
  {"122C22199", {0x1f000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C7:B0", "10.160.142.170"},
  {"122C22200", {0x1c000e00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C8:B0", "10.160.142.171"},
  {"122C22201", {0x1e000200, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:C9:B0", "10.160.142.172"},
  {"122C22202", {0x2e001000, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:CA:B0", "10.160.142.173"},
  {"122C22203", {0x2c001800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:CB:B0", "10.160.142.174"},
  {"122C22204", {0x1a000d00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:CC:B0", "10.160.142.175"},
  {"122C22205", {0x23001900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:CD:B0", "10.160.142.180"},
  {"122C22206", {0x15001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:CE:B0", "10.160.142.181"},
  {"122C22207", {0x23000900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:CF:B0", "10.160.142.182"},
  {"122C22208", {0x1f001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D0:B0", "10.160.142.183"},
  {"122C22209", {0x1b001500, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D1:B0", "10.160.142.184"},
  {"122C22210", {0x14000900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D2:B0", "10.160.142.185"},
  {"122C22211", {0x23001100, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D3:B0", "10.160.142.190"},
  {"122C22212", {0x20000100, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D4:B0", "10.160.142.191"},
  {"122C22213", {0x24001100, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D5:B0", "10.160.142.192"},
  {"122C22214", {0x22001d00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D6:B0", "10.160.142.193"},
  {"122C22215", {0x23000600, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D7:B0", "10.160.142.194"},
  {"122C22216", {0x2a000e00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D8:B0", "10.160.142.195"},
  {"122C22217", {0x1c000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:D9:B0", "in"},
  {"122C22218", {0x22001e00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:DA:B0", "10.160.142.201"},
  {"122C22219", {0x1c001a00, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:DB:B0", "10.160.142.202"},
  {"122C22220", {0x19001100, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:DC:B0", "10.160.142.203"},
  {"122C22221", {0x11001400, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:DD:B0", "10.160.142.204"},
  {"122C22222", {0x25000f00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:DE:B0", "10.160.142.205"},
  {"122C22223", {0x1e001400, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:DF:B0", "in"},
  {"122C22224", {0x16000c00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E0:B0", "10.160.142.211"},
  {"122C22225", {0x19000900, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E1:B0", "10.160.142.212"},
  {"122C22226", {0x1e000700, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E2:B0", "-"},
  {"122C22227", {0x20000500, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E3:B0", "-"},
  {"122C22228", {0x2c000a00, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E4:B0", "-"},
  {"122C22229", {0x22000800, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E5:B0", "10.160.142.220"},
  {"122C22230", {0x1f001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E6:B0", "10.160.142.221"},
  {"122C22066", {0x1f001700, 0x08007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E6:B0", "10.160.142.221"},
  {"121B21002", {0x27001700, 0x06007800, 0x5acd0000, 0x04d88813}, "00:50:C2:DB:E6:BF", "10.160.142.223"}
};


/*
** DDL2 SERDES specific from SoftConsole
*/
typedef struct
{
    uint32_t * p_reg;
    uint32_t value;
} cfg_addr_value_pair_t;

//// SERDES DDL 2.125 Gb/s

uint32_t ddl2_speed;

#define SERDES_0_CFG_NB_OF_PAIRS_2Gbps 17u // From sys_config_SERDESIF_0//
const cfg_addr_value_pair_t g_m2s_serdes_0_config_2Gbps[] =
{
  { (uint32_t*)( 0x40028000 + 0x2028 ), 0x10F }, // SYSTEM_CONFIG_PHY_MODE_1 // ,
  { (uint32_t*)( 0x40028000 + 0x1198 ), 0x30 }, // LANE0_PHY_RESET_OVERRIDE // ,
  { (uint32_t*)( 0x40028000 + 0x1000 ), 0x80 }, // LANE0_CR0 // ,
  { (uint32_t*)( 0x40028000 + 0x1004 ), 0x20 }, // LANE0_ERRCNT_DEC // ,
  { (uint32_t*)( 0x40028000 + 0x1008 ), 0xF8 }, // LANE0_RXIDLE_MAX_ERRCNT_THR // ,
  { (uint32_t*)( 0x40028000 + 0x100c ), 0x80 }, // LANE0_IMPED_RATIO // ,
  { (uint32_t*)( 0x40028000 + 0x1014 ), 0x13 }, // LANE0_PLL_M_N // ,
  { (uint32_t*)( 0x40028000 + 0x1018 ), 0x1B }, // LANE0_CNT250NS_MAX // ,
  { (uint32_t*)( 0x40028000 + 0x1024 ), 0x80 }, // LANE0_TX_AMP_RATIO // ,
  { (uint32_t*)( 0x40028000 + 0x1030 ), 0x10 }, // LANE0_ENDCALIB_MAX // ,
  { (uint32_t*)( 0x40028000 + 0x1034 ), 0x38 }, // LANE0_CALIB_STABILITY_COUNT // ,
  { (uint32_t*)( 0x40028000 + 0x103c ), 0x70 }, // LANE0_RX_OFFSET_COUNT // ,
  { (uint32_t*)( 0x40028000 + 0x11d4 ), 0x2 }, // LANE0_GEN1_TX_PLL_CCP // ,
  { (uint32_t*)( 0x40028000 + 0x11d8 ), 0x2 }, // LANE0_GEN1_RX_PLL_CCP // ,
  { (uint32_t*)( 0x40028000 + 0x1198 ), 0x0 }, // LANE0_PHY_RESET_OVERRIDE // ,
  { (uint32_t*)( 0x40028000 + 0x1200 ), 0x1 }, // LANE0_UPDATE_SETTINGS // ,
  { (uint32_t*)( 0x40028000 + 0x2028 ), 0xF0F } // SYSTEM_CONFIG_PHY_MODE_1 // 
};

//// SERDES DDL 4.25 Gb/s
#define SERDES_0_CFG_NB_OF_PAIRS_4Gbps 18u // From sys_config_SERDESIF_0
const cfg_addr_value_pair_t g_m2s_serdes_0_config_4Gbps[] =
  {
    { (uint32_t*)( 0x40028000 + 0x2028 ), 0x10F }, // SYSTEM_CONFIG_PHY_MODE_1 // ,
    { (uint32_t*)( 0x40028000 + 0x1198 ), 0x30 }, // LANE0_PHY_RESET_OVERRIDE // ,
    { (uint32_t*)( 0x40028000 + 0x1000 ), 0x80 }, // LANE0_CR0 // ,
    { (uint32_t*)( 0x40028000 + 0x1004 ), 0x20 }, // LANE0_ERRCNT_DEC // ,
    { (uint32_t*)( 0x40028000 + 0x1008 ), 0xF8 }, // LANE0_RXIDLE_MAX_ERRCNT_THR // ,
    { (uint32_t*)( 0x40028000 + 0x100c ), 0x80 }, // LANE0_IMPED_RATIO // ,
    { (uint32_t*)( 0x40028000 + 0x1010 ), 0x1 }, // LANE0_PLL_F_PCLK_RATIO // ,
    { (uint32_t*)( 0x40028000 + 0x1014 ), 0x13 }, // LANE0_PLL_M_N // ,
    { (uint32_t*)( 0x40028000 + 0x1018 ), 0x36 }, // LANE0_CNT250NS_MAX // ,
    { (uint32_t*)( 0x40028000 + 0x1024 ), 0x80 }, // LANE0_TX_AMP_RATIO // ,
    { (uint32_t*)( 0x40028000 + 0x1030 ), 0x10 }, // LANE0_ENDCALIB_MAX // ,
    { (uint32_t*)( 0x40028000 + 0x1034 ), 0x38 }, // LANE0_CALIB_STABILITY_COUNT // ,
    { (uint32_t*)( 0x40028000 + 0x103c ), 0x70 }, // LANE0_RX_OFFSET_COUNT // ,
    { (uint32_t*)( 0x40028000 + 0x11d4 ), 0x7 }, // LANE0_GEN1_TX_PLL_CCP // ,
    { (uint32_t*)( 0x40028000 + 0x11d8 ), 0x57 }, // LANE0_GEN1_RX_PLL_CCP // ,
    { (uint32_t*)( 0x40028000 + 0x1198 ), 0x0 }, // LANE0_PHY_RESET_OVERRIDE // ,
    { (uint32_t*)( 0x40028000 + 0x1200 ), 0x1 }, // LANE0_UPDATE_SETTINGS // ,
    { (uint32_t*)( 0x40028000 + 0x2028 ), 0xF0F } // SYSTEM_CONFIG_PHY_MODE_1 // 
  };

#define SDIF_RELEASED_MASK  0x00000002u
#define CONFIG_1_DONE   1u
#define CONFIG_2_DONE   2u
#define SYSREG_FPGA_SOFTRESET_MASK          ( (uint32_t)0x01u << 16u )
#define INIT_DONE_MASK 0x00000001u


/*
 * SERDES1 registers for the Marvell M881111 Ethernet PHY
*/

#define SERDES1_CFG_BASE			0x4002C000
const cfg_addr_value_pair_t g_m2s_serdes_1_config[] =
{
    { (uint32_t*)( 0x4002C000 + 0x2028 ), 0x80F } /* SYSTEM_CONFIG_PHY_MODE_1 */ ,
    { (uint32_t*)( 0x4002C000 + 0x1d98 ), 0x30 } /* LANE3_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c00 ), 0x80 } /* LANE3_CR0 */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c04 ), 0x20 } /* LANE3_ERRCNT_DEC */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c08 ), 0xF8 } /* LANE3_RXIDLE_MAX_ERRCNT_THR */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c0c ), 0x80 } /* LANE3_IMPED_RATIO */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c14 ), 0x29 } /* LANE3_PLL_M_N */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c18 ), 0x20 } /* LANE3_CNT250NS_MAX */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c24 ), 0x80 } /* LANE3_TX_AMP_RATIO */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c30 ), 0x10 } /* LANE3_ENDCALIB_MAX */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c34 ), 0x38 } /* LANE3_CALIB_STABILITY_COUNT */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c3c ), 0x70 } /* LANE3_RX_OFFSET_COUNT */ ,
    { (uint32_t*)( 0x4002C000 + 0x1dd4 ), 0x2 } /* LANE3_GEN1_TX_PLL_CCP */ ,
    { (uint32_t*)( 0x4002C000 + 0x1dd8 ), 0x22 } /* LANE3_GEN1_RX_PLL_CCP */ ,
    { (uint32_t*)( 0x4002C000 + 0x1d98 ), 0x0 } /* LANE3_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x4002C000 + 0x1e00 ), 0x1 } /* LANE3_UPDATE_SETTINGS */ ,
    { (uint32_t*)( 0x4002C000 + 0x2028 ), 0xF0F } /* SYSTEM_CONFIG_PHY_MODE_1 */ 
};
#define SERDES_1_CFG_NB_OF_PAIRS 17u

/* 
 * SERDES0 registers for the DDL2
 * extracted from SoftConsole files generated by Libero 
 */

#define SERDES0_LANE0_REGS			0x40028000
#define SYSTEM_CONFIG_PHY_MODE_1 	0x2028
#define LANE0_PHY_RESET_OVERRIDE	0x1198
#define LANE0_CR0					0x1000
#define LANE0_ERRCNT_DEC			0x1004
#define LANE0_RXIDLE_MAX_ERRCNT_THR	0x1008
#define LANE0_IMPED_RATIO			0x100c
#define LANE0_PLL_M_N				0x1014
#define LANE0_CNT250NS_MAX			0x1018
#define LANE0_TX_AMP_RATIO			0x1024
#define LANE0_ENDCALIB_MAX			0x1030
#define LANE0_CALIB_STABILITY_COUNT 0x1034
#define LANE0_RX_OFFSET_COUNT		0x103c
#define LANE0_GEN1_TX_PLL_CCP		0x11d4
#define LANE0_GEN1_RX_PLL_CCP		0x11d8
#define LANE0_UPDATE_SETTINGS		0x1200

/*
 * Generate DDR timings depending on the following DDR clock
 */
#define M2S_DDR_MHZ		2*(CONFIG_SYS_M2S_SYSREF / (1000 * 1000))

/*
 * Common conversion macros used for DDR cfg
 */

#define M2S_CLK_VAL(ns, div)	((((ns) * M2S_DDR_MHZ) / div))
#define M2S_CLK_MOD(ns, div)	((((ns) * M2S_DDR_MHZ) % div))

#define M2S_CLK_MIN(ns)		(M2S_CLK_MOD(ns,1000) ?			\
				 M2S_CLK_VAL(ns,1000) + 1 :		\
				 M2S_CLK_VAL(ns,1000))
#define M2S_CLK32_MIN(ns)	(M2S_CLK_MOD(ns,32000) ?		\
				 M2S_CLK_VAL(ns,32000) + 1 :		\
				 M2S_CLK_VAL(ns,32000))
#define M2S_CLK1024_MIN(ns)	(M2S_CLK_MOD(ns,1024000) ?		\
				 M2S_CLK_VAL(ns,1024000) + 1 :		\
				 M2S_CLK_VAL(ns,1024000))
#define M2S_CLK_MAX(ns)		(M2S_CLK_VAL(ns,1000))
#define M2S_CLK32_MAX(ns)	(M2S_CLK_VAL(ns,32000))
#define M2S_CLK1024_MAX(ns)	(M2S_CLK_VAL(ns,1024000))



/*
 * MT41J256M8 params & timings
 */
//#define DDR_BL			8	/* Burst length (value)		*/
//#define DDR_MR_BL		3	/* Burst length (power of 2)	*/
//#define DDR_BT			0	/* Burst type int(1)/seq(0)	*/

//#define DDR_CL			9	/* CAS (read) latency		*/
//#define DDR_WL			1	/* Write latency		*/
/*#define DDR_tMRD		2*/
/*#define DDR_tWTR		2*/
/*#define DDR_tXP			1*/
/*#define DDR_tCKE		1*/

/*#define DDR_tRFC		M2S_CLK_MIN(72)*/
/*#define DDR_tREFI		M2S_CLK32_MAX(7800)*/
/*#define DDR_tCKE_pre		M2S_CLK1024_MIN(200000)*/
/*#define DDR_tCKE_post		M2S_CLK1024_MIN(400)*/
/*#define DDR_tRCD		M2S_CLK_MIN(18)*/
/*#define DDR_tRRD		M2S_CLK_MIN(12)*/
/*#define DDR_tRP			M2S_CLK_MIN(18)*/
/*#define DDR_tRC			M2S_CLK_MIN(60)*/
/*#define DDR_tRAS_max		M2S_CLK1024_MAX(70000)*/
/*#define DDR_tRAS_min		M2S_CLK_MIN(42)*/
/*#define DDR_tWR			M2S_CLK_MIN(15)*/

/*
 * There are no these timings exactly in spec, so take smth
 */
#define DDR_tCCD		2	/* 2-reads/writes (bank A to B)	*/

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
#if 0
	/* Values extracted from SoftConsole files generated by Libero */
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + SYSTEM_CONFIG_PHY_MODE_1) = 0x10F;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_PHY_RESET_OVERRIDE) = 0x30;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_CR0) = 0x80;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_ERRCNT_DEC) = 0x20;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_RXIDLE_MAX_ERRCNT_THR) = 0xF8;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_IMPED_RATIO) = 0x80;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_PLL_M_N) = 0x13;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_CNT250NS_MAX) = 0x1B;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_TX_AMP_RATIO) = 0x80;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_ENDCALIB_MAX) = 0x10;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_CALIB_STABILITY_COUNT) = 0x38;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_RX_OFFSET_COUNT) = 0x70;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_GEN1_TX_PLL_CCP) = 0x2;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_GEN1_RX_PLL_CCP) = 0x2;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_PHY_RESET_OVERRIDE) = 0x0;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_UPDATE_SETTINGS) = 0x1;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + SYSTEM_CONFIG_PHY_MODE_1) = 0xF0F;
	/* Signal to CoreSF2Reset that peripheral configuration registers have been written.*/
	CORE_SF2_CFG->config_done = 1u;
#endif


	//printf("SYSREG_ENVM_CR = 0x%x\n",  M2S_SYSREG->envm_cr);
	//printf("SYSREG_MSSDDR_FACC1_CR = 0x%x\n",  M2S_SYSREG->mssddr_facc1_cr);
	//printf("SYSREG_MSSDDR_FACC2_CR = 0x%x\n",  M2S_SYSREG->mssddr_facc2_cr);

	/* 
	** DDL2 SERDES stuff from SystemInit() in SoftConsole generated code
	*/	
	uint32_t sdif_released; /* if MSS_SYS_SERDES_CONFIG_BY_CORTEX */
	uint32_t init_done; /* if MSS_SYS_CORESF2RESET_USED */
	//uint32_t core_cfg_version; /* if MSS_SYS_SERDES_CONFIG_BY_CORTEX */
	/*core_cfg_version = CORE_SF2_CFG->IP_VERSION_SR;*/
	
#if 0
	/*configure_serdes_intf()*/
	uint32_t inc = 0;
	////////
	const char *s;
	ddl2_speed = 0;
	if((s = getenv("DDL2_CONFIG_SPEED"))==NULL){
	  printf("No DDL2_CONFIG_SPEED(=2,4) in the uBoot environments.. Set to 2.125Gbps\n"); 
	  ddl2_speed = 0;
	}else{
	  if(strcmp(s,"2") == 0){
	    printf("DDL2_CONFIG_SPEED=2\n"); 
	    ddl2_speed = 0;
	  }else if(strcmp(s,"4") == 0){
	    printf("DDL2_CONFIG_SPEED=4\n"); 
	    ddl2_speed = 1;
	  }else{
	    ddl2_speed = 0;
	  }
	}

	if(ddl2_speed==0){
	  printf("DDL2-SerDes for 2.125Gbps\n"); 
	  for(inc = 0; inc < SERDES_0_CFG_NB_OF_PAIRS_2Gbps; ++inc){
	    *g_m2s_serdes_0_config_2Gbps[inc].p_reg = g_m2s_serdes_0_config_2Gbps[inc].value;
	  }
	}else if(ddl2_speed==1){
	  printf("DDL2-SerDes for 4.25Gbps\n"); 
	  for(inc = 0; inc < SERDES_0_CFG_NB_OF_PAIRS_4Gbps; ++inc){
	    *g_m2s_serdes_0_config_4Gbps[inc].p_reg = g_m2s_serdes_0_config_4Gbps[inc].value;
	  }
	}
#endif
	
	//	if(core_cfg_version >= CORE_CONFIGP_V7_0)
	//    {
        CORE_SF2_CFG->config_done = CONFIG_1_DONE;
	
        /* Poll for SDIF_RELEASED. */
        do
        {
            sdif_released = ((CORE_SF2_CFG->init_done) & SDIF_RELEASED_MASK);
        } while (0u == sdif_released);
	//    }
	/* CoreSF2Reset stuff */
	M2S_SYSREG->soft_reset_cr &= ~SYSREG_FPGA_SOFTRESET_MASK;
	CORE_SF2_CFG->config_done |= (CONFIG_1_DONE | CONFIG_2_DONE);


/* Wait for INIT_DONE from CoreSF2Reset. */
#if 0
	do
	  {
	    init_done = CORE_SF2_CFG->init_done & INIT_DONE_MASK;
	  } while (0u == init_done);
#endif
	return 0;
}

int checkboard(void)
{
  printf("Board: RCU-2.0 Rev %s\n",
	 CONFIG_SYS_BOARD_REV_STR);
  printf("SYSREG_ENVM_CR = 0x%x\n",  M2S_SYSREG->envm_cr);
  printf("SYSREG_MSSDDR_FACC1_CR = 0x%x\n",  M2S_SYSREG->mssddr_facc1_cr);
  printf("SYSREG_MSSDDR_FACC2_CR = 0x%x\n",  M2S_SYSREG->mssddr_facc2_cr);
  
  return 0;
}

/*
 * Initialize DRAM
 */
int dram_init (void)
{


#if ( CONFIG_NR_DRAM_BANKS > 0 )

  const char *s;
  int secded_config = 0;
  if((s = getenv("SECDED_CONFIG"))==NULL){
    printf("No SECDED_CONFIG(=Y,N) in the uBoot environments.. Set to No SECDED!\n"); 
    secded_config = 0;
  }else{
    if(strcmp(s,"Y") == 0){
      printf("Set to use SECDED!\n"); 
      secded_config = 1;
    }else if(strcmp(s,"N") == 0){
      printf("No use of SECDED!\n"); 
      secded_config = 0;
    }else{
      secded_config = 0;
    }
  }


  volatile struct ddr_regs	*ddr = (void *)0x40020000;
  u16				val;
  
  /*
   * Enable access to MDDR regs
   */
  M2S_SYSREG->mddr_cr = (1 << 0);
  
  /*
   * No non-bufferable regions
   */
  M2S_SYSREG->ddrb_nb_size_cr = 0;
  
  ddr->ddrc.DYN_POWERDOWN_CR = (0 << REG_DDRC_POWERDOWN_EN);
  
  /*
   * Apply DDR settings from Microsemi
   */
  ddr->ddrc.DYN_SOFT_RESET_CR =			0;
  ddr->ddrc.DYN_REFRESH_1_CR =			0x27de;
  ddr->ddrc.DYN_REFRESH_2_CR =			0x30f;
  /* ddr->ddrc.DYN_POWERDOWN_CR =			0x02; */
  ddr->ddrc.DYN_DEBUG_CR =			0x00;

  if(secded_config!=1){
    ddr->ddrc.MODE_CR =				0x101; /* this is without SECDED */
  }else{
    ddr->ddrc.MODE_CR =				0x115; /* this is with SECDED */
  }

  ddr->ddrc.ECC_DATA_MASK_CR =			0x0000;
  
  ddr->ddrc.ADDR_MAP_BANK_CR =			0x999; // this is for 256MB
  ddr->ddrc.ADDR_MAP_COL_1_CR =			0x3333;
  
  ///taken from exporting MDDR config.
  ddr->ddrc.ADDR_MAP_COL_2_CR =			0xffff; 
  ddr->ddrc.ADDR_MAP_ROW_1_CR =			0x8888; 
  //ddr->ddrc.ADDR_MAP_ROW_2_CR =			0x888;  ///this is for 512MB
  ddr->ddrc.ADDR_MAP_ROW_2_CR =			0x8ff;  ///this is for 256MB
  
  ddr->ddrc.INIT_1_CR =				0x0001;
  ddr->ddrc.CKE_RSTN_CYCLES_1_CR =		0x4242;
  ddr->ddrc.CKE_RSTN_CYCLES_2_CR =		0x8;
  ddr->ddrc.INIT_MR_CR =				0x520;
  ddr->ddrc.INIT_EMR_CR =				0x44;
  ddr->ddrc.INIT_EMR2_CR =			0x0000;
  ddr->ddrc.INIT_EMR3_CR =			0x0000;
  ddr->ddrc.DRAM_BANK_TIMING_PARAM_CR =		0xce0;
  ddr->ddrc.DRAM_RD_WR_LATENCY_CR =		0x86;
  ddr->ddrc.DRAM_RD_WR_PRE_CR =			0x235;
  ddr->ddrc.DRAM_MR_TIMING_PARAM_CR =		0x5c;
  ddr->ddrc.DRAM_RAS_TIMING_CR =			0x10f;
  ddr->ddrc.DRAM_RD_WR_TRNARND_TIME_CR =		0x178;
  ddr->ddrc.DRAM_T_PD_CR =			0x33;
  ddr->ddrc.DRAM_BANK_ACT_TIMING_CR =		0x1947;
  ddr->ddrc.ODT_PARAM_1_CR =			0x10;
  ddr->ddrc.ODT_PARAM_2_CR =			0x0000;
  ddr->ddrc.ADDR_MAP_COL_3_CR =			0x3300;
  /* ddr->ddrc.DEBUG_CR =				0x3300; */
  ddr->ddrc.MODE_REG_RD_WR_CR =			0x0000;
  ddr->ddrc.MODE_REG_DATA_CR =			0x0000;
  ddr->ddrc.PWR_SAVE_1_CR =			0x506;
  ddr->ddrc.PWR_SAVE_2_CR =			0x0000;
  ddr->ddrc.ZQ_LONG_TIME_CR =			0x200;
  ddr->ddrc.ZQ_SHORT_TIME_CR =			0x40;
  ddr->ddrc.ZQ_SHORT_INT_REFRESH_MARGIN_1_CR =	0x12;
  ddr->ddrc.ZQ_SHORT_INT_REFRESH_MARGIN_2_CR =	0x2;
  
  if(secded_config!=1){
    ddr->ddrc.PERF_PARAM_1_CR =			0x4000; /* this is without SECDED */
  }else{
    ddr->ddrc.PERF_PARAM_1_CR =			0x4002; /* this is with SECDED */
  }
  ddr->ddrc.HPR_QUEUE_PARAM_1_CR =		0x80f8;
  ddr->ddrc.HPR_QUEUE_PARAM_2_CR =		0x7;
  ddr->ddrc.LPR_QUEUE_PARAM_1_CR =		0x80f8;
  ddr->ddrc.LPR_QUEUE_PARAM_2_CR =		0x7;
  ddr->ddrc.WR_QUEUE_PARAM_CR =			0x200;
  ddr->ddrc.PERF_PARAM_2_CR =			0x400;
  ddr->ddrc.PERF_PARAM_3_CR =			0x0000;
  ddr->ddrc.DFI_RDDATA_EN_CR =			0x5;
  ddr->ddrc.DFI_MIN_CTRLUPD_TIMING_CR =		0x0003;
  ddr->ddrc.DFI_MAX_CTRLUPD_TIMING_CR =		0x0040;
  ddr->ddrc.DFI_WR_LVL_CONTROL_1_CR =		0x0000;
  ddr->ddrc.DFI_WR_LVL_CONTROL_2_CR =		0x0000;
  ddr->ddrc.DFI_RD_LVL_CONTROL_1_CR =		0x0000;
  ddr->ddrc.DFI_RD_LVL_CONTROL_2_CR =		0x0000;
  ddr->ddrc.DFI_CTRLUPD_TIME_INTERVAL_CR =	0x309;
  /* ddr->ddrc.DYN_SOFT_RESET_CR2 =		0x4; */
  ddr->ddrc.AXI_FABRIC_PRI_ID_CR =		0x0000;
  ddr->ddrc.ECC_INT_CLR_REG =			0x0000;
  
  ddr->phy.DYN_BIST_TEST_CR =			0x0;
  ddr->phy.DYN_BIST_TEST_ERRCLR_1_CR =		0x0;
  ddr->phy.DYN_BIST_TEST_ERRCLR_2_CR =		0x0;
  ddr->phy.DYN_BIST_TEST_ERRCLR_3_CR =		0x0;
  ddr->phy.BIST_TEST_SHIFT_PATTERN_1_CR =		0x0;
  ddr->phy.BIST_TEST_SHIFT_PATTERN_2_CR =		0x0;
  ddr->phy.BIST_TEST_SHIFT_PATTERN_3_CR =		0x0;
  ddr->phy.DYN_LOOPBACK_TEST_CR =			0x0000;
  ddr->phy.BOARD_LOOPBACK_CR =			0x0;
  ddr->phy.CTRL_SLAVE_RATIO_CR =			0x80;
  ddr->phy.CTRL_SLAVE_FORCE_CR =			0x0;
  ddr->phy.CTRL_SLAVE_DELAY_CR =			0x0;
  if(secded_config!=1){
    ddr->phy.DATA_SLICE_IN_USE_CR =			0xf;  /* this is without SECDED */
  }else{
    ddr->phy.DATA_SLICE_IN_USE_CR =			0x13;  /* this is with SECDED */
  }
  ddr->phy.LVL_NUM_OF_DQ0_CR =			0x0;
  ddr->phy.DQ_OFFSET_1_CR =			0x0;
  ddr->phy.DQ_OFFSET_2_CR =			0x0;
  ddr->phy.DQ_OFFSET_3_CR =			0x0;
  ddr->phy.DIS_CALIB_RST_CR =			0x0;
  ddr->phy.DLL_LOCK_DIFF_CR =			0xb;
  ddr->phy.FIFO_WE_IN_DELAY_1_CR =		0x0;
  ddr->phy.FIFO_WE_IN_DELAY_2_CR =		0x0;
  ddr->phy.FIFO_WE_IN_DELAY_3_CR =		0x0;
  ddr->phy.FIFO_WE_IN_FORCE_CR =			0x0;
  ddr->phy.FIFO_WE_SLAVE_RATIO_1_CR =		0x80;
  ddr->phy.FIFO_WE_SLAVE_RATIO_2_CR =		0x2004;
  ddr->phy.FIFO_WE_SLAVE_RATIO_3_CR =		0x100;
  ddr->phy.FIFO_WE_SLAVE_RATIO_4_CR =		0x8;
  ddr->phy.GATELVL_INIT_MODE_CR =			0x0;
  ddr->phy.GATELVL_INIT_RATIO_1_CR =		0x0;
  ddr->phy.GATELVL_INIT_RATIO_2_CR =		0x0;
  ddr->phy.GATELVL_INIT_RATIO_3_CR =		0x0;
  ddr->phy.GATELVL_INIT_RATIO_4_CR =		0x0;
  ddr->phy.LOCAL_ODT_CR =				0x1;
  ddr->phy.INVERT_CLKOUT_CR =			0x0;
  ddr->phy.RD_DQS_SLAVE_DELAY_1_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_DELAY_2_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_DELAY_3_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_FORCE_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_RATIO_1_CR =		0x4050;
  ddr->phy.RD_DQS_SLAVE_RATIO_2_CR =		0x501;
  ddr->phy.RD_DQS_SLAVE_RATIO_3_CR =		0x5014;
  ddr->phy.RD_DQS_SLAVE_RATIO_4_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_DELAY_1_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_DELAY_2_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_DELAY_3_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_FORCE_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_1_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_2_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_3_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_4_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_DELAY_1_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_DELAY_2_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_DELAY_3_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_FORCE_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_RATIO_1_CR =		0x50;
  ddr->phy.WR_DATA_SLAVE_RATIO_2_CR =		0x501;
  ddr->phy.WR_DATA_SLAVE_RATIO_3_CR =		0x5010;
  ddr->phy.WR_DATA_SLAVE_RATIO_4_CR =		0x0;
  ddr->phy.WRLVL_INIT_MODE_CR =			0x0;
  ddr->phy.WRLVL_INIT_RATIO_1_CR =		0x0;
  ddr->phy.WRLVL_INIT_RATIO_2_CR =		0x0;
  ddr->phy.WRLVL_INIT_RATIO_3_CR =		0x0;
  ddr->phy.WRLVL_INIT_RATIO_4_CR =		0x0;
  ddr->phy.WR_RD_RL_CR =				0x43;
  ddr->phy.RDC_FIFO_RST_ERRCNTCLR_CR =		0x0;
  ddr->phy.RDC_WE_TO_RE_DELAY_CR =		0x3;
  ddr->phy.USE_FIXED_RE_CR =			0x1;
  ddr->phy.USE_RANK0_DELAYS_CR =			0x1;
  ddr->phy.USE_LVL_TRNG_LEVEL_CR =		0x0;
  ddr->phy.DYN_CONFIG_CR =			0x0000;
  ddr->phy.RD_WR_GATE_LVL_CR =			0x0;
  
  /* ddr->fic.NB_ADDR_CR =			0x0; */
  /* ddr->fic.NBRWB_SIZE_CR =			0x0; */
  /* ddr->fic.WB_TIMEOUT_CR =			0x0; */
  /* ddr->fic.HPD_SW_RW_EN_CR =			0x0; */
  /* ddr->fic.HPD_SW_RW_INVAL_CR =		0x0; */
  /* ddr->fic.SW_WR_ERCLR_CR =			0x0; */
  /* ddr->fic.ERR_INT_ENABLE_CR =			0x0; */
  /* ddr->fic.NUM_AHB_MASTERS_CR =		0x0; */
  /* ddr->fic.LOCK_TIMEOUTVAL_1_CR =		0x0; */
  /* ddr->fic.LOCK_TIMEOUTVAL_2_CR =		0x0; */
  /* ddr->fic.LOCK_TIMEOUT_EN_CR =		0x0; */

  ddr->phy.DYN_RESET_CR =				0x1;
  ddr->ddrc.DYN_SOFT_RESET_CR =			0x0001;
  
  /*
   * Set up U-Boot global data
   */
  gd->bd->bi_dram[0].start = CONFIG_SYS_RAM_BASE;
  gd->bd->bi_dram[0].size = CONFIG_SYS_RAM_SIZE;
  
  ///// this is for 512MB setting
  //gd->bd->bi_dram[1].start = CONFIG_SYS_RAM_BASE_2;
  //gd->bd->bi_dram[1].size = CONFIG_SYS_RAM_SIZE;
  
  //ddl2_serdes_init();
#endif
  return 0;
}


/*
 * Initialize DRAM
 */
int dram_init_no_secded (void)
{
#if ( CONFIG_NR_DRAM_BANKS > 0 )
	volatile struct ddr_regs	*ddr = (void *)0x40020000;
	u16				val;

	/*
	 * Enable access to MDDR regs
	 */
	M2S_SYSREG->mddr_cr = (1 << 0);

	/*
	 * No non-bufferable regions
	 */
	M2S_SYSREG->ddrb_nb_size_cr = 0;

	ddr->ddrc.DYN_POWERDOWN_CR = (0 << REG_DDRC_POWERDOWN_EN);

	/*
	 * Apply DDR settings from Microsemi
	 */
	ddr->ddrc.DYN_SOFT_RESET_CR =			0;
	ddr->ddrc.DYN_REFRESH_1_CR =			0x27de;
	ddr->ddrc.DYN_REFRESH_2_CR =			0x30f;
	/* ddr->ddrc.DYN_POWERDOWN_CR =			0x02; */
	ddr->ddrc.DYN_DEBUG_CR =			0x00;
	ddr->ddrc.MODE_CR =				0x101; /* this is without SECDED */
	//ddr->ddrc.MODE_CR =				0x115; /* this is with SECDED */
	ddr->ddrc.ADDR_MAP_BANK_CR =			0x999;
	ddr->ddrc.ECC_DATA_MASK_CR =			0x0000;
	ddr->ddrc.ADDR_MAP_COL_1_CR =			0x3333;
	ddr->ddrc.ADDR_MAP_COL_2_CR =			0xffff;
	ddr->ddrc.ADDR_MAP_ROW_1_CR =			0x8888;
	ddr->ddrc.ADDR_MAP_ROW_2_CR =			0x8ff;
	ddr->ddrc.INIT_1_CR =				0x0001;
	ddr->ddrc.CKE_RSTN_CYCLES_1_CR =		0x4242;
	ddr->ddrc.CKE_RSTN_CYCLES_2_CR =		0x8;
	ddr->ddrc.INIT_MR_CR =				0x520;
	ddr->ddrc.INIT_EMR_CR =				0x44;
	ddr->ddrc.INIT_EMR2_CR =			0x0000;
	ddr->ddrc.INIT_EMR3_CR =			0x0000;
	ddr->ddrc.DRAM_BANK_TIMING_PARAM_CR =		0xce0;
	ddr->ddrc.DRAM_RD_WR_LATENCY_CR =		0x86;
	ddr->ddrc.DRAM_RD_WR_PRE_CR =			0x235;
	ddr->ddrc.DRAM_MR_TIMING_PARAM_CR =		0x5c;
	ddr->ddrc.DRAM_RAS_TIMING_CR =			0x10f;
	ddr->ddrc.DRAM_RD_WR_TRNARND_TIME_CR =		0x178;
	ddr->ddrc.DRAM_T_PD_CR =			0x33;
	ddr->ddrc.DRAM_BANK_ACT_TIMING_CR =		0x1947;
	ddr->ddrc.ODT_PARAM_1_CR =			0x10;
	ddr->ddrc.ODT_PARAM_2_CR =			0x0000;
	ddr->ddrc.ADDR_MAP_COL_3_CR =			0x3300;
	/* ddr->ddrc.DEBUG_CR =				0x3300; */
	ddr->ddrc.MODE_REG_RD_WR_CR =			0x0000;
	ddr->ddrc.MODE_REG_DATA_CR =			0x0000;
	ddr->ddrc.PWR_SAVE_1_CR =			0x506;
	ddr->ddrc.PWR_SAVE_2_CR =			0x0000;
	ddr->ddrc.ZQ_LONG_TIME_CR =			0x200;
	ddr->ddrc.ZQ_SHORT_TIME_CR =			0x40;
	ddr->ddrc.ZQ_SHORT_INT_REFRESH_MARGIN_1_CR =	0x12;
	ddr->ddrc.ZQ_SHORT_INT_REFRESH_MARGIN_2_CR =	0x2;
	ddr->ddrc.PERF_PARAM_1_CR =			0x4000; /* this is without SECDED */
	//ddr->ddrc.PERF_PARAM_1_CR =			0x4002; /* this is with SECDED */
	ddr->ddrc.HPR_QUEUE_PARAM_1_CR =		0x80f8;
	ddr->ddrc.HPR_QUEUE_PARAM_2_CR =		0x7;
	ddr->ddrc.LPR_QUEUE_PARAM_1_CR =		0x80f8;
	ddr->ddrc.LPR_QUEUE_PARAM_2_CR =		0x7;
	ddr->ddrc.WR_QUEUE_PARAM_CR =			0x200;
	ddr->ddrc.PERF_PARAM_2_CR =			0x400;
	ddr->ddrc.PERF_PARAM_3_CR =			0x0000;
	ddr->ddrc.DFI_RDDATA_EN_CR =			0x5;
	ddr->ddrc.DFI_MIN_CTRLUPD_TIMING_CR =		0x0003;
	ddr->ddrc.DFI_MAX_CTRLUPD_TIMING_CR =		0x0040;
	ddr->ddrc.DFI_WR_LVL_CONTROL_1_CR =		0x0000;
	ddr->ddrc.DFI_WR_LVL_CONTROL_2_CR =		0x0000;
	ddr->ddrc.DFI_RD_LVL_CONTROL_1_CR =		0x0000;
	ddr->ddrc.DFI_RD_LVL_CONTROL_2_CR =		0x0000;
	ddr->ddrc.DFI_CTRLUPD_TIME_INTERVAL_CR =	0x309;
	/* ddr->ddrc.DYN_SOFT_RESET_CR2 =		0x4; */
	ddr->ddrc.AXI_FABRIC_PRI_ID_CR =		0x0000;
	ddr->ddrc.ECC_INT_CLR_REG =			0x0000;

	ddr->phy.DYN_BIST_TEST_CR =			0x0;
	ddr->phy.DYN_BIST_TEST_ERRCLR_1_CR =		0x0;
	ddr->phy.DYN_BIST_TEST_ERRCLR_2_CR =		0x0;
	ddr->phy.DYN_BIST_TEST_ERRCLR_3_CR =		0x0;
	ddr->phy.BIST_TEST_SHIFT_PATTERN_1_CR =		0x0;
	ddr->phy.BIST_TEST_SHIFT_PATTERN_2_CR =		0x0;
	ddr->phy.BIST_TEST_SHIFT_PATTERN_3_CR =		0x0;
	ddr->phy.DYN_LOOPBACK_TEST_CR =			0x0000;
	ddr->phy.BOARD_LOOPBACK_CR =			0x0;
	ddr->phy.CTRL_SLAVE_RATIO_CR =			0x80;
	ddr->phy.CTRL_SLAVE_FORCE_CR =			0x0;
	ddr->phy.CTRL_SLAVE_DELAY_CR =			0x0;
	ddr->phy.DATA_SLICE_IN_USE_CR =			0xf;  /* this is without SECDED */
        //ddr->phy.DATA_SLICE_IN_USE_CR =			0x13;  /* this is with SECDED */
	ddr->phy.LVL_NUM_OF_DQ0_CR =			0x0;
	ddr->phy.DQ_OFFSET_1_CR =			0x0;
	ddr->phy.DQ_OFFSET_2_CR =			0x0;
	ddr->phy.DQ_OFFSET_3_CR =			0x0;
	ddr->phy.DIS_CALIB_RST_CR =			0x0;
	ddr->phy.DLL_LOCK_DIFF_CR =			0xb;
	ddr->phy.FIFO_WE_IN_DELAY_1_CR =		0x0;
	ddr->phy.FIFO_WE_IN_DELAY_2_CR =		0x0;
	ddr->phy.FIFO_WE_IN_DELAY_3_CR =		0x0;
	ddr->phy.FIFO_WE_IN_FORCE_CR =			0x0;
	ddr->phy.FIFO_WE_SLAVE_RATIO_1_CR =		0x80;
	ddr->phy.FIFO_WE_SLAVE_RATIO_2_CR =		0x2004;
	ddr->phy.FIFO_WE_SLAVE_RATIO_3_CR =		0x100;
	ddr->phy.FIFO_WE_SLAVE_RATIO_4_CR =		0x8;
	ddr->phy.GATELVL_INIT_MODE_CR =			0x0;
	ddr->phy.GATELVL_INIT_RATIO_1_CR =		0x0;
	ddr->phy.GATELVL_INIT_RATIO_2_CR =		0x0;
	ddr->phy.GATELVL_INIT_RATIO_3_CR =		0x0;
	ddr->phy.GATELVL_INIT_RATIO_4_CR =		0x0;
	ddr->phy.LOCAL_ODT_CR =				0x1;
	ddr->phy.INVERT_CLKOUT_CR =			0x0;
	ddr->phy.RD_DQS_SLAVE_DELAY_1_CR =		0x0;
	ddr->phy.RD_DQS_SLAVE_DELAY_2_CR =		0x0;
	ddr->phy.RD_DQS_SLAVE_DELAY_3_CR =		0x0;
	ddr->phy.RD_DQS_SLAVE_FORCE_CR =		0x0;
	ddr->phy.RD_DQS_SLAVE_RATIO_1_CR =		0x4050;
	ddr->phy.RD_DQS_SLAVE_RATIO_2_CR =		0x501;
	ddr->phy.RD_DQS_SLAVE_RATIO_3_CR =		0x5014;
	ddr->phy.RD_DQS_SLAVE_RATIO_4_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_DELAY_1_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_DELAY_2_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_DELAY_3_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_FORCE_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_RATIO_1_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_RATIO_2_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_RATIO_3_CR =		0x0;
	ddr->phy.WR_DQS_SLAVE_RATIO_4_CR =		0x0;
	ddr->phy.WR_DATA_SLAVE_DELAY_1_CR =		0x0;
	ddr->phy.WR_DATA_SLAVE_DELAY_2_CR =		0x0;
	ddr->phy.WR_DATA_SLAVE_DELAY_3_CR =		0x0;
	ddr->phy.WR_DATA_SLAVE_FORCE_CR =		0x0;
	ddr->phy.WR_DATA_SLAVE_RATIO_1_CR =		0x50;
	ddr->phy.WR_DATA_SLAVE_RATIO_2_CR =		0x501;
	ddr->phy.WR_DATA_SLAVE_RATIO_3_CR =		0x5010;
	ddr->phy.WR_DATA_SLAVE_RATIO_4_CR =		0x0;
	ddr->phy.WRLVL_INIT_MODE_CR =			0x0;
	ddr->phy.WRLVL_INIT_RATIO_1_CR =		0x0;
	ddr->phy.WRLVL_INIT_RATIO_2_CR =		0x0;
	ddr->phy.WRLVL_INIT_RATIO_3_CR =		0x0;
	ddr->phy.WRLVL_INIT_RATIO_4_CR =		0x0;
	ddr->phy.WR_RD_RL_CR =				0x43;
	ddr->phy.RDC_FIFO_RST_ERRCNTCLR_CR =		0x0;
	ddr->phy.RDC_WE_TO_RE_DELAY_CR =		0x3;
	ddr->phy.USE_FIXED_RE_CR =			0x1;
	ddr->phy.USE_RANK0_DELAYS_CR =			0x1;
	ddr->phy.USE_LVL_TRNG_LEVEL_CR =		0x0;
	ddr->phy.DYN_CONFIG_CR =			0x0000;
	ddr->phy.RD_WR_GATE_LVL_CR =			0x0;

	/* ddr->fic.NB_ADDR_CR =			0x0; */
	/* ddr->fic.NBRWB_SIZE_CR =			0x0; */
	/* ddr->fic.WB_TIMEOUT_CR =			0x0; */
	/* ddr->fic.HPD_SW_RW_EN_CR =			0x0; */
	/* ddr->fic.HPD_SW_RW_INVAL_CR =		0x0; */
	/* ddr->fic.SW_WR_ERCLR_CR =			0x0; */
	/* ddr->fic.ERR_INT_ENABLE_CR =			0x0; */
	/* ddr->fic.NUM_AHB_MASTERS_CR =		0x0; */
	/* ddr->fic.LOCK_TIMEOUTVAL_1_CR =		0x0; */
	/* ddr->fic.LOCK_TIMEOUTVAL_2_CR =		0x0; */
	/* ddr->fic.LOCK_TIMEOUT_EN_CR =		0x0; */

	ddr->phy.DYN_RESET_CR =				0x1;
	ddr->ddrc.DYN_SOFT_RESET_CR =			0x0001;

	/*
	 * Set up U-Boot global data
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_RAM_BASE;
	gd->bd->bi_dram[0].size = CONFIG_SYS_RAM_SIZE;

	//gd->bd->bi_dram[1].start = CONFIG_SYS_RAM_BASE_2;
	//gd->bd->bi_dram[1].size = CONFIG_SYS_RAM_SIZE;

	//ddl2_serdes_init();
#endif
	return 0;
}

int misc_init_r(void)
{
	/* Try to force SPI */
/*	MSS_SPI_init(&g_mss_spi0);
	MSS_SPI_configure_master_mode(&g_mss_spi0,
                                  MSS_SPI_SLAVE_0,
                                  MSS_SPI_MODE0,
                                  MSS_SPI_PCLK_DIV_256,
                                  frame_size);

    This did nothing
  */        
	return 0;
}

#ifdef CONFIG_M2S_ETH
int board_eth_init(bd_t *bis)
{
//	*(unsigned int *)(SERDES1_CFG_BASE + SYSTEM_CONFIG_PHY_MODE_1) = 0x80F;/*Lane 3*/
//	CORE_SF2_CFG->config_done = 1u;
  //return 0;
  /*
  printf(" : ethernet init \n");
  int inc;
  for(inc = 0; inc < SERDES_0_CFG_NB_OF_PAIRS; ++inc){
    printf("%p -- 0x%x\n", g_m2s_serdes_0_config[inc].p_reg, *g_m2s_serdes_0_config[inc].p_reg);
    }*/


  return m2s_eth_driver_init(bis);
  //return 0;

  //return rcu2_eth_driver_init(bis); USE THIS
}
#endif


int ddl2_serdes_init(void)
{

  printf("DDL2_serdes_init\n");
  /* Values extracted from SoftConsole files generated by Libero */
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + SYSTEM_CONFIG_PHY_MODE_1) = 0x10F;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_PHY_RESET_OVERRIDE) = 0x30;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_CR0) = 0x80;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_ERRCNT_DEC) = 0x20;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_RXIDLE_MAX_ERRCNT_THR) = 0xF8;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_IMPED_RATIO) = 0x80;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_PLL_M_N) = 0x13;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_CNT250NS_MAX) = 0x1B;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_TX_AMP_RATIO) = 0x80;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_ENDCALIB_MAX) = 0x10;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_CALIB_STABILITY_COUNT) = 0x38;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_RX_OFFSET_COUNT) = 0x70;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_GEN1_TX_PLL_CCP) = 0x2;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_GEN1_RX_PLL_CCP) = 0x2;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_PHY_RESET_OVERRIDE) = 0x0;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + LANE0_UPDATE_SETTINGS) = 0x1;
	*(volatile uint32_t *)(SERDES0_LANE0_REGS + SYSTEM_CONFIG_PHY_MODE_1) = 0xF0F;
	/* Signal to CoreSF2Reset that peripheral configuration registers have been written.*/
	CORE_SF2_CFG->config_done = 1u;
#if 0 
/* Lars -- look into this... taken from emcraft board file */
/* FIXME: init_done is never signalled after a soft reset if the DDR has been initialized before the reset. */
	while(!CORE_SF2_CFG->init_done)
	{
		;   /* Wait for INIT_DONE from CoreSF2Reset. */
	}
#endif
	
	return 0;
}



void re_init_mac_addr(void){

  uint32_t inc = 0;


  ///////////////////
  puts("adjust new IP addr and mac address based on Serial ID number (default_uboot_env is loaded).\n");
  /// update mac address and ip address based on serial ID number of SF2
  printf("get serial number = \n");  
  asm volatile ("cpsie i");
  
  u8 *p_serial_number;
  p_serial_number = (u8*) malloc(16);
    exec_comblk_get_serial_number_out(p_serial_number);
    printf("Device serial number = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
	   p_serial_number[0],p_serial_number[1],p_serial_number[2],p_serial_number[3],
	   p_serial_number[4],p_serial_number[5],p_serial_number[6],p_serial_number[7],
	   p_serial_number[8],p_serial_number[9],p_serial_number[10],p_serial_number[11],
	   p_serial_number[12],p_serial_number[13],p_serial_number[14],p_serial_number[15]);
    
    asm volatile ("cpsid i");
    
    u32 serial_decode[4];
    serial_decode[0] = ( (p_serial_number[0] << 24) + (p_serial_number[1] << 16) +
			 (p_serial_number[2] << 8) + p_serial_number[3] );
    serial_decode[1] = ( (p_serial_number[4] << 24) + (p_serial_number[5] << 16) +
			 (p_serial_number[6] << 8) + p_serial_number[7] );
    serial_decode[2] = ( (p_serial_number[8] << 24) + (p_serial_number[9] << 16) +
			 (p_serial_number[10] << 8) + p_serial_number[11] );
    serial_decode[3] = ( (p_serial_number[12] << 24) + (p_serial_number[13] << 16) +
			 (p_serial_number[14] << 8) + p_serial_number[15] );
    
    
    printf("update ethernet and ipaddr\n");
    for(inc=0;inc<NUMBER_OF_RCU2_BOARD;inc++){
      if(g_m2s_serial_mac_config[inc].serial[0]==serial_decode[0] &&
	 g_m2s_serial_mac_config[inc].serial[1]==serial_decode[1] &&
	 g_m2s_serial_mac_config[inc].serial[2]==serial_decode[2] &&
	 g_m2s_serial_mac_config[inc].serial[3]==serial_decode[3]){
	
	printf(" found in the list : serial = %x%x%x%x, %s, %s \n",
	       g_m2s_serial_mac_config[inc].serial[0],
	       g_m2s_serial_mac_config[inc].serial[1],
	       g_m2s_serial_mac_config[inc].serial[2],
	       g_m2s_serial_mac_config[inc].serial[3],
	       g_m2s_serial_mac_config[inc].macaddr,
	       g_m2s_serial_mac_config[inc].ipaddr);
	
	printf("ethaddr, ipaddr, and serverip are set\n");
        setenv("ethaddr",g_m2s_serial_mac_config[inc].macaddr);
        setenv("ipaddr",g_m2s_serial_mac_config[inc].ipaddr);


	if(g_m2s_serial_mac_config[inc].ipaddr[7]=='1' &&
	   g_m2s_serial_mac_config[inc].ipaddr[8]=='2' &&
	   g_m2s_serial_mac_config[inc].ipaddr[9]=='9'){
	  setenv("serverip",MK_STR(CONFIG_SERVERIP1));
	}else if(g_m2s_serial_mac_config[inc].ipaddr[7]=='1' &&
		 g_m2s_serial_mac_config[inc].ipaddr[8]=='4' &&
		 g_m2s_serial_mac_config[inc].ipaddr[9]=='2'){
	  setenv("serverip",MK_STR(CONFIG_SERVERIP2));
	}


      }
    }
}


int board_late_init(void)
{

  M2S_SYSREG->mddr_cr = 0x0;

  /* 
  ** DDL2 SERDES stuff from SystemInit() in SoftConsole generated code
  */	
  uint32_t sdif_released; /* if MSS_SYS_SERDES_CONFIG_BY_CORTEX */
  uint32_t init_done; /* if MSS_SYS_CORESF2RESET_USED */
  //uint32_t core_cfg_version; /* if MSS_SYS_SERDES_CONFIG_BY_CORTEX */
  /*core_cfg_version = CORE_SF2_CFG->IP_VERSION_SR;*/
  
  /*configure_serdes_intf()*/
  uint32_t inc = 0;
  ////////
  const char *s;
  ddl2_speed = 0;
  if((s = getenv("DDL2_CONFIG_SPEED"))==NULL){
    printf("No DDL2_CONFIG_SPEED(=2,4) in the uBoot environments.. Set to 2.125Gbps\n"); 
    ddl2_speed = 0;
  }else{
    if(strcmp(s,"2") == 0){
      printf("DDL2_CONFIG_SPEED=2\n"); 
      ddl2_speed = 0;
    }else if(strcmp(s,"4") == 0){
      printf("DDL2_CONFIG_SPEED=4\n"); 
      ddl2_speed = 1;
    }else{
      ddl2_speed = 0;
    }
  }
  
  if(ddl2_speed==0){
    printf("DDL2-SerDes for 2.125Gbps\n"); 
    for(inc = 0; inc < SERDES_0_CFG_NB_OF_PAIRS_2Gbps; ++inc){
      *g_m2s_serdes_0_config_2Gbps[inc].p_reg = g_m2s_serdes_0_config_2Gbps[inc].value;
    }
  }else if(ddl2_speed==1){
    printf("DDL2-SerDes for 4.25Gbps\n"); 
    for(inc = 0; inc < SERDES_0_CFG_NB_OF_PAIRS_4Gbps; ++inc){
      *g_m2s_serdes_0_config_4Gbps[inc].p_reg = g_m2s_serdes_0_config_4Gbps[inc].value;
    }
  }
  
  //	if(core_cfg_version >= CORE_CONFIGP_V7_0)
  //    {
  CORE_SF2_CFG->config_done = CONFIG_1_DONE;
  
  /* Poll for SDIF_RELEASED. */
  do
    {
      sdif_released = ((CORE_SF2_CFG->init_done) & SDIF_RELEASED_MASK);
    } while (0u == sdif_released);
  //    }
  /* CoreSF2Reset stuff */
  M2S_SYSREG->soft_reset_cr &= ~SYSREG_FPGA_SOFTRESET_MASK;
  CORE_SF2_CFG->config_done |= (CONFIG_1_DONE | CONFIG_2_DONE);
  
  printf("dump the DDL2-SERDES registers\n");
  
  if(ddl2_speed==0){
    for(inc = 0; inc < SERDES_0_CFG_NB_OF_PAIRS_2Gbps; ++inc){
      printf("%p -- 0x%x\n", g_m2s_serdes_0_config_2Gbps[inc].p_reg, 
	     *g_m2s_serdes_0_config_2Gbps[inc].p_reg);
    }
  }else if(ddl2_speed==1){
    for(inc = 0; inc < SERDES_0_CFG_NB_OF_PAIRS_4Gbps; ++inc){
      printf("%p -- 0x%x\n", g_m2s_serdes_0_config_4Gbps[inc].p_reg, 
	     *g_m2s_serdes_0_config_4Gbps[inc].p_reg);
    }
  }


  
  printf("check uboot from flash (0 or 1) or default (2) = %d \n", uboot_pars_from_flash); 
  if(uboot_pars_from_flash==2){
    re_init_mac_addr();
  }
  ///
  //////////////////

  //#ifdef CONFIG_USE_SPI_FLASH_2
  //  MSMVERSION->uboot_version = (0x17081D00 + uboot_pars_from_flash);
  //#else
  //  MSMVERSION->uboot_version = (0x17080D00 + uboot_pars_from_flash);
  //#endif
  
}
