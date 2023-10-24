
x0.exe:     file format elf64-littleriscv


Disassembly of section .text:

00000000000100e8 <exit>:
   100e8:	1141                	c.addi	sp, -16
   100ea:	4581                	c.li	a1, 0
   100ec:	e022                	c.sdsp	s0, 0(sp)
   100ee:	e406                	c.sdsp	ra, 8(sp)
   100f0:	842a                	c.mv	s0, a0
   100f2:	1ee000ef          	jal	ra, 102e0 <__call_exitprocs>
   100f6:	f481b503          	ld	a0, -184(gp) # 11be8 <_global_impure_ptr>
   100fa:	6d3c                	c.ld	a5, 88(a0)
   100fc:	c391                	c.beqz	a5, 10100 <exit+0x18>
   100fe:	9782                	c.jalr	a5
   10100:	8522                	c.mv	a0, s0
   10102:	354000ef          	jal	ra, 10456 <_exit>

0000000000010106 <register_fini>:
   10106:	00000793          	addi	a5, zero, 0
   1010a:	c789                	c.beqz	a5, 10114 <register_fini+0xe>
   1010c:	6541                	c.lui	a0, 0x10
   1010e:	3ac50513          	addi	a0, a0, 940 # 103ac <__libc_fini_array>
   10112:	ac41                	c.j	103a2 <atexit>
   10114:	8082                	c.jr	ra

0000000000010116 <_start>:
   10116:	00002197          	auipc	gp, 0x2
   1011a:	b8a18193          	addi	gp, gp, -1142 # 11ca0 <__global_pointer$>
   1011e:	f6018513          	addi	a0, gp, -160 # 11c00 <completed.1>
   10122:	f9818613          	addi	a2, gp, -104 # 11c38 <__BSS_END__>
   10126:	8e09                	c.sub	a2, a0
   10128:	4581                	c.li	a1, 0
   1012a:	10c000ef          	jal	ra, 10236 <memset>
   1012e:	00000517          	auipc	a0, 0x0
   10132:	27450513          	addi	a0, a0, 628 # 103a2 <atexit>
   10136:	c519                	c.beqz	a0, 10144 <_start+0x2e>
   10138:	00000517          	auipc	a0, 0x0
   1013c:	27450513          	addi	a0, a0, 628 # 103ac <__libc_fini_array>
   10140:	262000ef          	jal	ra, 103a2 <atexit>
   10144:	088000ef          	jal	ra, 101cc <__libc_init_array>
   10148:	4502                	c.lwsp	a0, 0(sp)
   1014a:	002c                	c.addi4spn	a1, sp, 8
   1014c:	4601                	c.li	a2, 0
   1014e:	04e000ef          	jal	ra, 1019c <main>
   10152:	bf59                	c.j	100e8 <exit>

0000000000010154 <__do_global_dtors_aux>:
   10154:	1141                	c.addi	sp, -16
   10156:	e022                	c.sdsp	s0, 0(sp)
   10158:	f601c783          	lbu	a5, -160(gp) # 11c00 <completed.1>
   1015c:	e406                	c.sdsp	ra, 8(sp)
   1015e:	ef91                	c.bnez	a5, 1017a <__do_global_dtors_aux+0x26>
   10160:	00000793          	addi	a5, zero, 0
   10164:	cb81                	c.beqz	a5, 10174 <__do_global_dtors_aux+0x20>
   10166:	6545                	c.lui	a0, 0x11
   10168:	48050513          	addi	a0, a0, 1152 # 11480 <__EH_FRAME_BEGIN__>
   1016c:	00000097          	auipc	ra, 0x0
   10170:	000000e7          	jalr	ra, 0(zero) # 0 <exit-0x100e8>
   10174:	4785                	c.li	a5, 1
   10176:	f6f18023          	sb	a5, -160(gp) # 11c00 <completed.1>
   1017a:	60a2                	c.ldsp	ra, 8(sp)
   1017c:	6402                	c.ldsp	s0, 0(sp)
   1017e:	0141                	c.addi	sp, 16
   10180:	8082                	c.jr	ra

0000000000010182 <frame_dummy>:
   10182:	00000793          	addi	a5, zero, 0
   10186:	cb91                	c.beqz	a5, 1019a <frame_dummy+0x18>
   10188:	6545                	c.lui	a0, 0x11
   1018a:	f6818593          	addi	a1, gp, -152 # 11c08 <object.0>
   1018e:	48050513          	addi	a0, a0, 1152 # 11480 <__EH_FRAME_BEGIN__>
   10192:	00000317          	auipc	t1, 0x0
   10196:	00000067          	jalr	zero, 0(zero) # 0 <exit-0x100e8>
   1019a:	8082                	c.jr	ra

000000000001019c <main>:
   1019c:	1101                	c.addi	sp, -32
   1019e:	ec22                	c.sdsp	s0, 24(sp)
   101a0:	1000                	c.addi4spn	s0, sp, 32
   101a2:	87aa                	c.mv	a5, a0
   101a4:	feb43023          	sd	a1, -32(s0)
   101a8:	fef42623          	sw	a5, -20(s0)
   101ac:	4519                	c.li	a0, 6
   101ae:	4599                	c.li	a1, 6
   101b0:	00050013          	addi	zero, a0, 0
   101b4:	852a                	c.mv	a0, a0
   101b6:	00b51363          	bne	a0, a1, 101bc <fail>

00000000000101ba <pass>:
   101ba:	a019                	c.j	101c0 <continue>

00000000000101bc <fail>:
   101bc:	0000                	c.unimp
	...

00000000000101c0 <continue>:
   101c0:	4081                	c.li	ra, 0
   101c2:	4781                	c.li	a5, 0
   101c4:	853e                	c.mv	a0, a5
   101c6:	6462                	c.ldsp	s0, 24(sp)
   101c8:	6105                	c.addi16sp	sp, 32
   101ca:	8082                	c.jr	ra

00000000000101cc <__libc_init_array>:
   101cc:	1101                	c.addi	sp, -32
   101ce:	e822                	c.sdsp	s0, 16(sp)
   101d0:	67c5                	c.lui	a5, 0x11
   101d2:	6445                	c.lui	s0, 0x11
   101d4:	e04a                	c.sdsp	s2, 0(sp)
   101d6:	48478793          	addi	a5, a5, 1156 # 11484 <__preinit_array_end>
   101da:	48440713          	addi	a4, s0, 1156 # 11484 <__preinit_array_end>
   101de:	ec06                	c.sdsp	ra, 24(sp)
   101e0:	e426                	c.sdsp	s1, 8(sp)
   101e2:	40e78933          	sub	s2, a5, a4
   101e6:	00e78d63          	beq	a5, a4, 10200 <__libc_init_array+0x34>
   101ea:	40395913          	srai	s2, s2, 0x3
   101ee:	48440413          	addi	s0, s0, 1156
   101f2:	4481                	c.li	s1, 0
   101f4:	601c                	c.ld	a5, 0(s0)
   101f6:	0485                	c.addi	s1, 1
   101f8:	0421                	c.addi	s0, 8
   101fa:	9782                	c.jalr	a5
   101fc:	ff24ece3          	bltu	s1, s2, 101f4 <__libc_init_array+0x28>
   10200:	6445                	c.lui	s0, 0x11
   10202:	67c5                	c.lui	a5, 0x11
   10204:	49878793          	addi	a5, a5, 1176 # 11498 <__do_global_dtors_aux_fini_array_entry>
   10208:	48840713          	addi	a4, s0, 1160 # 11488 <__init_array_start>
   1020c:	40e78933          	sub	s2, a5, a4
   10210:	40395913          	srai	s2, s2, 0x3
   10214:	00e78b63          	beq	a5, a4, 1022a <__libc_init_array+0x5e>
   10218:	48840413          	addi	s0, s0, 1160
   1021c:	4481                	c.li	s1, 0
   1021e:	601c                	c.ld	a5, 0(s0)
   10220:	0485                	c.addi	s1, 1
   10222:	0421                	c.addi	s0, 8
   10224:	9782                	c.jalr	a5
   10226:	ff24ece3          	bltu	s1, s2, 1021e <__libc_init_array+0x52>
   1022a:	60e2                	c.ldsp	ra, 24(sp)
   1022c:	6442                	c.ldsp	s0, 16(sp)
   1022e:	64a2                	c.ldsp	s1, 8(sp)
   10230:	6902                	c.ldsp	s2, 0(sp)
   10232:	6105                	c.addi16sp	sp, 32
   10234:	8082                	c.jr	ra

0000000000010236 <memset>:
   10236:	433d                	c.li	t1, 15
   10238:	872a                	c.mv	a4, a0
   1023a:	02c37163          	bgeu	t1, a2, 1025c <memset+0x26>
   1023e:	00f77793          	andi	a5, a4, 15
   10242:	e3c1                	c.bnez	a5, 102c2 <memset+0x8c>
   10244:	e1bd                	c.bnez	a1, 102aa <memset+0x74>
   10246:	ff067693          	andi	a3, a2, -16
   1024a:	8a3d                	c.andi	a2, 15
   1024c:	96ba                	c.add	a3, a4
   1024e:	e30c                	c.sd	a1, 0(a4)
   10250:	e70c                	c.sd	a1, 8(a4)
   10252:	0741                	c.addi	a4, 16
   10254:	fed76de3          	bltu	a4, a3, 1024e <memset+0x18>
   10258:	e211                	c.bnez	a2, 1025c <memset+0x26>
   1025a:	8082                	c.jr	ra
   1025c:	40c306b3          	sub	a3, t1, a2
   10260:	068a                	c.slli	a3, 0x2
   10262:	00000297          	auipc	t0, 0x0
   10266:	9696                	c.add	a3, t0
   10268:	00a68067          	jalr	zero, 10(a3)
   1026c:	00b70723          	sb	a1, 14(a4)
   10270:	00b706a3          	sb	a1, 13(a4)
   10274:	00b70623          	sb	a1, 12(a4)
   10278:	00b705a3          	sb	a1, 11(a4)
   1027c:	00b70523          	sb	a1, 10(a4)
   10280:	00b704a3          	sb	a1, 9(a4)
   10284:	00b70423          	sb	a1, 8(a4)
   10288:	00b703a3          	sb	a1, 7(a4)
   1028c:	00b70323          	sb	a1, 6(a4)
   10290:	00b702a3          	sb	a1, 5(a4)
   10294:	00b70223          	sb	a1, 4(a4)
   10298:	00b701a3          	sb	a1, 3(a4)
   1029c:	00b70123          	sb	a1, 2(a4)
   102a0:	00b700a3          	sb	a1, 1(a4)
   102a4:	00b70023          	sb	a1, 0(a4)
   102a8:	8082                	c.jr	ra
   102aa:	0ff5f593          	andi	a1, a1, 255
   102ae:	00859693          	slli	a3, a1, 0x8
   102b2:	8dd5                	c.or	a1, a3
   102b4:	01059693          	slli	a3, a1, 0x10
   102b8:	8dd5                	c.or	a1, a3
   102ba:	02059693          	slli	a3, a1, 0x20
   102be:	8dd5                	c.or	a1, a3
   102c0:	b759                	c.j	10246 <memset+0x10>
   102c2:	00279693          	slli	a3, a5, 0x2
   102c6:	00000297          	auipc	t0, 0x0
   102ca:	9696                	c.add	a3, t0
   102cc:	8286                	c.mv	t0, ra
   102ce:	fa2680e7          	jalr	ra, -94(a3)
   102d2:	8096                	c.mv	ra, t0
   102d4:	17c1                	c.addi	a5, -16
   102d6:	8f1d                	c.sub	a4, a5
   102d8:	963e                	c.add	a2, a5
   102da:	f8c371e3          	bgeu	t1, a2, 1025c <memset+0x26>
   102de:	b79d                	c.j	10244 <memset+0xe>

00000000000102e0 <__call_exitprocs>:
   102e0:	715d                	c.addi16sp	sp, -80
   102e2:	f052                	c.sdsp	s4, 32(sp)
   102e4:	f481ba03          	ld	s4, -184(gp) # 11be8 <_global_impure_ptr>
   102e8:	f84a                	c.sdsp	s2, 48(sp)
   102ea:	e486                	c.sdsp	ra, 72(sp)
   102ec:	1f8a3903          	ld	s2, 504(s4)
   102f0:	e0a2                	c.sdsp	s0, 64(sp)
   102f2:	fc26                	c.sdsp	s1, 56(sp)
   102f4:	f44e                	c.sdsp	s3, 40(sp)
   102f6:	ec56                	c.sdsp	s5, 24(sp)
   102f8:	e85a                	c.sdsp	s6, 16(sp)
   102fa:	e45e                	c.sdsp	s7, 8(sp)
   102fc:	e062                	c.sdsp	s8, 0(sp)
   102fe:	02090863          	beq	s2, zero, 1032e <__call_exitprocs+0x4e>
   10302:	8b2a                	c.mv	s6, a0
   10304:	8bae                	c.mv	s7, a1
   10306:	4a85                	c.li	s5, 1
   10308:	59fd                	c.li	s3, -1
   1030a:	00892483          	lw	s1, 8(s2)
   1030e:	fff4841b          	addiw	s0, s1, -1
   10312:	00044e63          	blt	s0, zero, 1032e <__call_exitprocs+0x4e>
   10316:	048e                	c.slli	s1, 0x3
   10318:	94ca                	c.add	s1, s2
   1031a:	020b8663          	beq	s7, zero, 10346 <__call_exitprocs+0x66>
   1031e:	2084b783          	ld	a5, 520(s1)
   10322:	03778263          	beq	a5, s7, 10346 <__call_exitprocs+0x66>
   10326:	347d                	c.addiw	s0, -1
   10328:	14e1                	c.addi	s1, -8
   1032a:	ff3418e3          	bne	s0, s3, 1031a <__call_exitprocs+0x3a>
   1032e:	60a6                	c.ldsp	ra, 72(sp)
   10330:	6406                	c.ldsp	s0, 64(sp)
   10332:	74e2                	c.ldsp	s1, 56(sp)
   10334:	7942                	c.ldsp	s2, 48(sp)
   10336:	79a2                	c.ldsp	s3, 40(sp)
   10338:	7a02                	c.ldsp	s4, 32(sp)
   1033a:	6ae2                	c.ldsp	s5, 24(sp)
   1033c:	6b42                	c.ldsp	s6, 16(sp)
   1033e:	6ba2                	c.ldsp	s7, 8(sp)
   10340:	6c02                	c.ldsp	s8, 0(sp)
   10342:	6161                	c.addi16sp	sp, 80
   10344:	8082                	c.jr	ra
   10346:	00892783          	lw	a5, 8(s2)
   1034a:	6498                	c.ld	a4, 8(s1)
   1034c:	37fd                	c.addiw	a5, -1
   1034e:	04878463          	beq	a5, s0, 10396 <__call_exitprocs+0xb6>
   10352:	0004b423          	sd	zero, 8(s1)
   10356:	db61                	c.beqz	a4, 10326 <__call_exitprocs+0x46>
   10358:	31092783          	lw	a5, 784(s2)
   1035c:	008a96bb          	sllw	a3, s5, s0
   10360:	00892c03          	lw	s8, 8(s2)
   10364:	8ff5                	c.and	a5, a3
   10366:	2781                	c.addiw	a5, 0
   10368:	ef89                	c.bnez	a5, 10382 <__call_exitprocs+0xa2>
   1036a:	9702                	c.jalr	a4
   1036c:	00892703          	lw	a4, 8(s2)
   10370:	1f8a3783          	ld	a5, 504(s4)
   10374:	01871463          	bne	a4, s8, 1037c <__call_exitprocs+0x9c>
   10378:	fb2787e3          	beq	a5, s2, 10326 <__call_exitprocs+0x46>
   1037c:	dbcd                	c.beqz	a5, 1032e <__call_exitprocs+0x4e>
   1037e:	893e                	c.mv	s2, a5
   10380:	b769                	c.j	1030a <__call_exitprocs+0x2a>
   10382:	31492783          	lw	a5, 788(s2)
   10386:	1084b583          	ld	a1, 264(s1)
   1038a:	8ff5                	c.and	a5, a3
   1038c:	2781                	c.addiw	a5, 0
   1038e:	e799                	c.bnez	a5, 1039c <__call_exitprocs+0xbc>
   10390:	855a                	c.mv	a0, s6
   10392:	9702                	c.jalr	a4
   10394:	bfe1                	c.j	1036c <__call_exitprocs+0x8c>
   10396:	00892423          	sw	s0, 8(s2)
   1039a:	bf75                	c.j	10356 <__call_exitprocs+0x76>
   1039c:	852e                	c.mv	a0, a1
   1039e:	9702                	c.jalr	a4
   103a0:	b7f1                	c.j	1036c <__call_exitprocs+0x8c>

00000000000103a2 <atexit>:
   103a2:	85aa                	c.mv	a1, a0
   103a4:	4681                	c.li	a3, 0
   103a6:	4601                	c.li	a2, 0
   103a8:	4501                	c.li	a0, 0
   103aa:	a81d                	c.j	103e0 <__register_exitproc>

00000000000103ac <__libc_fini_array>:
   103ac:	1101                	c.addi	sp, -32
   103ae:	e822                	c.sdsp	s0, 16(sp)
   103b0:	67c5                	c.lui	a5, 0x11
   103b2:	6445                	c.lui	s0, 0x11
   103b4:	49878793          	addi	a5, a5, 1176 # 11498 <__do_global_dtors_aux_fini_array_entry>
   103b8:	4a040413          	addi	s0, s0, 1184 # 114a0 <impure_data>
   103bc:	8c1d                	c.sub	s0, a5
   103be:	e426                	c.sdsp	s1, 8(sp)
   103c0:	ec06                	c.sdsp	ra, 24(sp)
   103c2:	40345493          	srai	s1, s0, 0x3
   103c6:	c881                	c.beqz	s1, 103d6 <__libc_fini_array+0x2a>
   103c8:	1461                	c.addi	s0, -8
   103ca:	943e                	c.add	s0, a5
   103cc:	601c                	c.ld	a5, 0(s0)
   103ce:	14fd                	c.addi	s1, -1
   103d0:	1461                	c.addi	s0, -8
   103d2:	9782                	c.jalr	a5
   103d4:	fce5                	c.bnez	s1, 103cc <__libc_fini_array+0x20>
   103d6:	60e2                	c.ldsp	ra, 24(sp)
   103d8:	6442                	c.ldsp	s0, 16(sp)
   103da:	64a2                	c.ldsp	s1, 8(sp)
   103dc:	6105                	c.addi16sp	sp, 32
   103de:	8082                	c.jr	ra

00000000000103e0 <__register_exitproc>:
   103e0:	f481b703          	ld	a4, -184(gp) # 11be8 <_global_impure_ptr>
   103e4:	1f873783          	ld	a5, 504(a4)
   103e8:	c3b1                	c.beqz	a5, 1042c <__register_exitproc+0x4c>
   103ea:	4798                	c.lw	a4, 8(a5)
   103ec:	487d                	c.li	a6, 31
   103ee:	06e84263          	blt	a6, a4, 10452 <__register_exitproc+0x72>
   103f2:	c505                	c.beqz	a0, 1041a <__register_exitproc+0x3a>
   103f4:	00371813          	slli	a6, a4, 0x3
   103f8:	983e                	c.add	a6, a5
   103fa:	10c83823          	sd	a2, 272(a6)
   103fe:	3107a883          	lw	a7, 784(a5)
   10402:	4605                	c.li	a2, 1
   10404:	00e6163b          	sllw	a2, a2, a4
   10408:	00c8e8b3          	or	a7, a7, a2
   1040c:	3117a823          	sw	a7, 784(a5)
   10410:	20d83823          	sd	a3, 528(a6)
   10414:	4689                	c.li	a3, 2
   10416:	02d50063          	beq	a0, a3, 10436 <__register_exitproc+0x56>
   1041a:	00270693          	addi	a3, a4, 2
   1041e:	068e                	c.slli	a3, 0x3
   10420:	2705                	c.addiw	a4, 1
   10422:	c798                	c.sw	a4, 8(a5)
   10424:	97b6                	c.add	a5, a3
   10426:	e38c                	c.sd	a1, 0(a5)
   10428:	4501                	c.li	a0, 0
   1042a:	8082                	c.jr	ra
   1042c:	20070793          	addi	a5, a4, 512
   10430:	1ef73c23          	sd	a5, 504(a4)
   10434:	bf5d                	c.j	103ea <__register_exitproc+0xa>
   10436:	3147a683          	lw	a3, 788(a5)
   1043a:	4501                	c.li	a0, 0
   1043c:	8ed1                	c.or	a3, a2
   1043e:	30d7aa23          	sw	a3, 788(a5)
   10442:	00270693          	addi	a3, a4, 2
   10446:	068e                	c.slli	a3, 0x3
   10448:	2705                	c.addiw	a4, 1
   1044a:	c798                	c.sw	a4, 8(a5)
   1044c:	97b6                	c.add	a5, a3
   1044e:	e38c                	c.sd	a1, 0(a5)
   10450:	8082                	c.jr	ra
   10452:	557d                	c.li	a0, -1
   10454:	8082                	c.jr	ra

0000000000010456 <_exit>:
   10456:	05d00893          	addi	a7, zero, 93
   1045a:	00000073          	ecall
   1045e:	00054363          	blt	a0, zero, 10464 <_exit+0xe>
   10462:	a001                	c.j	10462 <_exit+0xc>
   10464:	1141                	c.addi	sp, -16
   10466:	e022                	c.sdsp	s0, 0(sp)
   10468:	842a                	c.mv	s0, a0
   1046a:	e406                	c.sdsp	ra, 8(sp)
   1046c:	4080043b          	subw	s0, zero, s0
   10470:	008000ef          	jal	ra, 10478 <__errno>
   10474:	c100                	c.sw	s0, 0(a0)
   10476:	a001                	c.j	10476 <_exit+0x20>

0000000000010478 <__errno>:
   10478:	f581b503          	ld	a0, -168(gp) # 11bf8 <_impure_ptr>
   1047c:	8082                	c.jr	ra

Disassembly of section .eh_frame:

0000000000011480 <__EH_FRAME_BEGIN__>:
   11480:	0000                	c.unimp
	...

Disassembly of section .init_array:

0000000000011488 <__init_array_start>:
   11488:	0106                	c.slli	sp, 0x1
   1148a:	0001                	c.addi	zero, 0
   1148c:	0000                	c.unimp
	...

0000000000011490 <__frame_dummy_init_array_entry>:
   11490:	0182                	c.slli64	gp
   11492:	0001                	c.addi	zero, 0
   11494:	0000                	c.unimp
	...

Disassembly of section .fini_array:

0000000000011498 <__do_global_dtors_aux_fini_array_entry>:
   11498:	0154                	c.addi4spn	a3, sp, 132
   1149a:	0001                	c.addi	zero, 0
   1149c:	0000                	c.unimp
	...

Disassembly of section .data:

00000000000114a0 <impure_data>:
	...
   114a8:	19d8                	c.addi4spn	a4, sp, 244
   114aa:	0001                	c.addi	zero, 0
   114ac:	0000                	c.unimp
   114ae:	0000                	c.unimp
   114b0:	1a88                	c.addi4spn	a0, sp, 368
   114b2:	0001                	c.addi	zero, 0
   114b4:	0000                	c.unimp
   114b6:	0000                	c.unimp
   114b8:	1b38                	c.addi4spn	a4, sp, 440
   114ba:	0001                	c.addi	zero, 0
	...
   11588:	0001                	c.addi	zero, 0
   1158a:	0000                	c.unimp
   1158c:	0000                	c.unimp
   1158e:	0000                	c.unimp
   11590:	330e                	c.fldsp	ft6, 224(sp)
   11592:	abcd                	c.j	11b84 <impure_data+0x6e4>
   11594:	1234                	c.addi4spn	a3, sp, 296
   11596:	e66d                	c.bnez	a2, 11680 <impure_data+0x1e0>
   11598:	deec                	c.sw	a1, 124(a3)
   1159a:	0005                	c.addi	zero, 1
   1159c:	0000000b          	.4byte	0xb
	...

Disassembly of section .sdata:

0000000000011be8 <_global_impure_ptr>:
   11be8:	14a0                	c.addi4spn	s0, sp, 616
   11bea:	0001                	c.addi	zero, 0
   11bec:	0000                	c.unimp
	...

0000000000011bf0 <__dso_handle>:
	...

0000000000011bf8 <_impure_ptr>:
   11bf8:	14a0                	c.addi4spn	s0, sp, 616
   11bfa:	0001                	c.addi	zero, 0
   11bfc:	0000                	c.unimp
	...

Disassembly of section .bss:

0000000000011c00 <completed.1>:
	...

0000000000011c08 <object.0>:
	...

Disassembly of section .comment:

0000000000000000 <.comment>:
   0:	3a434347          	fmsub.d	ft6, ft6, ft4, ft7, rmm
   4:	2820                	c.fld	fs0, 80(s0)
   6:	2029                	.2byte	0x2029
   8:	3231                	c.addiw	tp, -20 # ffffffffffffffec <__global_pointer$+0xfffffffffffee34c>
   a:	322e                	c.fldsp	ft4, 232(sp)
   c:	302e                	c.fldsp	ft0, 232(sp)
   e:	4700                	c.lw	s0, 8(a4)
  10:	203a4343          	fmadd.s	ft6, fs4, ft3, ft4, rmm
  14:	4728                	c.lw	a0, 72(a4)
  16:	554e                	c.lwsp	a0, 240(sp)
  18:	2029                	.2byte	0x2029
  1a:	3231                	c.addiw	tp, -20 # ffffffffffffffec <__global_pointer$+0xfffffffffffee34c>
  1c:	322e                	c.fldsp	ft4, 232(sp)
  1e:	302e                	c.fldsp	ft0, 232(sp)
	...

Disassembly of section .riscv.attributes:

0000000000000000 <.riscv.attributes>:
   0:	3d41                	c.addiw	s10, -16
   2:	0000                	c.unimp
   4:	7200                	c.ld	s0, 32(a2)
   6:	7369                	c.lui	t1, 0xffffa
   8:	01007663          	bgeu	zero, a6, 14 <exit-0x100d4>
   c:	00000033          	add	zero, zero, zero
  10:	1004                	c.addi4spn	s1, sp, 32
  12:	7205                	c.lui	tp, 0xfffe1
  14:	3676                	c.fldsp	fa2, 376(sp)
  16:	6934                	c.ld	a3, 80(a0)
  18:	7032                	.2byte	0x7032
  1a:	5f30                	c.lw	a2, 120(a4)
  1c:	326d                	c.addiw	tp, -5 # fffffffffffe0ffb <__global_pointer$+0xfffffffffffcf35b>
  1e:	3070                	c.fld	fa2, 224(s0)
  20:	615f 7032 5f30      	.byte	0x5f, 0x61, 0x32, 0x70, 0x30, 0x5f
  26:	3266                	c.fldsp	ft4, 120(sp)
  28:	3070                	c.fld	fa2, 224(s0)
  2a:	645f 7032 5f30      	.byte	0x5f, 0x64, 0x32, 0x70, 0x30, 0x5f
  30:	30703263          	.4byte	0x30703263
  34:	7a5f 6d6d 6c75      	.byte	0x5f, 0x7a, 0x6d, 0x6d, 0x75, 0x6c
  3a:	7031                	c.lui	zero, 0xfffec
  3c:	0030                	c.addi4spn	a2, sp, 8
