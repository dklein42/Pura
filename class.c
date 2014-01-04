/*
 *  class.c
 *  Class structures, management, resolution and loading.
 *
 *  Copyright (c) 2006, 2007 Daniel Klein. All rights reserved.
 *  
 *  Licensed under GPL version 2.
 *  See here for the full license: http://www.gnu.org/licenses/gpl.html
 */

#include <string.h>
#include "puraGlobals.h"
#include "fileClassLoader.h"
#include "methodArea.h"
#include "interpreter.h"
#include "memoryManager.h"
#include "heap.h"
#include "class.h"

/**********************************************************************************************
 * Constant Pool handling
 **********************************************************************************************/

void addToConstantPool( Class* cls, cp_info* data, int constantPoolPos )
{
	cp_info** cp= cls->constant_pool;
	cp+= constantPoolPos;
	*cp= data;
}

void readMethodRefInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Methodref_info* ref= mm_staticMalloc( sizeof(CONSTANT_Methodref_info) );
	ref->tag= cl_readU1( cl );
	ref->class_index= cl_readU2( cl );
	ref->name_and_type_index= cl_readU2( cl );
	ref->class= NULL; /* rt pointers, will be initialized later */
	ref->methodInfo= NULL;

	logVerbose( "<#%i, #%i>\n", ref->class_index, ref->name_and_type_index );

	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readFieldRefInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Fieldref_info* ref= mm_staticMalloc( sizeof(CONSTANT_Fieldref_info) );
	ref->tag= cl_readU1( cl );
	ref->class_index= cl_readU2( cl );
	ref->name_and_type_index= cl_readU2( cl );
	ref->class= NULL; /* rt pointers, will be initialized later */
	ref->variableInfo= NULL;
	
	logVerbose( "<#%i, #%i>\n", ref->class_index, ref->name_and_type_index );

	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readInterfaceMethodRefInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_InterfaceMethodref_info* ref= mm_staticMalloc( sizeof(CONSTANT_InterfaceMethodref_info) );
	ref->tag= cl_readU1( cl );
	ref->class_index= cl_readU2( cl );
	ref->name_and_type_index= cl_readU2( cl );
	ref->class= NULL; /* rt pointers, will be initialized later */
	ref->methodInfo= NULL;
	
	logVerbose( "<#%i, #%i>\n", ref->class_index, ref->name_and_type_index );

	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readStringInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_String_info* ref= mm_staticMalloc( sizeof(CONSTANT_String_info) );
	ref->tag= cl_readU1( cl );
	ref->string_index= cl_readU2( cl );
	ref->stringRef= NULL_REFERENCE; /* stores according String instance after resolution */ 
	
	logVerbose( "<#%i>\n", ref->string_index );

	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readClassInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Class_info* ref= mm_staticMalloc( sizeof(CONSTANT_Class_info) );
	ref->tag= cl_readU1( cl );
	ref->name_index= cl_readU2( cl );
	ref->class= NULL; /* rt pointer */
	
	logVerbose( "<#%i>\n", ref->name_index );

	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readConstantUtf8Info( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Utf8_info* ref= mm_staticMalloc( sizeof(CONSTANT_Utf8_info) );
	ref->tag= cl_readU1( cl );
	ref->length= cl_readU2( cl );
	
	ref->bytes= mm_staticMalloc( ref->length+1 );
	cl_readBytes( cl, ref->length, ref->bytes );
	
	/* add terminating null-byte */
	byte* data= ref->bytes;
	data+= ref->length;
	*data= '\0';
	
	logVerbose( "\"%s\"\n", (char*)ref->bytes );
	
	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readConstantNameAndTypeInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_NameAndType_info* ref= mm_staticMalloc( sizeof(CONSTANT_NameAndType_info) );
	ref->tag= cl_readU1( cl );
	ref->name_index= cl_readU2( cl );
	ref->descriptor_index= cl_readU2( cl );
	
	logVerbose( "<#%i, #%i>\n", ref->name_index, ref->descriptor_index );
	
	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readConstantLongInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Long_info* ref= mm_staticMalloc( sizeof(CONSTANT_Long_info) );
	ref->tag= cl_readU1( cl );
	ref->high_bytes= cl_readU4( cl );
	ref->low_bytes= cl_readU4( cl );

	logVerbose( "(high: %X low: %X) -> ", ref->high_bytes, ref->low_bytes );

	uint64 number= ((uint64)ref->high_bytes) << 32;
	number|= ref->low_bytes;
	logVerbose( "%llX\n", number );
	
	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readConstantDoubleInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Double_info* ref= mm_staticMalloc( sizeof(CONSTANT_Double_info) );
	ref->tag= cl_readU1( cl );
	ref->high_bytes= cl_readU4( cl );
	ref->low_bytes= cl_readU4( cl );

	logVerbose( "(high: %X low: %X) -> ", ref->high_bytes, ref->low_bytes );

	uint64 number= ((uint64)ref->high_bytes) << 32;
	number|= ref->low_bytes;
	double n2= *(double*)&number;
	logVerbose( "%f\n", number, n2 );
	
	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readConstantFloatInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Float_info* ref= mm_staticMalloc( sizeof(CONSTANT_Float_info) );
	ref->tag= cl_readU1( cl );
	ref->bytes= cl_readU4( cl );

	logVerbose( "(raw: %X) -> ", ref->bytes );
	float number= *(float*)&ref->bytes;
	logVerbose( "%f\n", number, number );
	
	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readConstantIntegerInfo( Class* cls, ClassLoaderState* cl, int constantPoolPos )
{
	CONSTANT_Integer_info* ref= mm_staticMalloc( sizeof(CONSTANT_Integer_info) );
	ref->tag= cl_readU1( cl );
	ref->bytes= cl_readU4( cl );

	logVerbose( "%X\n", ref->bytes );
	
	addToConstantPool( cls, (cp_info*)ref, constantPoolPos );
}

void readConstantPoolEntries( Class* cls, ClassLoaderState* cl )
{
	int i;
	for( i= 1; i < cls->constant_pool_count; i++ )
	{
		switch( cl_peekNextU1( cl ) )
		{
		case CONSTANT_Utf8:
			logVerbose( "%i: CONSTANT_Utf8 ", i );
			readConstantUtf8Info( cls, cl, i );			
			break;
		case CONSTANT_Integer:
			logVerbose( "%i: CONSTANT_Integer ", i );
			readConstantIntegerInfo( cls, cl, i );
			break;
		case CONSTANT_Float:
			logVerbose( "%i: CONSTANT_Float ", i );
			readConstantFloatInfo( cls, cl, i );
			break;
		case CONSTANT_Long:
			logVerbose( "%i: CONSTANT_Long ", i );
			readConstantLongInfo( cls, cl, i );
			/* use two slots! -> skip one */
			i++;
			break;
		case CONSTANT_Double:
			logVerbose( "%i: CONSTANT_Double ", i );
			readConstantDoubleInfo( cls, cl, i );
			/* use two slots! -> skip one */
			i++;
			break;
		case CONSTANT_Class:
			logVerbose( "%i: CONSTANT_Class ", i );
			readClassInfo( cls, cl, i );
			break;
		case CONSTANT_String:
			logVerbose( "%i: CONSTANT_String ", i );
			readStringInfo( cls, cl, i );
			break;
		case CONSTANT_Fieldref:
			logVerbose( "%i: CONSTANT_Fieldref ", i );
			readFieldRefInfo( cls, cl, i );
			break;
		case CONSTANT_Methodref:
			logVerbose( "%i: CONSTANT_Methodref ", i );
			readMethodRefInfo( cls, cl, i );
			break;
		case CONSTANT_InterfaceMethodref:
			logVerbose( "%i: CONSTANT_InterfaceMethodref ", i );
			readInterfaceMethodRefInfo( cls, cl, i );
			break;
		case CONSTANT_NameAndType:
			logVerbose( "%i: CONSTANT_NameAndType ", i );
			readConstantNameAndTypeInfo( cls, cl, i );
			break;
		default:
			logVerbose( "Unrecognized constant pool entry: %i\n", cl_peekNextU1(cl) );
			error( "Unknown constant pool type!\n" );
			break;
		}
	}
}

char* cls_resolveConstantPoolIndexToUtf8( Class* cls, int index )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);

	if( cpEntry->tag != CONSTANT_Utf8 )
		error( "Tried to resolve a UTF8 string from constant pool, but entry wasn't a UTF8 string." );

	CONSTANT_Utf8_info* utf= (CONSTANT_Utf8_info*)cpEntry;
	return (char*)utf->bytes;
}

char* cls_resolveConstantPoolIndexToClassName( Class* cls, int index )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	if( cpEntry->tag != CONSTANT_Class )
		error( "Tried to resolve a CONSTANT_Class_info from constant pool, but entry wasn't of the correct type." );
	
	CONSTANT_Class_info* info= (CONSTANT_Class_info*)cpEntry;
	return cls_resolveConstantPoolIndexToUtf8( cls, info->name_index );
}

Class* cls_resolveConstantPoolIndexToClass( Class* cls, int index )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	if( cpEntry->tag != CONSTANT_Class )
		error( "Tried to resolve a CONSTANT_Class_info from constant pool, but entry wasn't of the correct type." );
	
	CONSTANT_Class_info* info= (CONSTANT_Class_info*)cpEntry;
	
	if( info->class == NULL )
		info->class= ma_getClass( cls_resolveConstantPoolIndexToUtf8(cls, info->name_index) );
	
	return info->class;
}

/* Loads int or float values (32 bit, one slot) from constant pool entries. */
int32 cls_getItemFromConstantPool( Class* cls, int index )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	/* int */
	if( cpEntry->tag == CONSTANT_Integer )
	{
		CONSTANT_Integer_info* intInfo= (CONSTANT_Integer_info*)cpEntry;
		return intInfo->bytes;
	}
	/* float */
	else if( cpEntry->tag == CONSTANT_Float )
	{
		CONSTANT_Float_info* floatInfo= (CONSTANT_Float_info*)cpEntry;
		return floatInfo->bytes;
	}
	/* String */
	else if( cpEntry->tag == CONSTANT_String )
	{
		CONSTANT_String_info* strInfo= (CONSTANT_String_info*)cpEntry;
		
		/* See if it has already been resolved before. */
		if( strInfo->stringRef != NULL_REFERENCE )
			return strInfo->stringRef;
		
		/* Otherwise create new String instance here, remember it and return. */
		strInfo->stringRef= heap_newStringInstance( cls_resolveConstantPoolIndexToUtf8(cls, strInfo->string_index) );
		logVerbose( "\tCreating new String instance with text \"%s\", reference is %i.\n", cls_resolveConstantPoolIndexToUtf8(cls, strInfo->string_index), strInfo->stringRef );
		return strInfo->stringRef;
	}
	
	/* No match yet? Error! */
	error( "Tried to resolve index of the constant pool, but no acceptable type has been found there." );
	return 0; /* never reached */ 
}

uint64 cls_getWideItemFromConstantPool( Class* cls, int index )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	/* long */
	if( cpEntry->tag == CONSTANT_Long )
	{
		CONSTANT_Long_info* longInfo= (CONSTANT_Long_info*)cpEntry;
		return (((uint64)longInfo->high_bytes) << 32) | longInfo->low_bytes;
	}
	/* double */
	else if( cpEntry->tag == CONSTANT_Double )
	{
		CONSTANT_Double_info* doubleInfo= (CONSTANT_Double_info*)cpEntry;
		return (((uint64)doubleInfo->high_bytes) << 32) | doubleInfo->low_bytes;
	}
		
	/* No match yet? Error! */
	error( "Tried to resolve index of the constant pool, but no acceptable type has been found there." );
	return 0; /* never reached */ 
}

/**********************************************************************************************
 * Fields
 **********************************************************************************************/

boolean isFlagSet( u2 flags, u2 flag )
{
	return (flags & flag) ? true : false;
}

void readFields( Class* cls, ClassLoaderState* cl )
{
	u2 fieldsCount= cl_readU2( cl );
	field_info* fields= (field_info*)mm_staticMalloc( fieldsCount * sizeof(field_info) );
	
	logVerbose( "Fields: %i\n", fieldsCount );
	
	cls->class_instance_variable_count= 0;
	cls->instance_variable_count= 0;
	int i;
	for( i= 0; i < fieldsCount; i++ )
	{
		field_info* field= &fields[i];
		field->access_flags= cl_readU2( cl );
		field->name_index= cl_readU2( cl );
		field->descriptor_index= cl_readU2( cl );
		
		/* attributes of a field */
		field->constantValueIndex= 0; /* pre-init, maybe  unused */
		u2 attributes_count= cl_readU2( cl );

		logVerbose( "Field: <#%i, #%i> attributes: %i\n", field->name_index, field->descriptor_index, attributes_count );

		if( attributes_count > 0 )
		{
			int j;
			for( j= 0; j < attributes_count; j++ )
			{
				u2 attribute_name_index= cl_readU2( cl );
				const char* attributeName= cls_resolveConstantPoolIndexToUtf8( cls, attribute_name_index );
			
				logVerbose( "--> Attribute: <#%i>\n", attribute_name_index );

				/* read the "ConstantValue" attribute, if present */
				if( strcmp(attributeName, "ConstantValue") == 0 )
				{
					/* must not be initialized -> only one ConstantValue attribute allowed */
					if( field->constantValueIndex != 0 )
						error( "Two ConstantValue attributes for one field are not allowed!\n" );
					
					/* attribute length must be two bytes (i.e. u2) */ 
					if( cl_readU4(cl) != 2 )
						error( "Malformed ConstantValue attribute.\n" );
						
					/* remember constant value */	
					field->constantValueIndex= cl_readU2( cl );
					continue;
				}
				
				/* add other attributes here */
				
				/* skip unknown attributes */
				u4 attributeLength= cl_readU4( cl );
				cl_skipBytes( cl, attributeLength );			
			}
		}
		
		/* count class instance and instance variables */
		if( isFlagSet(field->access_flags, ACC_STATIC) )
			cls->class_instance_variable_count++;
		else
			cls->instance_variable_count++;
	}
	
	/* allocate class instance variable table (for faster lookup) and assign memory slots for the storage of the variable data */
	logVerbose( "Creating class instance variable table with %i entries.\n", cls->class_instance_variable_count );
	logVerbose( "Creating instance variable table with %i entries.\n", cls->instance_variable_count );

	int currentClassInstanceTableIndex= 0;
	int currentInstanceTableIndex= 0;
	cls->class_instance_variable_slot_count= 0;
	cls->instance_variable_slot_count= 0;
	
	cls->class_instance_variable_table= mm_staticMalloc( sizeof(variable*) * cls->class_instance_variable_count );
	cls->instance_variable_table= mm_staticMalloc( sizeof(variable*) * cls->instance_variable_count );
	
	int k;
	for( k= 0; k < fieldsCount; k++ )
	{
		field_info* field= &fields[k];
		variable* var= mm_staticMalloc( sizeof(variable) );

		var->name= cls_resolveConstantPoolIndexToUtf8( cls, field->name_index );
		var->descriptor= cls_resolveConstantPoolIndexToUtf8( cls, field->descriptor_index );
		var->access_flags= field->access_flags;
		
		/* this is a class instance variable */
		if( isFlagSet(field->access_flags, ACC_STATIC) )
		{
			var->slot_index= cls->class_instance_variable_slot_count;
		
			/* decide if we require one or two slots (i.e. 32 vs. 64 bits) */
			if( *var->descriptor == BASE_TYPE_LONG || *var->descriptor == BASE_TYPE_DOUBLE )
				cls->class_instance_variable_slot_count+= 2;
			else
				cls->class_instance_variable_slot_count++;
			
			/* add this variable to the according list */
			cls->class_instance_variable_table[currentClassInstanceTableIndex++]= var;
		}
		else /* this is an instance variable */
		{
			var->slot_index= cls->instance_variable_slot_count;
		
			/* decide if we require one or two slots (i.e. 32 vs. 64 bits) */
			if( *var->descriptor == BASE_TYPE_LONG || *var->descriptor == BASE_TYPE_DOUBLE )
				cls->instance_variable_slot_count+= 2;
			else
				cls->instance_variable_slot_count++;
			
			/* add this variable to the according list */
			cls->instance_variable_table[currentInstanceTableIndex++]= var;
		}
	}
	
	/* allocate memory for the static instance variable slots */
	cls->class_inctance_variable_slots= mm_staticMalloc( sizeof(int32) * cls->class_instance_variable_slot_count );
	
	mm_staticFree( fields );
}

/**********************************************************************************************
 * Methods
 **********************************************************************************************/

Code_attribute* readCodeAttribute( Class* cls, ClassLoaderState* cl )
{
	Code_attribute* code= mm_staticMalloc( sizeof(Code_attribute) );
	
	cl_readU2( cl ); /* attribute_name_index */
	cl_readU4( cl ); /* attribute_length */
	code->max_stack= cl_readU2( cl );
	code->max_locals= cl_readU2( cl );
	code->code_length= cl_readU4( cl );
	code->code= mm_staticMalloc( code->code_length );
	cl_readBytes( cl, code->code_length, code->code );
	
	/* read exception table */
	code->exception_table_length= cl_readU2( cl );
	code->exception_table_tab= NULL;
	
	if( code->exception_table_length > 0 )
	{
		code->exception_table_tab= mm_staticMalloc( code->exception_table_length * sizeof(exception_table*) );

		int i;
		for( i= 0; i < code->exception_table_length; i++ )
		{
			exception_table* ex= mm_staticMalloc( sizeof(exception_table) );
			
			ex->start_pc= cl_readU2( cl );
			ex->end_pc= cl_readU2( cl );
			ex->handler_pc= cl_readU2( cl );
			ex->catch_type= cl_readU2( cl );
			
			code->exception_table_tab[i]= ex;
		}
	}
	
	/* Skip code attribute's attributes, they only contain debugging information which we're not going to use (yet). */
	int attributes_count= cl_readU2( cl );
	
	if( attributes_count > 0 )
	{
		int i;
		for( i= 0; i < attributes_count; i++ )
		{
			cl_readU2( cl ); /* attribute name index */
			int attribute_size= cl_readU4( cl );
			cl_skipBytes( cl, attribute_size );
		}
	}
	
	return code;
}

int determineSlotCountFromDescriptor( const char* descriptor )
{
	int count= 0;
	boolean ignoreNext= false;
	
	while( *descriptor )
	{
		switch( *descriptor )
		{
			case BASE_TYPE_BYTE:
			case BASE_TYPE_CHAR:
			case BASE_TYPE_FLOAT:
			case BASE_TYPE_INT:
			case BASE_TYPE_SHORT:
			case BASE_TYPE_BOOLEAN:
				if( !ignoreNext )
					count++;
				else
					ignoreNext= false;
				break;
				
			case BASE_TYPE_LONG:
			case BASE_TYPE_DOUBLE:
				if( !ignoreNext )
					count+= 2;
				else
					ignoreNext= false;
				break;
				
			case BASE_TYPE_REFERENCE:
				if( !ignoreNext )
					count++;
				else
					ignoreNext= false;
				
				/* skip reference type */
				while( *descriptor != ';' )
					descriptor++;
					break;
				
			case BASE_TYPE_ONE_ARRAY_DIMENSION:
				count++;
				while( *(descriptor+1) == '[' )
					descriptor++;
					ignoreNext= true;
				break;
				
			case '(':
				break;
				
			case ')':
				return count; /* done */
				break;
				
			default:
				error( "Unrecognized descriptor type!" );
				break;
		}
		
		descriptor++;
	}
	
	/* we should never reach this */
	error( "Error while parsing type descriptor!" );
	return 0;
}

void readMethods( Class* cls, ClassLoaderState* cl )
{
	cls->methods_count= cl_readU2( cl );
	logVerbose( "Methods: %i\n", cls->methods_count );
	cls->methods= (method_info**)mm_staticMalloc( cls->methods_count * sizeof(method_info*) );

	int i;
	for( i= 0; i < cls->methods_count; i++ )
	{
		method_info* method= mm_staticMalloc( sizeof(method_info) );
		method->access_flags= cl_readU2( cl );
		u2 nameIndex= cl_readU2( cl );
		u2 nameDescriptor= cl_readU2( cl );
		int attributesCount= cl_readU2( cl );

		/* Resolve and store strings. */
		method->name= cls_resolveConstantPoolIndexToUtf8( cls, nameIndex );
		method->descriptor= cls_resolveConstantPoolIndexToUtf8( cls, nameDescriptor );
		
		logVerbose( "Method: %s%s attributes: %i\n", method->name, method->descriptor, attributesCount );
		
		/* pre-initialize for the case that fields will be unused */
		method->code= NULL;
		
		if( attributesCount > 0 )
		{
			int j;
			for( j= 0; j < attributesCount; j++ )
			{
				int attributeNameIndex= cl_peekNextU2( cl );
				char* attributeNameString= cls_resolveConstantPoolIndexToUtf8( cls, attributeNameIndex );
				
				/* is this a code attribute? */
				if( strcmp(attributeNameString, "Code") == 0 )
				{
					method->code= readCodeAttribute( cls, cl );
					continue;
				}
				
				/* Add other attributes here, if any. */
				
				/* skip unknown attribute */
				logVerbose( "--> Unrecognized method attribute: %s\n", attributeNameString );
				cl_readU2( cl );
				u4 length= cl_readU4( cl );
				cl_skipBytes( cl, length );
			}
		}
		
		/* Precalculate descriptor slot count. */
		method->parameterSlotCount= determineSlotCountFromDescriptor( method->descriptor );
		method->parameterSlotCount+= isFlagSet(method->access_flags, ACC_STATIC) ? 0 : 1;
		
		/* Add to method list. */
		cls->methods[i]= method;
	}
}

/* Tries to recursively resolve the given method, starting at class cls and going up through the hierarchy of super classes. */
method_info* cls_resolveMethod( Class** cls, const char* name, const char* descriptor )
{
	int i;
	for( i= 0; i < (*cls)->methods_count; i++ )
	{
		method_info* currentMethod= (*cls)->methods[i];
		
		if( strcmp(currentMethod->name, name) == 0 && strcmp(currentMethod->descriptor, descriptor) == 0 )
			return currentMethod;
	}
	
	/* Uppermost class? Abort search. Method not found. */
	if( (*cls)->superClass == NULL )
		return NULL;
	
	/* Not found yet? Recursively search through the super classes. */
	*cls= (*cls)->superClass;
	return cls_resolveMethod( cls, name, descriptor );
}

/* Looks for a method in the given class only. */
method_info* cls_getMethod( Class* cls, const char* name, const char* descriptor )
{
	int i;
	for( i= 0; i < cls->methods_count; i++ )
	{
		method_info* currentMethod= cls->methods[i];
	
		if( strcmp(currentMethod->name, name) == 0 && strcmp(currentMethod->descriptor, descriptor) == 0 )
			return currentMethod;
	}
	
	return NULL;
}

/* Recursively resolves a static field. */ 
variable* cls_resolveStaticField( Class** cls, const char* name, const char* descriptor )
{
	int i;
	for( i= 0; i < (*cls)->class_instance_variable_count ; i++ )
	{
		variable* currentVar= (*cls)->class_instance_variable_table[i];
		
		if( strcmp(currentVar->name, name) == 0 && strcmp(currentVar->descriptor, descriptor) == 0 )
			return currentVar;
	}
	
	/* Uppermost class? Abort search. Field not found. */
	if( (*cls)->superClass == NULL )
		return NULL;
	
	/* Not found yet? Recursively search through the super classes. */
	*cls= (*cls)->superClass;
	return cls_resolveStaticField( cls, name, descriptor );
}

/* Recursively resolves a field. */ 
variable* cls_resolveField( Class** cls, const char* name, const char* descriptor )
{
	int i;
	for( i= 0; i < (*cls)->instance_variable_count ; i++ )
	{
		variable* currentVar= (*cls)->instance_variable_table[i];
		
		if( strcmp(currentVar->name, name) == 0 && strcmp(currentVar->descriptor, descriptor) == 0 )
			return currentVar;
	}
	
	/* Uppermost class? Abort search. Field not found. */
	if( (*cls)->superClass == NULL )
		return NULL;
	
	/* Not found yet? Recursively search through the super classes. */
	*cls= (*cls)->superClass;
	return cls_resolveField( cls, name, descriptor );
}

CONSTANT_NameAndType_info* cls_resolveConstantPoolIndexToNameAndType( Class* cls, uint16 index )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	if( cpEntry->tag != CONSTANT_NameAndType )
		error( "CONSTANT_NameAndType expected but not found!" );

	CONSTANT_NameAndType_info* nat= (CONSTANT_NameAndType_info*) cpEntry;
	
	/* Resolve indices and store pointers for future use. */
	nat->name= cls_resolveConstantPoolIndexToUtf8( cls, nat->name_index );
	nat->descriptor= cls_resolveConstantPoolIndexToUtf8( cls, nat->descriptor_index );
	
	return nat;
}

/* Resolves a constant pool entry into a class and a method info. If the according class has not already been loaded, it will be loaded now. */
void cls_resolveConstantPoolIndexToClassAndMethodInfo( Class* cls, uint16 index, Class** otherClass, method_info** methodInfo )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	if( cpEntry->tag != CONSTANT_Methodref )
		error( "CONSTANT_Methodref expected but not found!" );
	
	CONSTANT_Methodref_info* constMethodInfo= (CONSTANT_Methodref_info*)cpEntry;
	
	/* Resolve class name and NameAndType_info, if they haven't been resolved yet. Store according pointers for future use. */
	if( constMethodInfo->class == NULL )
	{
		constMethodInfo->class= cls_resolveConstantPoolIndexToClass( cls, constMethodInfo->class_index );
		CONSTANT_NameAndType_info* nameAndType= cls_resolveConstantPoolIndexToNameAndType( cls, constMethodInfo->name_and_type_index );
		
		/* get appropriate class from method area, load if not present */
		constMethodInfo->class= ma_getClass( constMethodInfo->class->className );
		
		/* get appropriate method_info */
		constMethodInfo->methodInfo= cls_resolveMethod( &(constMethodInfo->class), nameAndType->name, nameAndType->descriptor );

		if( constMethodInfo->methodInfo == NULL )
		{
			logVerbose( "Method not found %s.%s%s\n", cls_resolveConstantPoolIndexToUtf8(cls, constMethodInfo->class_index), nameAndType->name, nameAndType->descriptor );
			error( "Execution haltet." );
		}
		
		logVerbose( "\tMethod resolved.\n" );
	}
	
	*otherClass= constMethodInfo->class;
	*methodInfo= constMethodInfo->methodInfo;
	
	return;
}

/* Resolves a constant pool entry into a class and a method info. If the according interface has not already been loaded, it will be loaded now. 
	This method handles the resolution of interface methods. First, the presence of the interface itself is validated, then a dynamic lookup for an implementation
	is performed. */
void cls_resolveConstantPoolIndexToClassAndInterfaceMethodInfo( Class* cls, uint16 index, Class** objRefClass, method_info** methodInfo )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	if( cpEntry->tag != CONSTANT_InterfaceMethodref )
		error( "CONSTANT_InterfaceMethodref expected but not found!" );
	
	CONSTANT_InterfaceMethodref_info* constInterfaceMethodInfo= (CONSTANT_InterfaceMethodref_info*)cpEntry;

	if( constInterfaceMethodInfo->class == NULL )
	{
		/* resolve class and method */
		constInterfaceMethodInfo->class= cls_resolveConstantPoolIndexToClass( cls, constInterfaceMethodInfo->class_index );
		CONSTANT_NameAndType_info* nameAndType= cls_resolveConstantPoolIndexToNameAndType( cls, constInterfaceMethodInfo->name_and_type_index );
	
		/* Resolve appropriate method from the given class or its super classes. */
		constInterfaceMethodInfo->methodInfo= cls_resolveMethod( &(constInterfaceMethodInfo->class), nameAndType->name , nameAndType->descriptor );
	
		/* Now verify that we correctly have an interface class with a matching method declaration here. */
		if( !isFlagSet(constInterfaceMethodInfo->class->access_flags, ACC_INTERFACE) )
			error( "IncompatibleClassChangeError: The given interface does not resolve to an interface class but a normal class instead." );
	
		if( constInterfaceMethodInfo->methodInfo == NULL )
		{
			logVerbose( "Method not found %s.%s%s\n", cls_resolveConstantPoolIndexToUtf8(cls, constInterfaceMethodInfo->class_index), nameAndType->name, nameAndType->descriptor );
			error( "Execution haltet." );
		}
		
		logVerbose( "\tInterface resolved.\n" );
	}
	
	/* Okay, we verified the existence of the interface and its method, look for an appropriate implementation now, starting with the class of objref,
		method call (objRefClass), that has been passed by the caller. */
	*methodInfo= cls_resolveMethod( objRefClass, constInterfaceMethodInfo->methodInfo->name, constInterfaceMethodInfo->methodInfo->descriptor );	
	return;
}

/* Resolves a constant pool entry of a methodref into the method's name. Only used in fatal error cases, no optimization necessary. */
void cls_resolveConstantPoolIndexOfMethodRefToMethodNameAndDescriptor( Class* cls, uint16 index, char** className, char** methodName, char** methodDescriptor )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	if( cpEntry->tag != CONSTANT_Methodref )
		error( "CONSTANT_Methodref expected but not found!" );
	
	/* resolve class and method name/descriptor */
	CONSTANT_Methodref_info* constMethodInfo= (CONSTANT_Methodref_info*)cpEntry;
	*className= cls_resolveConstantPoolIndexToClassName( cls, constMethodInfo->class_index );
	CONSTANT_NameAndType_info* nameAndType= cls_resolveConstantPoolIndexToNameAndType( cls, constMethodInfo->name_and_type_index );
	*methodName= cls_resolveConstantPoolIndexToUtf8( cls, nameAndType->name_index );
	*methodDescriptor= cls_resolveConstantPoolIndexToUtf8( cls, nameAndType->descriptor_index );

	return;
}

/* Resolves a constant pool entry into a class and a field info. If the according class has not already been loaded, it will be loaded now. */
void cls_resolveConstantPoolIndexToClassAndVariableInfo( Class* cls, uint16 index, Class** otherClass, variable** variableInfo, boolean isStaticField )
{
	/* calculate address of cp entry */
	cp_info* cpEntry= *(cls->constant_pool+index);
	
	if( cpEntry->tag != CONSTANT_Fieldref )
		error( "CONSTANT_Fieldref expected but not found!" );
	
	CONSTANT_Fieldref_info* fieldrefInfo= (CONSTANT_Fieldref_info*)cpEntry;

	if( fieldrefInfo->class == NULL )
	{
		/* resolve class and field name/descriptor */
		fieldrefInfo->class= cls_resolveConstantPoolIndexToClass( cls, fieldrefInfo->class_index );
	
		CONSTANT_NameAndType_info* nameAndType= cls_resolveConstantPoolIndexToNameAndType( cls, fieldrefInfo->name_and_type_index );
	
		/* get appropriate variable info (which probably also adjusts the class) */
		if( isStaticField )
			fieldrefInfo->variableInfo= cls_resolveStaticField( &(fieldrefInfo->class), nameAndType->name , nameAndType->descriptor );	
		else
			fieldrefInfo->variableInfo= cls_resolveField( &(fieldrefInfo->class), nameAndType->name , nameAndType->descriptor );
		
		logVerbose( "\tField resolved.\n" );
	}
	
	*otherClass= fieldrefInfo->class;
	*variableInfo= fieldrefInfo->variableInfo;
	
	return;
}

/* Recursively determines if the given class or its super classes implement the given interface. */
boolean cls_implementsInterface( Class* cls, Class* interf )
{
	uint16 index;
	Class* possibleInterf;
	int i;
	for( i= 0; i < cls->interfaces_count; i++ )
	{
		index= cls->interfaces[i];
		possibleInterf= cls_resolveConstantPoolIndexToClass( cls, index );
		
		if( possibleInterf == interf )
			return true;
		
		boolean rekRes= cls_implementsInterface( possibleInterf, interf );
		
		if( rekRes )
			return true;
	}
	
	if( cls->superClass == NULL )
		return false;
	
	return cls_implementsInterface( cls->superClass, interf );	
}

/**********************************************************************************************
 * Class file handling
 **********************************************************************************************/

void cls_initArrayClass( Class* cls, const char* type )
{
	char* typeCopy= mm_staticMalloc( strlen(type)+1 );
	strcpy( typeCopy, type );
	
	cls->access_flags= ACC_FINAL|ACC_PUBLIC;
	cls->class_inctance_variable_slots= 0;
	cls->class_instance_variable_count= 0;
	cls->class_instance_variable_slot_count= 0;
	cls->class_instance_variable_table= NULL;
	cls->className= typeCopy;
	cls->constant_pool= NULL;
	cls->constant_pool_count= 0;
	cls->instance_variable_count= 0;
	cls->instance_variable_slot_count= 0;
	cls->instance_variable_table= NULL;
	cls->interfaces= NULL;
	cls->interfaces_count= 0;
	cls->isInitialized= false;
	cls->methods= NULL;
	cls->methods_count= 0;
	cls->superClass= ma_getClass( "java/lang/Object" );
	cls->sourceFileName= NULL;
}

void cls_load( Class* cls, ClassLoaderState* cl )
{
	logVerbose( "Parsing class data...\n" );
	
	/* read class file header */
	u4 magic= cl_readU4( cl );
	u2 minorVersion= cl_readU2( cl );
	u2 majorVersion= cl_readU2( cl );
	cls->constant_pool_count= cl_readU2( cl );
	
	logVerbose( "Magic: 0x%X\nmajor version: %i\nminor version: %i\n", magic, majorVersion, minorVersion );
	
	if( magic != 0xCAFEBABE )
		error( "Magic does not match! -> Not a Java class file?" );

	/* TODO: Which version numbers are we going to support? */

	/* constant pool */
	logVerbose( "Number of constant pool entries: %i\n", cls->constant_pool_count-1 );

	/* Allocate a pointer list. The entries will point to the separate cp entries after loading */
	cls->constant_pool= (cp_info**)mm_staticMalloc( cls->constant_pool_count * sizeof(cp_info*) );
	readConstantPoolEntries( cls, cl );
	
	cls->access_flags= cl_readU2( cl );
	
	u2 thisClassIndex= cl_readU2( cl );
	cls->className= cls_resolveConstantPoolIndexToClassName( cls, thisClassIndex );

	u2 superClassIndex= cl_readU2( cl );
	if( superClassIndex == 0 )
		cls->superClass= NULL;
	else
		cls->superClass= ma_getClass( cls_resolveConstantPoolIndexToClassName(cls, superClassIndex) );

	/* interfaces */
	cls->interfaces_count= cl_readU2( cl );
	
	logVerbose( "Interfaces: %i\n", cls->interfaces_count );
	
	cls->interfaces= (u2*)mm_staticMalloc( cls->interfaces_count * sizeof(u2) );
	
	int i;
	for( i= 0; i < cls->interfaces_count; i++ )
	{
		cls->interfaces[i]= cl_readU2( cl );
		logVerbose( "Interface: <#%i>\n", cls->interfaces[i] );
	}
	
	/* fields */
	readFields( cls, cl );
	
	/* methods */
	readMethods( cls, cl );
	
	/* unrelated attributes */
	cls->sourceFileName= NULL;
	u2 attributesCount= cl_readU2( cl );
	
	logVerbose( "Attributes: %i\n", attributesCount );

	int n;
	for( n= 0; n < attributesCount; n++ )
	{
		uint16 attributeNameIndex= cl_readU2( cl );
		const char* attributeName= cls_resolveConstantPoolIndexToUtf8( cls, attributeNameIndex );
		uint32 attributeLength= cl_readU4( cl );
			
		logVerbose( "Attribute: \"%s\", %i bytes\n", attributeName, attributeLength );
		
		if( strcmp(attributeName, "SourceFile") == 0 )
		{
			u2 sourceFileNameIndex= cl_readU2( cl );
			cls->sourceFileName= cls_resolveConstantPoolIndexToUtf8( cls, sourceFileNameIndex );
			continue;
		}
		
		cl_skipBytes( cl, attributeLength );
	}
	
	/* mark that initialization is still pending */
	cls->isInitialized= false;
	logVerbose( "Done parsing class data.\n" );
}
