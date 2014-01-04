/*
 *  opcodes.h
 *  Pura 
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _opcodes_h_
#define _opcodes_h_

#define NOP 0 /* no operation opcode */

/* push constants onto the stack */
#define ACONST_NULL 1 /* u1; push null reference onto the stack */

#define ICONST_M1 2 /* push integer constant -1 onto the stack */

#define ICONST_0 3 /* push integer value 0 onto the stack */
#define ICONST_1 4 /* push integer value 1 onto the stack */
#define ICONST_2 5 /* push integer value 2 onto the stack */
#define ICONST_3 6 /* push integer value 3 onto the stack */
#define ICONST_4 7 /* push integer value 4 onto the stack */
#define ICONST_5 8 /* push integer value 5 onto the stack */

#define LCONST_0 9 /* u1; push the long integer 0 onto the stack */
#define LCONST_1 10 /* u1; push the long integer 1 onto the stack */

#define FCONST_0 11 /* u1; push the single float 0.0 onto the stack */
#define FCONST_1 12 /* u1; push the single float 1.0 onto the stack */
#define FCONST_2 13 /* u1; push the single float 2.0 onto the stack */

#define DCONST_0 14 /* u1, u1; push the double 0.0 onto the stack */
#define DCONST_1 15 /* u1, u1; push the double 1.0 onto the stack */

/* stack manipulation */
#define BIPUSH 16 /* u1, s1; push one signed byte onto stack (expands to 32bit) */
#define SIPUSH 17 /* u1, s2; push signed short (2 byte) onto stack (expands to 32bit) */

#define LDC 18 /* u1, u1; push single-word constant onto stack */
#define LDC_W 19 /* u1, u2; push single-word constant onto stack (wide index) */
#define LDC2_W 20 /* u1, u2; push two-word constant onto stack (wide index) */

/* working with local variables */
#define ILOAD 21 /* u1, u1 (u1, u1, u2 using wide opcode); retrieve integer from local variable */
#define LLOAD 22 /* u1, u1 (u1, u1, u2 using wide opcode); retrieve long integer from local variable */
#define FLOAD 23 /* u1, u1 (u1, u1, u2 using wide opcode); retrieve float from local variable */
#define DLOAD 24 /* u1, u1 (u1, u1, u2 using wide opcode); retrieve double from local variable */
#define ALOAD 25 /* u1, u1 (u1, u1, u2 using wide opcode); retrieve object reference from local variable */

#define ILOAD_0 26 /* u1; retrieve integer from local variable 0 */
#define ILOAD_1 27 /* u1; retrieve integer from local variable 1 */
#define ILOAD_2 28 /* u1; retrieve integer from local variable 2 */
#define ILOAD_3 29 /* u1; retrieve integer from local variable 3 */

#define LLOAD_0 30 /* u1; retrieve long integer from local variable 0 */
#define LLOAD_1 31 /* u1; retrieve long integer from local variable 1 */
#define LLOAD_2 32 /* u1; retrieve long integer from local variable 2 */
#define LLOAD_3 33 /* u1; retrieve long integer from local variable 3 */

#define FLOAD_0 34 /* u1; retrieve float from local variable 0 */
#define FLOAD_1 35 /* u1; retrieve float from local variable 1 */
#define FLOAD_2 36 /* u1; retrieve float from local variable 2 */
#define FLOAD_3 37 /* u1; retrieve float from local variable 3 */

#define DLOAD_0 38 /* u1; retrieve double from local variable 0 */
#define DLOAD_1 39 /* u1; retrieve double from local variable 1 */
#define DLOAD_2 40 /* u1; retrieve double from local variable 2 */
#define DLOAD_3 41 /* u1; retrieve double from local variable 3 */

#define ALOAD_0 42 /* u1; retrieve object reference from local variable 0 */
#define ALOAD_1 43 /* u1; retrieve object reference from local variable 1 */
#define ALOAD_2 44 /* u1; retrieve object reference from local variable 2 */
#define ALOAD_3 45 /* u1; retrieve object reference from local variable 3 */

/* working with arrays */
#define IALOAD 46 /* u1; retrieve integer from array */
#define LALOAD 47 /* u1; retrieve long integer from array */
#define FALOAD 48 /* u1; retrieve float from array */
#define DALOAD 49 /* u1; retrieve double-precision float from array */
#define AALOAD 50 /* u1; retrieve obkject reference from array */
#define BALOAD 51 /* u1; retrieve byte/boolean from array */
#define CALOAD 52 /* u1; retrieve character from array */
#define SALOAD 53 /* u1; retrieve short from array */

/* working with local variables */
#define ISTORE 54 /* u1, u1 (u1, u1, u2 using wide opcode); store integer in local variable */
#define LSTORE 55 /* u1, u1 (u1, u1, u2 using wide opcode); store long integer in local variable */
#define FSTORE 56 /* u1, u1 (u1, u1, u2 using wide opcode); store float in local variable */
#define DSTORE 57 /* u1, u1 (u1, u1, u2 using wide opcode); store double in local variable */
#define ASTORE 58 /* u1, u1 (u1, u1, u2 if using wide opcode); store object reference in local variable */

#define ISTORE_0 59 /* u1; store integer in local variable 0 */
#define ISTORE_1 60 /* u1; store integer in local variable 1 */
#define ISTORE_2 61 /* u1; store integer in local variable 2 */
#define ISTORE_3 62 /* u1; store integer in local variable 3 */

#define LSTORE_0 63 /* u1; store long integer in local variable 0 */
#define LSTORE_1 64 /* u1; store long integer in local variable 1 */
#define LSTORE_2 65 /* u1; store long integer in local variable 2 */
#define LSTORE_3 66 /* u1; store long integer in local variable 3 */

#define FSTORE_0 67 /* u1; store float in local variable 0 */
#define FSTORE_1 68 /* u1; store float in local variable 1 */
#define FSTORE_2 69 /* u1; store float in local variable 2 */
#define FSTORE_3 70 /* u1; store float in local variable 3 */

#define DSTORE_0 71 /* u1; store double in local variable 0 */
#define DSTORE_1 72 /* u1; store double in local variable 1 */
#define DSTORE_2 73 /* u1; store double in local variable 2 */
#define DSTORE_3 74 /* u1; store double in local variable 3 */

#define ASTORE_0 75 /* u1; store object reference in local variable 0 */
#define ASTORE_1 76 /* u1; store object reference in local variable 1 */
#define ASTORE_2 77 /* u1; store object reference in local variable 2 */
#define ASTORE_3 78 /* u1; store object reference in local variable 3 */

/* working with arrays */
#define IASTORE 79 /* u1; store in integer array */
#define LASTORE 80 /* u1; store in long integer array */
#define FASTORE 81 /* u1; store in single-precision float array */
#define DASTORE 82 /* u1; store in double-precision float array */
#define AASTORE 83 /* u1; store object reference in array */
#define BASTORE 84 /* u1; store in byte/boolean */
#define CASTORE 85 /* u1; store in character array */
#define SASTORE 86 /* u1; store in short array */

/* stack managment */
#define POP 87 /* u1; discard top item on stack */
#define POP2 88 /* u1; discard top two items on stack */

#define DUP 89 /* u1; duplicate top single item on the stack */
#define DUP_X1 90 /* u1; duplicate top stack item and insert beneath second item */
#define DUP_X2 91 /* u1; duplicate top stack item and insert beneath third item */
#define DUP2 92 /* u1; duplicate top two stack items */
#define DUP2_X1 93 /* u1; duplicate two items and insert beneath third item */
#define DUP2_X2 94 /* u1; duplicate two items and insert beneath fourth item */

#define SWAP 95 /* u1; swap top two stack items */

/* arithmetic operators */
#define IADD 96 /* u1; add two integers */
#define LADD 97 /* u1; add two long integers */
#define FADD 98 /* u1; add two floats */
#define DADD 99 /* u1; add two doubles */

#define ISUB 100 /* u1; substract two integers */
#define LSUB 101 /* u1; substract two long integers */
#define FSUB 102 /* u1; substract two floats */
#define DSUB 103 /* u1; substract two doubles */

#define IMUL 104 /* u1; multiply two integers */
#define LMUL 105 /* u1; multiply two long integers */
#define FMUL 106 /* u1; multiply two floats */
#define DMUL 107 /* u1; multiply two doubles */

#define IDIV 108 /* u1; divides two integers */
#define LDIV 109 /* u1; divides two long integers */
#define FDIV 110 /* u1; divides two floats */
#define DDIV 111 /* u1; divides two doubles */

#define IREM 112 /* u1; remainder of two integers */
#define LREM 113 /* u1; remainder of two long integers */
#define FREM 114 /* u1; remainder of two floats */
#define DREM 115 /* u1; remainder of two doubles */

/* logical operators */
#define INEG 116 /* u1; negate a integer */
#define LNEG 117 /* u1; negate a long integer */
#define FNEG 118 /* u1; negate a float */
#define DNEG 119 /* u1; negate a double */

#define ISHL 120 /* u1; integer shift left */
#define LSHL 121 /* u1; long integer shift left */

#define ISHR 122 /* u1; integer arithmetic shift right */
#define LSHR 123 /* u1; long integer arithmetic shift right */

#define IUSHR 124 /* u1; integer logical shift right */
#define LUSHR 125 /* u1; long integer logical shift right */

#define IAND 126 /* u1; integer bitwise and */
#define LAND 127 /* u1; long integer bitwise and */

#define IOR 128 /* u1; integer bitwise or */
#define LOR 129 /* u1; long integer bitwise or */

#define IXOR 130 /* u1; integer bitwise exclusive or */
#define LXOR 131 /* u1; long integer bitwise exclusive or */

/* arithmetic again */
#define IINC 132 /* u1, u1, s1 (u1, u1, u2, s2 using wide opcode); increment integer in local variable */

/* converting */
#define I2L 133 /* u1; convert integer to long ineteger */
#define I2F 134 /* u1; convert integer to float */
#define I2D 135 /* u1; convert integer to double */

#define L2I 136 /* u1; convert long integer to integer */
#define L2F 137 /* u1; convert long integer to float */
#define L2D 138 /* u1; convert long integer to double */

#define F2I 139 /* u1; convert float to integer */
#define F2L 140 /* u1; convert float to long integer */
#define F2D 141 /* u1; convert float to double */

#define D2I 142 /* u1; convert double to integer */
#define D2L 143 /* u1; convert double to long integer */
#define D2F 144 /* u1; convert double to float */

#define I2B 145 /* u1; convert integer to byte */
#define I2C 146 /* u1; convert integer to char */
#define I2S 147 /* u1; convert integer to string(?) */

/* comparison */
#define LCMP 148 /* u1; long integer comparison */
#define FCMPL 149 /* u1; single precision float comparison (-1 on NaN) */
#define FCMPG 150 /* u1; single precision float comparison (1 on NaN) */
#define DCMPL 151 /* u1; comapre two doubles (-1 on NaN) */
#define DCMPG 152 /* u1; compare two doubles (1 on NaN) */

/* conditional branching (jumps) */
#define IFEQ 153 /* u1, s2; jump if zero */
#define IFNE 154 /* u1, s2; jump if non zero */
#define IFLT 155 /* u1, s2; jump if less than zero */
#define IFGE 156 /* u1, s2; jump if greater than or equal to zero */
#define IFGT 157 /* u1, s2; jump if greater than zero */
#define IFLE 158 /* u1, s2; jump if less than or equal to zero */

#define IF_ICMPEQ 159 /* u1, s2; jump if two integers are equal */
#define IF_ICMPNE 160 /* u1, s2; jump if two integers are not equal */
#define IF_ICMPLT 161 /* u1, s2; jump if one integer is less than another */
#define IF_ICMPGE 162 /* u1, s2; jump if one integer is greater than or equal to another */
#define IF_ICMPGT 163 /* u1, s2; jump if one integer is greater than another */
#define IF_ICMPLE 164 /* u1, s2; jump if one integer is less than or equal to another */

#define IF_ACMPEQ 165 /* u1, s2; jump if two object references are equal */
#define IF_ACMPNE 166 /* u1, s2; jump if two object references are not equal */

/* non-conditional branching (simple jumps) */
#define GOTO 167 /* u1, s2; branch to address */
#define JSR 168 /* u1, s2; jump subroutine */
#define RET 169 /* u1, u1 (u1, u1, u2 using wide opcode); return from subroutine */

/* switch statements */
#define TABLESWITCH 170 /* u1, ...; jump according to a table */
#define LOOKUPSWITCH 171 /* u1, s4, s4, ...; match key in table and jump */

/* return from a method */
#define IRETURN 172 /* u1; return from method with integer result */
#define LRETURN 173 /* u1; return from method with long integer result */
#define FRETURN 174 /* u1; return from method with float result */
#define DRETURN 175 /* u1; return from method with double result */
#define ARETURN 176 /* u1; return from method with object reference result */
#define RETURN 177 /* u1; return from a method */

/* get/set static field */
#define GETSTATIC 178 /* u1, u2; get value of static field */
#define PUTSTATIC 179 /* u1, u2; set value of static field */

/* get/set object field */
#define GETFIELD 180 /* u1, u2; get value of object field */
#define PUTFIELD 181 /* u1, u2; set value of object field */

/* method calls */
#define INVOKEVIRTUAL 182 /* u1, u2; call an instance method */
#define INVOKESPECIAL 183 /* u1, u2; invoke method belonging to a specific class */
#define INVOKESTATIC 184 /* u1, u2; invoke a static method */
#define INVOKEINTERFACE 185 /* u1, u2, u1, u1; invoke an interface method */

#define XXX_UNUSED_XXX 186 /* unused */

/* object/array creation and length */
#define NEW 187 /* u1, u2; create an object */

#define NEWARRAY 188 /* u1, u1; allocate new array for numbers or booleans */
#define ANEWARRAY 189 /* u1, u2; allocate new array for objects */
#define ARRAYLENGTH 190 /* u1; get length of array */

/* exception mechanisnm */
#define ATHROW 191 /* u1; throw an exception error */
#define CHECKCAST 192 /* u1, u2; ensure object or array belongs to type */
#define INSTANCEOF 193 /* u1, u2; negate an integer */

/* monitors */
#define MONITORENTER 194 /* u1; enter synchronized region of code */
#define MONITOREXIT 195 /* u1; leave synchronized region of code */

#define WIDE 196 /* u1; next instruction uses 16bit index */

/* array creation again */
#define MULTIANEWARRAY 197 /* u1, u2, u1; allocate multi-dimensional array */

/* some branches again */
#define IFNULL 198 /* u1, s2; jump if null */
#define IFNONNULL 199 /* u1, s2; jump if non null */

#define GOTO_W 200 /* u1, s4; branch to address using wide offset */
#define JSR_W 201 /* u1, s4; jump to subroutine using wide offset */

#define BREAKPOINT 202 /* RESEVED; must not appear in class file (fails verification) */

/* quick opcodes (may only be internally used by VM) */
#define LDC_QUICK 203 
#define UNKNOWN1 204 /* UNKNOWN */
#define LDC2_W_QUICK 205 
#define GETFIELD_QUICK 206 
#define PUTFIELD_QUICK 207 
#define GETFIELD2_QUICK 208 
#define PUTFIELD2_QUICK 209 
#define UNKNOWN2 210 /* UNKNOWN */
#define PUTSTATIC_QUICK 211 
#define GETSTATIC2_QUICK 212
#define PUTSTATIC2_QUICK 213
#define INVOKEVIRTUAL_QUICK 214
#define INVOKENONVIRTUAL_QUICK 215
#define INVOKESUPER_QUICK 216
#define INVOKESTATIC_QUICK 217
#define INVOKEINTERFACE_QUICK 218
#define INVOKEVIRTUALOBJECT_QUICK 219
#define UNKNOWN3 220 /* UNKNOWN */
#define NEW_QUICK 221
#define ANEWARRAY_QUICK 222
#define MULTIANEWARRAY_QUICK 223
#define CHECKCAST_QUICK 224
#define INSTANCEOF_QUICK 225
#define INVOKEVIRTUAL_QUICK_W 226
#define GETFIELD_QUICK_W 227
#define PUTFIELD_QUICK_W 228

/* unused opcodes */
#define UNUSED1 229
#define UNUSED2 230
#define UNUSED3 231
#define UNUSED4 232
#define UNUSED5 233
#define UNUSED6 234
#define UNUSED7 235
#define UNUSED8 236
#define UNUSED9 237
#define UNUSED10 238
#define UNUSED11 239
#define UNUSED12 240
#define UNUSED13 241
#define UNUSED14 242
#define UNUSED15 243
#define UNUSED16 244
#define UNUSED17 245
#define UNUSED18 246
#define UNUSED19 247
#define UNUSED20 248
#define UNUSED21 249
#define UNUSED22 250
#define UNUSED23 251
#define UNUSED24 252
#define UNUSED25 253

#define IMPDEP1 254 /* RESERVED: implementation depedant 1; must not appear in class file */
#define IMPDEP2 255 /* RESERVED: implementation depedant 2; must not appear in class file */

char* opcodeNames[]= {"NOP", "ACONST_NULL", "ICONST_M1", "ICONST_0", "ICONST_1", "ICONST_2", "ICONST_3", "ICONST_4", "ICONST_5", "LCONST_0", "LCONST_1", "FCONST_0",
	"FCONST_1", "FCONST_2", "DCONST_0", "DCONST_1", "BIPUSH", "SIPUSH", "LDC", "LDC_W", "LDC2_W", "ILOAD", "LLOAD", "FLOAD", "DLOAD", "ALOAD", "ILOAD_0", "ILOAD_1",
	"ILOAD_2", "ILOAD_3", "LLOAD_0", "LLOAD_1", "LLOAD_2", "LLOAD_3", "FLOAD_0", "FLOAD_1", "FLOAD_2", "FLOAD_3", "DLOAD_0", "DLOAD_1", "DLOAD_2", "DLOAD_3", "ALOAD_0",
	"ALOAD_1", "ALOAD_2", "ALOAD_3", "IALOAD", "LALOAD", "FALOAD", "DALOAD", "AALOAD", "BALOAD", "CALOAD", "SALOAD", "ISTORE", "LSTORE", "FSTORE", "DSTORE", "ASTORE",
	"ISTORE_0", "ISTORE_1", "ISTORE_2", "ISTORE_3", "LSTORE_0", "LSTORE_1", "LSTORE_2", "LSTORE_3", "FSTORE_0", "FSTORE_1", "FSTORE_2", "FSTORE_3", "DSTORE_0",
	"DSTORE_1", "DSTORE_2", "DSTORE_3", "ASTORE_0", "ASTORE_1", "ASTORE_2", "ASTORE_3", "IASTORE", "LASTORE", "FASTORE", "DASTORE", "AASTORE", "BASTORE", "CASTORE",
	"SASTORE", "POP", "POP2", "DUP", "DUP_X1", "DUP_X2", "DUP2", "DUP2_X1", "DUP2_X2", "SWAP", "IADD", "LADD", "FADD", "DADD", "ISUB", "LSUB", "FSUB", "DSUB", "IMUL",
	"LMUL", "FMUL", "DMUL", "IDIV", "LDIV", "FDIV", "DDIV", "IREM", "LREM", "FREM", "DREM", "INEG", "LNEG", "FNEG", "DNEG", "ISHL", "LSHL", "ISHR", "LSHR", "IUSHR", 
	"LUSHR",	"IAND", "LAND", "IOR", "LOR", "IXOR", "LXOR", "IINC", "I2L", "I2F", "I2D", "L2I", "L2F", "L2D", "F2I", "F2L", "F2D", "D2I", "D2L", "D2F", "I2B", "I2C", 
	"I2S", "LCMP", "FCMPL", "FCMPG", "DCMPL", "DCMPG", "IFEQ", "IFNE", "IFLT", "IFGE", "IFGT", "IFLE", "IF_ICMPEQ", "IF_ICMPNE", "IF_ICMPLT", "IF_ICMPGE", "IF_ICMPGT", 
	"IF_ICMPLE", "IF_ACMPEQ", "IF_ACMPNE", "GOTO", "JSR", "RET", "TABLESWITCH", "LOOKUPSWITCH", "IRETURN", "LRETURN", "FRETURN", "DRETURN", "ARETURN", "RETURN", 
	"GETSTATIC", "PUTSTATIC", "GETFIELD", "PUTFIELD", "INVOKEVIRTUAL", "INVOKESPECIAL", "INVOKESTATIC", "INVOKEINTERFACE", "XXX_UNUSED_XXX", "NEW", "NEWARRAY", 
	"ANEWARRAY", "ARRAYLENGTH", "ATHROW", "CHECKCAST", "INSTANCEOF", "MONITORENTER", "MONITOREXIT", "WIDE", "MULTIANEWARRAY", "IFNULL", "IFNONNULL", "GOTO_W", "JSR_W", 
	"BREAKPOINT", "LDC_QUICK", "UNKNOWN1", "LDC2_W_QUICK", "GETFIELD_QUICK", "PUTFIELD_QUICK", "GETFIELD2_QUICK", "PUTFIELD2_QUICK", "UNKNOWN2", "PUTSTATIC_QUICK",  
	"GETSTATIC2_QUICK", "PUTSTATIC2_QUICK", "INVOKEVIRTUAL_QUICK", "INVOKENONVIRTUAL_QUICK", "INVOKESUPER_QUICK", "INVOKESTATIC_QUICK", "INVOKEINTERFACE_QUICK", 
	"INVOKEVIRTUALOBJECT_QUICK", "UNKNOWN3", "NEW_QUICK", "ANEWARRAY_QUICK", "MULTIANEWARRAY_QUICK", "CHECKCAST_QUICK", "INSTANCEOF_QUICK", "INVOKEVIRTUAL_QUICK_W", 
	"GETFIELD_QUICK_W", "PUTFIELD_QUICK_W", "UNUSED1", "UNUSED2", "UNUSED3", "UNUSED4", "UNUSED5", "UNUSED6", "UNUSED7", "UNUSED8", "UNUSED9", "UNUSED10", "UNUSED11", 
	"UNUSED12", "UNUSED13", "UNUSED14", "UNUSED15", "UNUSED16", "UNUSED17", "UNUSED18", "UNUSED19", "UNUSED20", "UNUSED21", "UNUSED22", "UNUSED23", "UNUSED24", 
	"UNUSED25", "IMPDEP1", "IMPDEP2"};

#endif /*_opcodes_h_*/
