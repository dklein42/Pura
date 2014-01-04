/*
 *  class.h
 *  Pura
 *
 *  Created by Daniel Klein on 06.11.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _class_h_
#define _class_h_

#include "fileClassLoader.h"

/* class constants */
#define MAGIC_NUMBER 0xCAFEBABE

/* constant pool tags */
#define CONSTANT_Utf8		1
#define CONSTANT_Integer	3
#define CONSTANT_Float		4
#define CONSTANT_Long		5
#define CONSTANT_Double		6
#define CONSTANT_Class		7
#define CONSTANT_String		8
#define CONSTANT_Fieldref	9
#define CONSTANT_Methodref	10
#define CONSTANT_InterfaceMethodref	11
#define CONSTANT_NameAndType		12

/* access flags */
#define ACC_PUBLIC		0x0001
#define ACC_PRIVATE		0x0002
#define ACC_PROTECTED	0x0004
#define ACC_STATIC		0x0008
#define ACC_FINAL		0x0010
#define ACC_SUPER		0x0020
#define ACC_VOLATILE	0x0040
#define ACC_TRANSIENT	0x0080
#define ACC_NATIVE		0x0100
#define ACC_INTERFACE	0x0200
#define ACC_ABSTRACT	0x0400
#define ACC_STRICT		0x0800

/* base types */
#define BASE_TYPE_BYTE      'B'
#define BASE_TYPE_CHAR      'C'
#define BASE_TYPE_DOUBLE    'D'
#define BASE_TYPE_FLOAT     'F'
#define BASE_TYPE_INT       'I'
#define BASE_TYPE_LONG      'J'
#define BASE_TYPE_REFERENCE 'L'
#define BASE_TYPE_SHORT     'S'
#define BASE_TYPE_BOOLEAN   'Z'
#define BASE_TYPE_ONE_ARRAY_DIMENSION '['

/* umbrella struct for constant pool entries */
typedef struct scp_info
{
	u1 tag;
	void* info;
} cp_info;

/* do we need this one? */
typedef struct sattribute_info
{
	u2 attribute_name_index;
	u4 attribute_length;
	u1* info;
} attribute_info;

typedef struct sfield_info
{
	u2 access_flags;
	u2 name_index; /* TODO: Optimize! */
	u2 descriptor_index;
	/*u2 attributes_count;*/
	/*attribute_info** attributes;*/
	u2 constantValueIndex;
} field_info;

typedef struct sexception_table
{
	u2 start_pc;
	u2 end_pc;
	u2 handler_pc;
	u2 catch_type;
} exception_table;

typedef struct sCode_attribute
{
	/*u2 attribute_name_index;*/
	/*u4 attribute_length;*/
	u2 max_stack;
	u2 max_locals;
	u4 code_length;
	u1* code;
	u2 exception_table_length;
	exception_table** exception_table_tab;
	u2 attributes_count;
	attribute_info* attributes;
} Code_attribute;

typedef struct smethod_info
{
	u2 access_flags;
	/*u2 name_index;
	u2 descriptor_index;*/
	/*u2 attributes_count;*/
	/*attribute_info** attributes;*/
	Code_attribute* code;
	uint8 parameterSlotCount; /* rt info */
	char* name;
	char* descriptor;
} method_info;

/* do we have to use this? */
typedef struct sclasses
{
	u2 inner_class_info_index;
	u2 outer_class_info_index;
	u2 inner_name_index;
	u2 inner_class_access_flags;
} classes;

/* -> we do not use debug structures (i.e. line_number_table and local_variable_table)
typedef struct sline_number_table
{
	u2 start_pc;
	u2 line_number;
} line_number_table;

typedef struct slocal_variable_table
{
	u2 start_pc;
	u2 length;
	u2 name_index;
	u2 descriptor_index;
	u2 index;
} local_variable_table;
*/


typedef struct sCONSTANT_Class_info
{
	u1 tag;
	u2 name_index; /* TODO: Optimize! */
} CONSTANT_Class_info;

typedef struct sCONSTANT_Fieldref_info
{
	u1 tag;
	u2 class_index; /* TODO: Optimize! */
	u2 name_and_type_index;
} CONSTANT_Fieldref_info;

typedef struct sCONSTANT_Methodref_info
{
	u1 tag;
	u2 class_index; /* TODO: Optimize! */
	u2 name_and_type_index;
} CONSTANT_Methodref_info;

typedef struct sCONSTANT_InterfaceMethodref_info
{
	u1 tag;
	u2 class_index; /* TODO: Optimize! */
	u2 name_and_type_index;
} CONSTANT_InterfaceMethodref_info;

typedef struct sCONSTANT_String_info
{
	u1 tag;
	u2 string_index; /* TODO: Optimize! */
	reference stringRef; /* rt */
} CONSTANT_String_info;

typedef struct sCONSTANT_Integer_info
{
	u1 tag;
	u4 bytes;
} CONSTANT_Integer_info;

typedef struct sCONSTANT_Float_info
{
	u1 tag;
	u4 bytes;
} CONSTANT_Float_info;

typedef struct sCONSTANT_Long_info
{
	u1 tag;
	u4 high_bytes;
	u4 low_bytes;
} CONSTANT_Long_info;

typedef struct sCONSTANT_Double_info
{
	u1 tag;
	u4 high_bytes;
	u4 low_bytes;
} CONSTANT_Double_info;

typedef struct sCONSTANT_NameAndType_info
{
	u1 tag;
	u2 name_index; /* TODO: Optimize! */
	u2 descriptor_index;
} CONSTANT_NameAndType_info;

typedef struct sCONSTANT_Utf8_info
{
	u1 tag;
	u2 length;
	u1* bytes;
} CONSTANT_Utf8_info;

/* no struct required for runtime storage, we store a u2 instead
typedef struct sConstantValue_attribute
{
	u2 attribute_name_index;
	u4 attribute_length;
	u2 constantvalue_index;
} ConstantValue_attribute;
*/

/* for what exactly is this for? */
typedef struct sExceptions_attribute
{
	u2 attribute_name_index;
	u4 attribute_length;
	u2 number_of_exceptions;
	u2* exception_index_table;
} Exceptions_attribute;

/* do we have to use this? */
typedef struct sInnerClasses_attribute
{
	u2 attribute_name_index;
	u4 attribute_length;
	u2 number_of_classes;
	classes* classes_table;
} InnerClasses_attribute;

/* unused
typedef struct sSynthetic_attribute
{
	u2 attribute_name_index;
	u4 attribute_length; /* must be zero */
/*} Synthetic_attribute;
*/

/* unused
typedef struct sSourceFile_attribute
{
	u2 attribute_name_index;
	u4 attribute_length;
	u2 sourcefile_index;
} SourceFile_attribute;
*/

/* debug info structures unused for now
typedef struct sLineNumberTable_attribute
{
	u2 attribute_name_index;
	u4 attribute_length;
	u2 line_number_table_length;
	line_number_table* line_number_table_tab;
} LineNumberTable_attribute;

typedef struct sLocalVariableTable_attribute
{
	u2 attribute_name_index;
	u4 attribute_length;
	u2 local_variable_table_length;
	local_variable_table* local_variable_table_tab;
} LocalVariableTable_attribute;
*/

/* unused, we don't want to complain about deprecated stuff
typedef struct sDeprecated_attribute
{
	u2 attribute_name_index;
	u4 attribute_length; *//* must be zero */
/*} Deprecated_attribute;*/

/* runtime structure */  /* TODO: Optimize? */
typedef struct sVariable
{
	char* name;
	char* descriptor;
	u2 access_flags;
	/* field_info* info; */
	uint32 slot_index;
} variable;

/* main class structure */
typedef struct sClass 
{
	/*u4 magic;*/
	/* u2 minor_version; */
	/* u2 major_version; */
	u2 constant_pool_count;
	cp_info** constant_pool; /* index 0 is always empty! */
	u2 access_flags;
	/* u2 this_class; */
	/* u2 super_class; */
	u2 interfaces_count;
	u2* interfaces;
	/* u2 fields_count; */
	/* field_info** fields; */
	u2 methods_count;
	method_info** methods;
	
	/* additional runtime data */
	u2 class_instance_variable_count;
	variable** class_instance_variable_table;
	
	u2 class_instance_variable_slot_count;
	int32* class_inctance_variable_slots;

	u2 instance_variable_count;
	variable** instance_variable_table;
	u2 instance_variable_slot_count;

	boolean isInitialized;
	const char* className;
	struct sClass* superClass;
	const char* sourceFileName;
} Class;

/* function declarations */
void cls_load( Class* cls, ClassLoaderState* cl );
void cls_initArrayClass( Class* cls, const char* type );

char* cls_resolveConstantPoolIndexToUtf8( Class* cls, int index );
char* cls_resolveConstantPoolIndexToClassName( Class* cls, int index );
Class* cls_resolveConstantPoolIndexToClass( Class* cls, int index );
method_info* cls_resolveMethod( Class** cls, const char* name, const char* descriptor );
method_info* cls_getMethod( Class* cls, const char* name, const char* descriptor );
variable* cls_resolveStaticField( Class** cls, const char* name, const char* descriptor );
variable* cls_resolveField( Class** cls, const char* name, const char* descriptor );

int32 cls_getItemFromConstantPool( Class* cls, int index );
uint64 cls_getWideItemFromConstantPool( Class* cls, int index );
CONSTANT_NameAndType_info* cls_resolveConstantPoolIndexToNameAndType( Class* cls, uint16 index );
void cls_resolveConstantPoolIndexToClassAndMethodInfo( Class* cls, uint16 index, Class** otherClass, method_info** methodInfo );
void cls_resolveConstantPoolIndexToClassAndInterfaceMethodInfo( Class* cls, uint16 index, Class** otherClass, method_info** methodInfo );
void cls_resolveConstantPoolIndexToClassAndVariableInfo( Class* cls, uint16 index, Class** otherClass, variable** variableInfo, boolean isStaticField );
void cls_resolveConstantPoolIndexOfMethodRefToMethodNameAndDescriptor( Class* cls, uint16 index, char** className, char** methodName, char** methodDescriptor );

boolean cls_implementsInterface( Class* cls, Class* interf );

boolean isFlagSet( u2 flags, u2 flag );

#endif /*_class_h_*/
