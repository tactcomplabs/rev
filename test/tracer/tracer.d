
tracer.exe:     file format elf64-littleriscv

SYMBOL TABLE:
00000000000100e8 l    d  .text	0000000000000000 .text
00000000000115e0 l    d  .eh_frame	0000000000000000 .eh_frame
0000000000011668 l    d  .init_array	0000000000000000 .init_array
0000000000011678 l    d  .fini_array	0000000000000000 .fini_array
0000000000011680 l    d  .data	0000000000000000 .data
0000000000011dc8 l    d  .sdata	0000000000000000 .sdata
0000000000011de0 l    d  .bss	0000000000000000 .bss
0000000000000000 l    d  .comment	0000000000000000 .comment
0000000000000000 l    d  .riscv.attributes	0000000000000000 .riscv.attributes
0000000000000000 l    d  .debug_aranges	0000000000000000 .debug_aranges
0000000000000000 l    d  .debug_info	0000000000000000 .debug_info
0000000000000000 l    d  .debug_abbrev	0000000000000000 .debug_abbrev
0000000000000000 l    d  .debug_line	0000000000000000 .debug_line
0000000000000000 l    d  .debug_str	0000000000000000 .debug_str
0000000000000000 l    d  .debug_line_str	0000000000000000 .debug_line_str
0000000000000000 l    df *ABS*	0000000000000000 exit.c
0000000000000000 l    df *ABS*	0000000000000000 __call_atexit.c
0000000000010106 l     F .text	0000000000000012 register_fini
0000000000000000 l    df *ABS*	0000000000000000 crt0.o
0000000000000000 l    df *ABS*	0000000000000000 crtstuff.c
00000000000115e0 l     O .eh_frame	0000000000000000 __EH_FRAME_BEGIN__
0000000000010156 l     F .text	0000000000000000 __do_global_dtors_aux
0000000000011de0 l     O .bss	0000000000000001 completed.1
0000000000011678 l     O .fini_array	0000000000000000 __do_global_dtors_aux_fini_array_entry
000000000001018a l     F .text	0000000000000000 frame_dummy
0000000000011de8 l     O .bss	0000000000000030 object.0
0000000000011670 l     O .init_array	0000000000000000 __frame_dummy_init_array_entry
0000000000000000 l    df *ABS*	0000000000000000 tracer.c
00000000000101d4 l       .text	0000000000000000 _fail
0000000000000000 l    df *ABS*	0000000000000000 impure.c
0000000000011680 l     O .data	0000000000000748 impure_data
0000000000000000 l    df *ABS*	0000000000000000 init.c
0000000000000000 l    df *ABS*	0000000000000000 lib_a-memset.o
0000000000000000 l    df *ABS*	0000000000000000 atexit.c
0000000000000000 l    df *ABS*	0000000000000000 fini.c
0000000000000000 l    df *ABS*	0000000000000000 __atexit.c
0000000000000000 l    df *ABS*	0000000000000000 sys_exit.c
0000000000000000 l    df *ABS*	0000000000000000 errno.c
0000000000000000 l    df *ABS*	0000000000000000 crtstuff.c
0000000000011664 l     O .eh_frame	0000000000000000 __FRAME_END__
0000000000011680 l       .fini_array	0000000000000000 __fini_array_end
0000000000011678 l       .fini_array	0000000000000000 __fini_array_start
0000000000011678 l       .init_array	0000000000000000 __init_array_end
0000000000011668 l       .init_array	0000000000000000 __preinit_array_end
0000000000011668 l       .init_array	0000000000000000 __init_array_start
0000000000011668 l       .init_array	0000000000000000 __preinit_array_start
0000000000011e80 g       *ABS*	0000000000000000 __global_pointer$
00000000000105d8 g     F .text	0000000000000006 __errno
0000000000011dc8 g       .sdata	0000000000000000 __SDATA_BEGIN__
0000000000011dd0 g     O .sdata	0000000000000000 .hidden __dso_handle
0000000000011dc8 g     O .sdata	0000000000000008 _global_impure_ptr
00000000000101a8 g     F .text	0000000000000054 _Z4faili
0000000000010328 g     F .text	000000000000006a __libc_init_array
0000000000010508 g     F .text	0000000000000038 __libc_fini_array
000000000001043c g     F .text	00000000000000c2 __call_exitprocs
0000000000010118 g     F .text	000000000000003e _start
0000000000010540 g     F .text	0000000000000076 __register_exitproc
0000000000011e18 g       .bss	0000000000000000 __BSS_END__
0000000000011de0 g       .bss	0000000000000000 __bss_start
0000000000010392 g     F .text	00000000000000aa memset
000000000001023c g     F .text	00000000000000ec main
00000000000104fe g     F .text	000000000000000a atexit
0000000000011dd8 g     O .sdata	0000000000000008 _impure_ptr
0000000000011680 g       .data	0000000000000000 __DATA_BEGIN__
0000000000011de0 g       .sdata	0000000000000000 _edata
0000000000011e18 g       .bss	0000000000000000 _end
00000000000100e8 g     F .text	000000000000001e exit
00000000000101fc g     F .text	0000000000000040 _Z5checki
00000000000105b6 g     F .text	0000000000000022 _exit



Disassembly of section .text:

00000000000100e8 <exit>:
exit():
   100e8:	1141                	add	sp,sp,-16
   100ea:	4581                	li	a1,0
   100ec:	e022                	sd	s0,0(sp)
   100ee:	e406                	sd	ra,8(sp)
   100f0:	842a                	mv	s0,a0
   100f2:	34a000ef          	jal	1043c <__call_exitprocs>
   100f6:	f481b503          	ld	a0,-184(gp) # 11dc8 <_global_impure_ptr>
   100fa:	6d3c                	ld	a5,88(a0)
   100fc:	c391                	beqz	a5,10100 <exit+0x18>
   100fe:	9782                	jalr	a5
   10100:	8522                	mv	a0,s0
   10102:	4b4000ef          	jal	105b6 <_exit>

0000000000010106 <register_fini>:
register_fini():
   10106:	00000793          	li	a5,0
   1010a:	c791                	beqz	a5,10116 <register_fini+0x10>
   1010c:	00000517          	auipc	a0,0x0
   10110:	3fc50513          	add	a0,a0,1020 # 10508 <__libc_fini_array>
   10114:	a6ed                	j	104fe <atexit>
   10116:	8082                	ret

0000000000010118 <_start>:
_start():
   10118:	00002197          	auipc	gp,0x2
   1011c:	d6818193          	add	gp,gp,-664 # 11e80 <__global_pointer$>
   10120:	f6018513          	add	a0,gp,-160 # 11de0 <completed.1>
   10124:	f9818613          	add	a2,gp,-104 # 11e18 <__BSS_END__>
   10128:	8e09                	sub	a2,a2,a0
   1012a:	4581                	li	a1,0
   1012c:	266000ef          	jal	10392 <memset>
   10130:	00000517          	auipc	a0,0x0
   10134:	3ce50513          	add	a0,a0,974 # 104fe <atexit>
   10138:	c519                	beqz	a0,10146 <_start+0x2e>
   1013a:	00000517          	auipc	a0,0x0
   1013e:	3ce50513          	add	a0,a0,974 # 10508 <__libc_fini_array>
   10142:	3bc000ef          	jal	104fe <atexit>
   10146:	1e2000ef          	jal	10328 <__libc_init_array>
   1014a:	4502                	lw	a0,0(sp)
   1014c:	002c                	add	a1,sp,8
   1014e:	4601                	li	a2,0
   10150:	0ec000ef          	jal	1023c <main>
   10154:	bf51                	j	100e8 <exit>

0000000000010156 <__do_global_dtors_aux>:
__do_global_dtors_aux():
   10156:	1141                	add	sp,sp,-16
   10158:	e022                	sd	s0,0(sp)
   1015a:	f6018413          	add	s0,gp,-160 # 11de0 <completed.1>
   1015e:	00044783          	lbu	a5,0(s0)
   10162:	e406                	sd	ra,8(sp)
   10164:	ef99                	bnez	a5,10182 <__do_global_dtors_aux+0x2c>
   10166:	00000793          	li	a5,0
   1016a:	cb89                	beqz	a5,1017c <__do_global_dtors_aux+0x26>
   1016c:	00001517          	auipc	a0,0x1
   10170:	47450513          	add	a0,a0,1140 # 115e0 <__EH_FRAME_BEGIN__>
   10174:	00000097          	auipc	ra,0x0
   10178:	000000e7          	jalr	zero # 0 <exit-0x100e8>
   1017c:	4785                	li	a5,1
   1017e:	00f40023          	sb	a5,0(s0)
   10182:	60a2                	ld	ra,8(sp)
   10184:	6402                	ld	s0,0(sp)
   10186:	0141                	add	sp,sp,16
   10188:	8082                	ret

000000000001018a <frame_dummy>:
frame_dummy():
   1018a:	00000793          	li	a5,0
   1018e:	cb99                	beqz	a5,101a4 <frame_dummy+0x1a>
   10190:	f6818593          	add	a1,gp,-152 # 11de8 <object.0>
   10194:	00001517          	auipc	a0,0x1
   10198:	44c50513          	add	a0,a0,1100 # 115e0 <__EH_FRAME_BEGIN__>
   1019c:	00000317          	auipc	t1,0x0
   101a0:	00000067          	jr	zero # 0 <exit-0x100e8>
   101a4:	8082                	ret
	...

00000000000101a8 <_Z4faili>:
_Z4faili():
/Users/kgriesser/rev/test/tracer/tracer.c:16
   101a8:	fd010113          	add	sp,sp,-48
   101ac:	02813423          	sd	s0,40(sp)
   101b0:	03010413          	add	s0,sp,48
   101b4:	00050793          	mv	a5,a0
   101b8:	fcf42e23          	sw	a5,-36(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:19
   101bc:	fdc42783          	lw	a5,-36(s0)
   101c0:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:20
   101c4:	00100793          	li	a5,1
   101c8:	fef42423          	sw	a5,-24(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:21
   101cc:	fec42783          	lw	a5,-20(s0)
   101d0:	fe842703          	lw	a4,-24(s0)

00000000000101d4 <_fail>:
   101d4:	40e787b3          	sub	a5,a5,a4
   101d8:	fef04ee3          	bgtz	a5,101d4 <_fail>
   101dc:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:24
   101e0:	fec42783          	lw	a5,-20(s0)
   101e4:	0017979b          	sllw	a5,a5,0x1
   101e8:	0007879b          	sext.w	a5,a5
/Users/kgriesser/rev/test/tracer/tracer.c:25
   101ec:	00078513          	mv	a0,a5
   101f0:	02813403          	ld	s0,40(sp)
   101f4:	03010113          	add	sp,sp,48
   101f8:	00008067          	ret

00000000000101fc <_Z5checki>:
_Z5checki():
/Users/kgriesser/rev/test/tracer/tracer.c:27
   101fc:	fe010113          	add	sp,sp,-32
   10200:	00813c23          	sd	s0,24(sp)
   10204:	02010413          	add	s0,sp,32
   10208:	00050793          	mv	a5,a0
   1020c:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:28
   10210:	fec42783          	lw	a5,-20(s0)
   10214:	0007871b          	sext.w	a4,a5
   10218:	02a00793          	li	a5,42
   1021c:	00f70663          	beq	a4,a5,10228 <_Z5checki+0x2c>
/Users/kgriesser/rev/test/tracer/tracer.c:28 (discriminator 1)
   10220:	00100793          	li	a5,1
   10224:	0080006f          	j	1022c <_Z5checki+0x30>
/Users/kgriesser/rev/test/tracer/tracer.c:29
   10228:	00000793          	li	a5,0
/Users/kgriesser/rev/test/tracer/tracer.c:30
   1022c:	00078513          	mv	a0,a5
   10230:	01813403          	ld	s0,24(sp)
   10234:	02010113          	add	sp,sp,32
   10238:	00008067          	ret

000000000001023c <main>:
main():
/Users/kgriesser/rev/test/tracer/tracer.c:32
   1023c:	fd010113          	add	sp,sp,-48
   10240:	02113423          	sd	ra,40(sp)
   10244:	02813023          	sd	s0,32(sp)
   10248:	03010413          	add	s0,sp,48
   1024c:	00050793          	mv	a5,a0
   10250:	fcb43823          	sd	a1,-48(s0)
   10254:	fcf42e23          	sw	a5,-36(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:33
   10258:	00900793          	li	a5,9
   1025c:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:34
   10260:	fec42783          	lw	a5,-20(s0)
   10264:	00078713          	mv	a4,a5
   10268:	fdc42783          	lw	a5,-36(s0)
   1026c:	00f707bb          	addw	a5,a4,a5
   10270:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:35
   10274:	00004033          	xor	zero,zero,zero
/Users/kgriesser/rev/test/tracer/tracer.c:36
   10278:	fe042423          	sw	zero,-24(s0)
   1027c:	0240006f          	j	102a0 <main+0x64>
/Users/kgriesser/rev/test/tracer/tracer.c:37 (discriminator 3)
   10280:	fec42783          	lw	a5,-20(s0)
   10284:	00078713          	mv	a4,a5
   10288:	fe842783          	lw	a5,-24(s0)
   1028c:	00f707bb          	addw	a5,a4,a5
   10290:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:36 (discriminator 3)
   10294:	fe842783          	lw	a5,-24(s0)
   10298:	0017879b          	addw	a5,a5,1
   1029c:	fef42423          	sw	a5,-24(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:36 (discriminator 1)
   102a0:	fe842783          	lw	a5,-24(s0)
   102a4:	0007871b          	sext.w	a4,a5
   102a8:	3e700793          	li	a5,999
   102ac:	fce7dae3          	bge	a5,a4,10280 <main+0x44>
/Users/kgriesser/rev/test/tracer/tracer.c:39
   102b0:	00004033          	xor	zero,zero,zero
/Users/kgriesser/rev/test/tracer/tracer.c:42
   102b4:	aced17b7          	lui	a5,0xaced1
   102b8:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:45
   102bc:	000107b7          	lui	a5,0x10
   102c0:	1a878793          	add	a5,a5,424 # 101a8 <_Z4faili>
   102c4:	fef43023          	sd	a5,-32(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:46
   102c8:	fe043783          	ld	a5,-32(s0)
   102cc:	0007871b          	sext.w	a4,a5
   102d0:	fec42783          	lw	a5,-20(s0)
   102d4:	00f707bb          	addw	a5,a4,a5
   102d8:	0007879b          	sext.w	a5,a5
   102dc:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:48
   102e0:	02a00513          	li	a0,42
   102e4:	f19ff0ef          	jal	101fc <_Z5checki>
   102e8:	00050793          	mv	a5,a0
   102ec:	00f037b3          	snez	a5,a5
   102f0:	0ff7f793          	zext.b	a5,a5
   102f4:	00078e63          	beqz	a5,10310 <main+0xd4>
/Users/kgriesser/rev/test/tracer/tracer.c:49
   102f8:	06400513          	li	a0,100
   102fc:	eadff0ef          	jal	101a8 <_Z4faili>
   10300:	00050793          	mv	a5,a0
   10304:	fec42703          	lw	a4,-20(s0)
   10308:	00f707bb          	addw	a5,a4,a5
   1030c:	fef42623          	sw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:52
   10310:	fec42783          	lw	a5,-20(s0)
/Users/kgriesser/rev/test/tracer/tracer.c:53
   10314:	00078513          	mv	a0,a5
   10318:	02813083          	ld	ra,40(sp)
   1031c:	02013403          	ld	s0,32(sp)
   10320:	03010113          	add	sp,sp,48
   10324:	00008067          	ret

0000000000010328 <__libc_init_array>:
__libc_init_array():
   10328:	1101                	add	sp,sp,-32
   1032a:	e822                	sd	s0,16(sp)
   1032c:	e04a                	sd	s2,0(sp)
   1032e:	00001797          	auipc	a5,0x1
   10332:	33a78793          	add	a5,a5,826 # 11668 <__init_array_start>
   10336:	00001417          	auipc	s0,0x1
   1033a:	33240413          	add	s0,s0,818 # 11668 <__init_array_start>
   1033e:	ec06                	sd	ra,24(sp)
   10340:	e426                	sd	s1,8(sp)
   10342:	40878933          	sub	s2,a5,s0
   10346:	00878b63          	beq	a5,s0,1035c <__libc_init_array+0x34>
   1034a:	40395913          	sra	s2,s2,0x3
   1034e:	4481                	li	s1,0
   10350:	601c                	ld	a5,0(s0)
   10352:	0485                	add	s1,s1,1
   10354:	0421                	add	s0,s0,8
   10356:	9782                	jalr	a5
   10358:	ff24ece3          	bltu	s1,s2,10350 <__libc_init_array+0x28>
   1035c:	00001797          	auipc	a5,0x1
   10360:	31c78793          	add	a5,a5,796 # 11678 <__do_global_dtors_aux_fini_array_entry>
   10364:	00001417          	auipc	s0,0x1
   10368:	30440413          	add	s0,s0,772 # 11668 <__init_array_start>
   1036c:	40878933          	sub	s2,a5,s0
   10370:	40395913          	sra	s2,s2,0x3
   10374:	00878963          	beq	a5,s0,10386 <__libc_init_array+0x5e>
   10378:	4481                	li	s1,0
   1037a:	601c                	ld	a5,0(s0)
   1037c:	0485                	add	s1,s1,1
   1037e:	0421                	add	s0,s0,8
   10380:	9782                	jalr	a5
   10382:	ff24ece3          	bltu	s1,s2,1037a <__libc_init_array+0x52>
   10386:	60e2                	ld	ra,24(sp)
   10388:	6442                	ld	s0,16(sp)
   1038a:	64a2                	ld	s1,8(sp)
   1038c:	6902                	ld	s2,0(sp)
   1038e:	6105                	add	sp,sp,32
   10390:	8082                	ret

0000000000010392 <memset>:
memset():
   10392:	433d                	li	t1,15
   10394:	872a                	mv	a4,a0
   10396:	02c37163          	bgeu	t1,a2,103b8 <memset+0x26>
   1039a:	00f77793          	and	a5,a4,15
   1039e:	e3c1                	bnez	a5,1041e <memset+0x8c>
   103a0:	e1bd                	bnez	a1,10406 <memset+0x74>
   103a2:	ff067693          	and	a3,a2,-16
   103a6:	8a3d                	and	a2,a2,15
   103a8:	96ba                	add	a3,a3,a4
   103aa:	e30c                	sd	a1,0(a4)
   103ac:	e70c                	sd	a1,8(a4)
   103ae:	0741                	add	a4,a4,16
   103b0:	fed76de3          	bltu	a4,a3,103aa <memset+0x18>
   103b4:	e211                	bnez	a2,103b8 <memset+0x26>
   103b6:	8082                	ret
   103b8:	40c306b3          	sub	a3,t1,a2
   103bc:	068a                	sll	a3,a3,0x2
   103be:	00000297          	auipc	t0,0x0
   103c2:	9696                	add	a3,a3,t0
   103c4:	00a68067          	jr	10(a3)
   103c8:	00b70723          	sb	a1,14(a4)
   103cc:	00b706a3          	sb	a1,13(a4)
   103d0:	00b70623          	sb	a1,12(a4)
   103d4:	00b705a3          	sb	a1,11(a4)
   103d8:	00b70523          	sb	a1,10(a4)
   103dc:	00b704a3          	sb	a1,9(a4)
   103e0:	00b70423          	sb	a1,8(a4)
   103e4:	00b703a3          	sb	a1,7(a4)
   103e8:	00b70323          	sb	a1,6(a4)
   103ec:	00b702a3          	sb	a1,5(a4)
   103f0:	00b70223          	sb	a1,4(a4)
   103f4:	00b701a3          	sb	a1,3(a4)
   103f8:	00b70123          	sb	a1,2(a4)
   103fc:	00b700a3          	sb	a1,1(a4)
   10400:	00b70023          	sb	a1,0(a4)
   10404:	8082                	ret
   10406:	0ff5f593          	zext.b	a1,a1
   1040a:	00859693          	sll	a3,a1,0x8
   1040e:	8dd5                	or	a1,a1,a3
   10410:	01059693          	sll	a3,a1,0x10
   10414:	8dd5                	or	a1,a1,a3
   10416:	02059693          	sll	a3,a1,0x20
   1041a:	8dd5                	or	a1,a1,a3
   1041c:	b759                	j	103a2 <memset+0x10>
   1041e:	00279693          	sll	a3,a5,0x2
   10422:	00000297          	auipc	t0,0x0
   10426:	9696                	add	a3,a3,t0
   10428:	8286                	mv	t0,ra
   1042a:	fa2680e7          	jalr	-94(a3)
   1042e:	8096                	mv	ra,t0
   10430:	17c1                	add	a5,a5,-16
   10432:	8f1d                	sub	a4,a4,a5
   10434:	963e                	add	a2,a2,a5
   10436:	f8c371e3          	bgeu	t1,a2,103b8 <memset+0x26>
   1043a:	b79d                	j	103a0 <memset+0xe>

000000000001043c <__call_exitprocs>:
__call_exitprocs():
   1043c:	715d                	add	sp,sp,-80
   1043e:	f052                	sd	s4,32(sp)
   10440:	f481ba03          	ld	s4,-184(gp) # 11dc8 <_global_impure_ptr>
   10444:	f84a                	sd	s2,48(sp)
   10446:	1f8a3903          	ld	s2,504(s4)
   1044a:	e486                	sd	ra,72(sp)
   1044c:	e0a2                	sd	s0,64(sp)
   1044e:	fc26                	sd	s1,56(sp)
   10450:	f44e                	sd	s3,40(sp)
   10452:	ec56                	sd	s5,24(sp)
   10454:	e85a                	sd	s6,16(sp)
   10456:	e45e                	sd	s7,8(sp)
   10458:	e062                	sd	s8,0(sp)
   1045a:	02090863          	beqz	s2,1048a <__call_exitprocs+0x4e>
   1045e:	8b2a                	mv	s6,a0
   10460:	8bae                	mv	s7,a1
   10462:	4a85                	li	s5,1
   10464:	59fd                	li	s3,-1
   10466:	00892483          	lw	s1,8(s2)
   1046a:	fff4841b          	addw	s0,s1,-1
   1046e:	00044e63          	bltz	s0,1048a <__call_exitprocs+0x4e>
   10472:	048e                	sll	s1,s1,0x3
   10474:	94ca                	add	s1,s1,s2
   10476:	020b8663          	beqz	s7,104a2 <__call_exitprocs+0x66>
   1047a:	2084b783          	ld	a5,520(s1)
   1047e:	03778263          	beq	a5,s7,104a2 <__call_exitprocs+0x66>
   10482:	347d                	addw	s0,s0,-1
   10484:	14e1                	add	s1,s1,-8
   10486:	ff3418e3          	bne	s0,s3,10476 <__call_exitprocs+0x3a>
   1048a:	60a6                	ld	ra,72(sp)
   1048c:	6406                	ld	s0,64(sp)
   1048e:	74e2                	ld	s1,56(sp)
   10490:	7942                	ld	s2,48(sp)
   10492:	79a2                	ld	s3,40(sp)
   10494:	7a02                	ld	s4,32(sp)
   10496:	6ae2                	ld	s5,24(sp)
   10498:	6b42                	ld	s6,16(sp)
   1049a:	6ba2                	ld	s7,8(sp)
   1049c:	6c02                	ld	s8,0(sp)
   1049e:	6161                	add	sp,sp,80
   104a0:	8082                	ret
   104a2:	00892783          	lw	a5,8(s2)
   104a6:	6498                	ld	a4,8(s1)
   104a8:	37fd                	addw	a5,a5,-1
   104aa:	04878463          	beq	a5,s0,104f2 <__call_exitprocs+0xb6>
   104ae:	0004b423          	sd	zero,8(s1)
   104b2:	db61                	beqz	a4,10482 <__call_exitprocs+0x46>
   104b4:	31092783          	lw	a5,784(s2)
   104b8:	008a96bb          	sllw	a3,s5,s0
   104bc:	00892c03          	lw	s8,8(s2)
   104c0:	8ff5                	and	a5,a5,a3
   104c2:	2781                	sext.w	a5,a5
   104c4:	ef89                	bnez	a5,104de <__call_exitprocs+0xa2>
   104c6:	9702                	jalr	a4
   104c8:	00892703          	lw	a4,8(s2)
   104cc:	1f8a3783          	ld	a5,504(s4)
   104d0:	01871463          	bne	a4,s8,104d8 <__call_exitprocs+0x9c>
   104d4:	fb2787e3          	beq	a5,s2,10482 <__call_exitprocs+0x46>
   104d8:	dbcd                	beqz	a5,1048a <__call_exitprocs+0x4e>
   104da:	893e                	mv	s2,a5
   104dc:	b769                	j	10466 <__call_exitprocs+0x2a>
   104de:	31492783          	lw	a5,788(s2)
   104e2:	1084b583          	ld	a1,264(s1)
   104e6:	8ff5                	and	a5,a5,a3
   104e8:	2781                	sext.w	a5,a5
   104ea:	e799                	bnez	a5,104f8 <__call_exitprocs+0xbc>
   104ec:	855a                	mv	a0,s6
   104ee:	9702                	jalr	a4
   104f0:	bfe1                	j	104c8 <__call_exitprocs+0x8c>
   104f2:	00892423          	sw	s0,8(s2)
   104f6:	bf75                	j	104b2 <__call_exitprocs+0x76>
   104f8:	852e                	mv	a0,a1
   104fa:	9702                	jalr	a4
   104fc:	b7f1                	j	104c8 <__call_exitprocs+0x8c>

00000000000104fe <atexit>:
atexit():
   104fe:	85aa                	mv	a1,a0
   10500:	4681                	li	a3,0
   10502:	4601                	li	a2,0
   10504:	4501                	li	a0,0
   10506:	a82d                	j	10540 <__register_exitproc>

0000000000010508 <__libc_fini_array>:
__libc_fini_array():
   10508:	1101                	add	sp,sp,-32
   1050a:	e822                	sd	s0,16(sp)
   1050c:	00001797          	auipc	a5,0x1
   10510:	16c78793          	add	a5,a5,364 # 11678 <__do_global_dtors_aux_fini_array_entry>
   10514:	00001417          	auipc	s0,0x1
   10518:	16c40413          	add	s0,s0,364 # 11680 <impure_data>
   1051c:	8c1d                	sub	s0,s0,a5
   1051e:	e426                	sd	s1,8(sp)
   10520:	ec06                	sd	ra,24(sp)
   10522:	40345493          	sra	s1,s0,0x3
   10526:	c881                	beqz	s1,10536 <__libc_fini_array+0x2e>
   10528:	1461                	add	s0,s0,-8
   1052a:	943e                	add	s0,s0,a5
   1052c:	601c                	ld	a5,0(s0)
   1052e:	14fd                	add	s1,s1,-1
   10530:	1461                	add	s0,s0,-8
   10532:	9782                	jalr	a5
   10534:	fce5                	bnez	s1,1052c <__libc_fini_array+0x24>
   10536:	60e2                	ld	ra,24(sp)
   10538:	6442                	ld	s0,16(sp)
   1053a:	64a2                	ld	s1,8(sp)
   1053c:	6105                	add	sp,sp,32
   1053e:	8082                	ret

0000000000010540 <__register_exitproc>:
__register_exitproc():
   10540:	f481b703          	ld	a4,-184(gp) # 11dc8 <_global_impure_ptr>
   10544:	1f873783          	ld	a5,504(a4)
   10548:	c3b1                	beqz	a5,1058c <__register_exitproc+0x4c>
   1054a:	4798                	lw	a4,8(a5)
   1054c:	487d                	li	a6,31
   1054e:	06e84263          	blt	a6,a4,105b2 <__register_exitproc+0x72>
   10552:	c505                	beqz	a0,1057a <__register_exitproc+0x3a>
   10554:	00371813          	sll	a6,a4,0x3
   10558:	983e                	add	a6,a6,a5
   1055a:	10c83823          	sd	a2,272(a6)
   1055e:	3107a883          	lw	a7,784(a5)
   10562:	4605                	li	a2,1
   10564:	00e6163b          	sllw	a2,a2,a4
   10568:	00c8e8b3          	or	a7,a7,a2
   1056c:	3117a823          	sw	a7,784(a5)
   10570:	20d83823          	sd	a3,528(a6)
   10574:	4689                	li	a3,2
   10576:	02d50063          	beq	a0,a3,10596 <__register_exitproc+0x56>
   1057a:	00270693          	add	a3,a4,2
   1057e:	068e                	sll	a3,a3,0x3
   10580:	2705                	addw	a4,a4,1
   10582:	c798                	sw	a4,8(a5)
   10584:	97b6                	add	a5,a5,a3
   10586:	e38c                	sd	a1,0(a5)
   10588:	4501                	li	a0,0
   1058a:	8082                	ret
   1058c:	20070793          	add	a5,a4,512
   10590:	1ef73c23          	sd	a5,504(a4)
   10594:	bf5d                	j	1054a <__register_exitproc+0xa>
   10596:	3147a683          	lw	a3,788(a5)
   1059a:	4501                	li	a0,0
   1059c:	8ed1                	or	a3,a3,a2
   1059e:	30d7aa23          	sw	a3,788(a5)
   105a2:	00270693          	add	a3,a4,2
   105a6:	068e                	sll	a3,a3,0x3
   105a8:	2705                	addw	a4,a4,1
   105aa:	c798                	sw	a4,8(a5)
   105ac:	97b6                	add	a5,a5,a3
   105ae:	e38c                	sd	a1,0(a5)
   105b0:	8082                	ret
   105b2:	557d                	li	a0,-1
   105b4:	8082                	ret

00000000000105b6 <_exit>:
_exit():
   105b6:	05d00893          	li	a7,93
   105ba:	00000073          	ecall
   105be:	00054363          	bltz	a0,105c4 <_exit+0xe>
   105c2:	a001                	j	105c2 <_exit+0xc>
   105c4:	1141                	add	sp,sp,-16
   105c6:	e022                	sd	s0,0(sp)
   105c8:	842a                	mv	s0,a0
   105ca:	e406                	sd	ra,8(sp)
   105cc:	4080043b          	negw	s0,s0
   105d0:	008000ef          	jal	105d8 <__errno>
   105d4:	c100                	sw	s0,0(a0)
   105d6:	a001                	j	105d6 <_exit+0x20>

00000000000105d8 <__errno>:
__errno():
   105d8:	f581b503          	ld	a0,-168(gp) # 11dd8 <_impure_ptr>
   105dc:	8082                	ret
