/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 * GDB register definitions
 *
 ****/

#pragma once

#include "gdbstub-cfg.h"
#include "signals.h"

#if GDBSTUB_GDB_PATCHED == 1
/*
 * Patched GDB uses a small subset of registers
 */
#define GDB_REGCOUNT 22
#define XT_REGISTER_MAP(XTREG)                                                                                         \
	XX(0, a0)                                                                                                          \
	XX(1, a1)                                                                                                          \
	XX(2, a2)                                                                                                          \
	XX(3, a3)                                                                                                          \
	XX(4, a4)                                                                                                          \
	XX(5, a5)                                                                                                          \
	XX(6, a6)                                                                                                          \
	XX(7, a7)                                                                                                          \
	XX(8, a8)                                                                                                          \
	XX(9, a9)                                                                                                          \
	XX(10, a10)                                                                                                        \
	XX(11, a11)                                                                                                        \
	XX(12, a12)                                                                                                        \
	XX(13, a13)                                                                                                        \
	XX(14, a14)                                                                                                        \
	XX(15, a15)                                                                                                        \
	XX(16, pc)                                                                                                         \
	XX(17, sar)                                                                                                        \
	XX(18, litbase)                                                                                                    \
	XX(19, sr176)                                                                                                      \
	XX(20, sr208)                                                                                                      \
	XX(21, ps)

#else
/*
 * Unpatched GDB full register set
 * Note that our stub doesn't return all of these, we send 'xxxxxxxx' for those
 */
#define GDB_REGCOUNT 113
// This table is extracted from xtensa-config.c in GDB source code
#define XT_REGISTER_MAP(XX)                                                                                            \
	XX(0, pc)                                                                                                          \
	XX(1, ar0)                                                                                                         \
	XX(2, ar1)                                                                                                         \
	XX(3, ar2)                                                                                                         \
	XX(4, ar3)                                                                                                         \
	XX(5, ar4)                                                                                                         \
	XX(6, ar5)                                                                                                         \
	XX(7, ar6)                                                                                                         \
	XX(8, ar7)                                                                                                         \
	XX(9, ar8)                                                                                                         \
	XX(10, ar9)                                                                                                        \
	XX(11, ar10)                                                                                                       \
	XX(12, ar11)                                                                                                       \
	XX(13, ar12)                                                                                                       \
	XX(14, ar13)                                                                                                       \
	XX(15, ar14)                                                                                                       \
	XX(16, ar15)                                                                                                       \
	XX(17, ar16)                                                                                                       \
	XX(18, ar17)                                                                                                       \
	XX(19, ar18)                                                                                                       \
	XX(20, ar19)                                                                                                       \
	XX(21, ar20)                                                                                                       \
	XX(22, ar21)                                                                                                       \
	XX(23, ar22)                                                                                                       \
	XX(24, ar23)                                                                                                       \
	XX(25, ar24)                                                                                                       \
	XX(26, ar25)                                                                                                       \
	XX(27, ar26)                                                                                                       \
	XX(28, ar27)                                                                                                       \
	XX(29, ar28)                                                                                                       \
	XX(30, ar29)                                                                                                       \
	XX(31, ar30)                                                                                                       \
	XX(32, ar31)                                                                                                       \
	XX(33, lbeg)                                                                                                       \
	XX(34, lend)                                                                                                       \
	XX(35, lcount)                                                                                                     \
	XX(36, sar)                                                                                                        \
	XX(37, litbase)                                                                                                    \
	XX(38, windowbase)                                                                                                 \
	XX(39, windowstart)                                                                                                \
	XX(40, sr176)                                                                                                      \
	XX(41, sr208)                                                                                                      \
	XX(42, ps)                                                                                                         \
	XX(43, threadptr)                                                                                                  \
	XX(44, scompare1)                                                                                                  \
	XX(45, ptevaddr)                                                                                                   \
	XX(46, mmid)                                                                                                       \
	XX(47, rasid)                                                                                                      \
	XX(48, itlbcfg)                                                                                                    \
	XX(49, dtlbcfg)                                                                                                    \
	XX(50, ibreakenable)                                                                                               \
	XX(51, ddr)                                                                                                        \
	XX(52, ibreaka0)                                                                                                   \
	XX(53, ibreaka1)                                                                                                   \
	XX(54, dbreaka0)                                                                                                   \
	XX(55, dbreaka1)                                                                                                   \
	XX(56, dbreakc0)                                                                                                   \
	XX(57, dbreakc1)                                                                                                   \
	XX(58, epc1)                                                                                                       \
	XX(59, epc2)                                                                                                       \
	XX(60, epc3)                                                                                                       \
	XX(61, epc4)                                                                                                       \
	XX(62, epc5)                                                                                                       \
	XX(63, epc6)                                                                                                       \
	XX(64, epc7)                                                                                                       \
	XX(65, depc)                                                                                                       \
	XX(66, eps2)                                                                                                       \
	XX(67, eps3)                                                                                                       \
	XX(68, eps4)                                                                                                       \
	XX(69, eps5)                                                                                                       \
	XX(70, eps6)                                                                                                       \
	XX(71, eps7)                                                                                                       \
	XX(72, excsave1)                                                                                                   \
	XX(73, excsave2)                                                                                                   \
	XX(74, excsave3)                                                                                                   \
	XX(75, excsave4)                                                                                                   \
	XX(76, excsave5)                                                                                                   \
	XX(77, excsave6)                                                                                                   \
	XX(78, excsave7)                                                                                                   \
	XX(79, cpenable)                                                                                                   \
	XX(80, interrupt)                                                                                                  \
	XX(81, intset)                                                                                                     \
	XX(82, intclear)                                                                                                   \
	XX(83, intenable)                                                                                                  \
	XX(84, vecbase)                                                                                                    \
	XX(85, exccause)                                                                                                   \
	XX(86, debugcause)                                                                                                 \
	XX(87, ccount)                                                                                                     \
	XX(88, prid)                                                                                                       \
	XX(89, icount)                                                                                                     \
	XX(90, icountlevel)                                                                                                \
	XX(91, excvaddr)                                                                                                   \
	XX(92, ccompare0)                                                                                                  \
	XX(93, ccompare1)                                                                                                  \
	XX(94, ccompare2)                                                                                                  \
	XX(95, misc0)                                                                                                      \
	XX(96, misc1)                                                                                                      \
	XX(97, a0)                                                                                                         \
	XX(98, a1)                                                                                                         \
	XX(99, a2)                                                                                                         \
	XX(100, a3)                                                                                                        \
	XX(101, a4)                                                                                                        \
	XX(102, a5)                                                                                                        \
	XX(103, a6)                                                                                                        \
	XX(104, a7)                                                                                                        \
	XX(105, a8)                                                                                                        \
	XX(106, a9)                                                                                                        \
	XX(107, a10)                                                                                                       \
	XX(108, a11)                                                                                                       \
	XX(109, a12)                                                                                                       \
	XX(110, a13)                                                                                                       \
	XX(111, a14)                                                                                                       \
	XX(112, a15)

#endif

// Register numbers
enum GdbReg {
#define XX(idx, name) GdbReg_##name = idx,
	XT_REGISTER_MAP(XX)
#undef XX
};
