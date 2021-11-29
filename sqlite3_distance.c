  /*
 * distance - hamming distance between same-length strings
 * in SQLite3 in a variety of numbered and named bases
 * Copyright 2021 Jason Welbourne <theorganicgypsy@gmail.com>
 * All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#define NAME				distance
#define STRNX(x)			#x
#define STR(x)				STRNX(x)
#define CATNX(a, b)			a ## b
#define CAT(a, b)			CATNX(a, b)
#ifndef SQLITE_SKIP_UTF8
#define SQLITE_SKIP_UTF8(x) if ((*((x)++)) >= 0xc0) { while ((*(x) & 0xc0) == 0x80){ (x)++; }; };	
#define NEXT(x)				(x)++;
#endif
#define HT2(n)				n,		n+1,		n+1,		n+2
#define HT4(n)				HT2(n),	HT2(n+1),	HT2(n+1),	HT2(n+2)
#define HT6(n)				HT4(n),	HT4(n+1),	HT4(n+1),	HT4(n+2)
static const unsigned char hammingTable[256] = { HT6(0), HT6(1), HT6(1), HT6(2) };
#undef HT2
#undef HT4
#undef HT6

#define characters			256
#define keySize				13
#define builderNodeArraySize		26
#define builderClassNodeArraySize	23
#define tableNodeArraySize		270
#define forceSameLength			0
#define allowDebugTable			1
#define skipUTF				1

typedef struct TableNode {
	unsigned char key[keySize];
	unsigned char value[characters]; } TableNode_type;

/* BuilderNode Notes
valueEnd 255:     increase value as high as needed to complete the set
valueEnd else:    literal
Example:
	"65-84",			65,		84,		{	{	33,	0,	255, 	},	},
base 65:
	chr(33) = 0,
	chr(34) = 1,
	chr(35) = 2,
	...,
	chr(33+65-1) = 64,
base 84:
	chr(33) = 0,
	chr(34) = 1,
	...,
	chr(33+84-1) = 83,	
By using 255 as a placeholder for the highest character we might need to use in any particular base,
we can use 1 definition to service an entire range
*/
typedef struct BuilderNode {
	unsigned char charStart;
	unsigned char valueStart;
	unsigned char valueEnd; } BuilderNode_type;

typedef struct BuilderClassNode {
	unsigned char key[keySize];
	unsigned char keyStart;
	unsigned char keyEnd;
	struct BuilderNode builders[builderNodeArraySize]; } BuilderClassNode_type;

TableNode_type TableNodes[tableNodeArraySize];
BuilderClassNode_type BuilderClassNodes[builderClassNodeArraySize] = {
	"2-10", 			2,		10,		{	{	'0',	0,	255, 	},	},
	"11-31",			11,		31,		{	{	'0',	0,	9, 	}, 
										{	'A',	10,	255, 	},
										{	'a',	10,	255, 	},	},
	"32",				32,		32,		{	{	'A',	0,	25,	},
										{	'a',	0,	25,	},
										{	'2',	26,	31,	},	},
	"32_RFC4648",			32,		32,		{	{	'A',	0,	25, 	},
										{	'2',	26,	31,	},	},
	"32_Crockford",			32,		32,		{	{	'0',	0,	9,	},
										{	'A',	10,	17,	},
										{	'J',	18,	19,	},
										{	'M',	20,	21,	},
										{	'P',	22,	31,	},	},
	"32_Hex",			32,		32,		{	{	'0',	0,	9,	},
										{	'A',	10,	31,	},	},
	"32_Geohash",			32,		32,		{	{	'0',	0,	9,	},
										{	'b',	10,	16,	},
										{	'j',	17,	18,	},
										{	'm',	19,	20,	},
										{	'p',	21,	31,	},	},
	"32_WordSafe",			32,		32,		{	{	'2',	0,	7,	},
										{	'C',	8,	8,	},
										{	'F',	9,	11,	},
										{	'J',	12,	12,	},
										{	'M',	13,	13,	},
										{	'P',	14,	16,	},
										{	'V',	17,	19,	},
										{	'c',	20,	20,	},
										{	'f',	21,	23,	},
										{	'j',	24,	24,	},
										{	'm',	25,	25,	},
										{	'p',	26,	28,	},
										{	'v',	29,	31,	},	},
	"z_base_32",			32,		32,		{	{	'y',	0,	0,	},
										{	'b',	1,	1,	},
										{	'n',	2,	2,	},
										{	'd',	3,	3,	},
										{	'r',	4,	4,	},
										{	'f',	5,	6,	},
										{	'8',	7,	7,	},
										{	'e',	8,	8,	},
										{	'j',	9,	10,	},
										{	'm',	11,	11,	},
										{	'c',	12,	12,	},
										{	'p',	13,	14,	},
										{	'x',	15,	15,	},
										{	'o',	16,	16,	},
										{	't',	17,	17,	},
										{	'1',	18,	18,	},
										{	'u',	19,	19,	},
										{	'w',	20,	20,	},
										{	'i',	21,	21,	},
										{	's',	22,	22,	},
										{	'z',	23,	23,	},
										{	'a',	24,	24,	},
										{	'3',	25,	27,	},
										{	'h',	28,	28,	},
										{	'6',	29,	29,	},
										{	'9',	30,	30, 	},	},
	"33-36",			33,		36,		{	{	'A',	0,	25,	},
										{	'0',	26,	255,	},	},
	"37-52",			37,		52,		{	{	'A',	0,	25,	},
										{	'a',	26,	255, 	},	},
	"53-57",			59,		62,		{	{	'A',	0,	25,  	},
										{	'a',	26,	51,	},
										{	'0',	52,	255,	},	},
	"58",				58,		58, 		{	{	'1',	0,	8,   	},
										{	'A',	9,	16,  	},
										{	'J',	17,	21,  	},
										{	'P',	22,	32,  	},
										{	'a',	33,	43,  	},
										{	'm',	44,	57,  	},	},
	"59-62",			59,		62,		{	{	'A',	0,	25,  	},
										{	'a',	26,	51,	},
										{	'0',	52,	255,	},	},
	"63",				63,		63, 		{	{	'A',	0,	25,	},
										{	'a',	26,	51,	},
										{	'0',	52,	61,	},
										{	'+',	62,	62,	},	},
	"64",				64,		64,		{	{	'A',	0,	25,	},
										{	'a',	26,	51,	},
										{	'0',	52,	61,	}, 
										{	'+',	62,	62,	},
										{	'/',	63,	63,	},	},
	"65-84",			65,		84,		{	{	33,	0,	255, 	},	},
	"85",				85,		85,	 	{ 	{	33,	0,	84, 	},	},
	"ascii85",			85,		85,	 	{ 	{	33,	0,	84, 	},	},
	"z85",				85,		85,		{	{	'0',	0,	9,	},
										{	'a',	10,	35,	},
										{	'A',	36,	61,	},
										{	'.',	62,	62,	},
										{	'-',	63,	63,	},
										{	':',	64,	64,	},
										{	'+',	65,	65,	},
										{	'=',	66,	66,	},
										{	'^',	67,	67,	},
										{	'!',	68,	68,	},
										{	'/',	69,	69,	},
										{	'*',	70,	70,	},
										{	'?',	71,	71,	},
										{	'&',	72,	72,	},
										{	'<',	73,	73,	},
										{	'>',	74,	74,	},
										{	'(',	75,	76,	},
										{	'[',	77,	77,	},
										{	']',	78,	78,	},
										{	'{',	79,	79,	},
										{	'}',	80,	80,	},
										{	'@',	81,	81,	},
										{	'%',	82,	82,	},
										{	'$',	83,	83,	},
										{	'#',	84,	84,	},	},
	"85_RFC1924",			85,		85,		{	{	'0',	0,	9,	},
										{	'A',	10,	35,	},
										{	'a',	36,	61,	},
										{	'!',	62,	62,	},
										{	'#',	63,	63,	},
										{	'$',	64,	64,	},
										{	'%',	65,	65,	},
										{	'&',	66,	66,	},
										{	'(',	67,	67,	},
										{	')',	68,	68,	},
										{	'*',	69,	69,	},
										{	'+',	70,	70,	},
										{	'-',	71,	71,	},
										{	';',	72,	72,	},
										{	'<',	73,	73,	},
										{	'=',	74,	74,	},
										{	'>',	75,	75,	},
										{	'?',	76,	76,	},
										{	'@',	77,	77,	},
										{	'^',	78,	78,	},
										{	'_',	79,	79,	},
										{	'`',	80,	80,	},
										{	'{',	81,	81,	},
										{	'|',	82,	82,	},
										{	'}',	83,	83,	},
										{	'~',	84,	84,	},	},
	"86-255",			86,		255, 		{	{	0,	0,	255, 	},	},	
	"basE91",			91,		91,		{	{	'A',	0,	25,	},
										{	'a',	26,	51,	},
										{	'0',	52,	61,	},
										{	'!',	62,	62,	},
										{	'#',	63,	66,	},
										{	'(',	67,	71,	},
										{	'.',	72,	73,	},
										{	':',	74,	78,	},
										{	'?',	79,	80,	},
										{	'[',	81,	81,	},
										{	']',	82,	82,	},
										{	'^',	83,	85,	},
										{	'{',	86,	89,	},
										{	'"',	90,	90,	},	},	};

static void CAT(NAME, _func)(sqlite3_context *ctx, int args, sqlite3_value **argv) {
	const unsigned char *a, *b;
	const unsigned char *type;
	const unsigned char *baseAsString;
	unsigned int baseAsInt = 0;
	int res = 0;
	int tableIndex = -1;
	// printf("Starting Up!\n");
	if (args != 4) {
		sqlite3_result_error(ctx, STR(NAME)": need 4 args", -1);
		return; };
	type = sqlite3_value_text(argv[0]);
	baseAsInt = sqlite3_value_int(argv[1]);
	baseAsString = sqlite3_value_text(argv[1]);
	a = sqlite3_value_text(argv[2]);
	b = sqlite3_value_text(argv[3]);
	// printf("Still Running!\n");
	// printf("baseAsString:  %16s\n",baseAsString);
	// printf("baseAsInt:     %4d\n",baseAsInt);
	// printf("bAI < 2:	   %1d\n",(baseAsInt < 2));
	if (baseAsInt) {
		if ((baseAsInt < 2) || (baseAsInt > 256)) {
			sqlite3_result_error(ctx, STR(NAME)": arg 2 has invalid value (2-256)",-1);
			return;
		} else {
			for (unsigned int i=0; i < tableNodeArraySize ; i += 1) {
				if (!strcmp(TableNodes[i].key, baseAsString)) {
					// printf("Found an Index!\n");
					tableIndex = i; }; }; };
	} else {
		if (baseAsString) {
			for (unsigned int i=0; i < tableNodeArraySize ; i += 1) {
				if (!strcmp(TableNodes[i].key, baseAsString)) {
					// printf("Found an Index!\n");
					tableIndex = i; }; };
		} else {
			sqlite3_result_error(ctx, STR(NAME)": arg 2 has no matching value",-1);
			return; }; };			
	if (tableIndex > -1) {
		if (!a || !b) {
			if (allowDebugTable) {
				printf("%s:\n",TableNodes[tableIndex].key);
				for (int j=0; j < characters; j += 8) {
					printf("%4d: %4d %4d %4d %4d %4d %4d %4d %4d\n",
						j,
						TableNodes[tableIndex].value[j],
						TableNodes[tableIndex].value[j+1],
						TableNodes[tableIndex].value[j+2],
						TableNodes[tableIndex].value[j+3],
						TableNodes[tableIndex].value[j+4],
						TableNodes[tableIndex].value[j+5],
						TableNodes[tableIndex].value[j+6],
						TableNodes[tableIndex].value[j+7]); }; };
			sqlite3_result_error(ctx, STR(NAME)": arg 3 or 4 has no text value",-1);
			return; };
		if (!strcmp("hamming",type)||!strcmp("h",type)) {
			if (forceSameLength) {
				while (*a && *b) {
					res += hammingTable[TableNodes[tableIndex].value[*a] ^ TableNodes[tableIndex].value[*b]];
					if (skipUTF) {
						SQLITE_SKIP_UTF8(a);
						SQLITE_SKIP_UTF8(b); 
					} else {
						NEXT(a);
						NEXT(b); }; };
				if (*a || *b) {
					sqlite3_result_error(ctx, STR(NAME)": args not same length", -1);
					return; };
			} else {
				while (*a || *b) {
					res += hammingTable[((a) ? TableNodes[tableIndex].value[*a] : 0) ^ ((b) ? TableNodes[tableIndex].value[*b] : 0)];
					if (skipUTF) {
						SQLITE_SKIP_UTF8(a);
						SQLITE_SKIP_UTF8(b);
					} else {
						NEXT(a);
						NEXT(b); }; }; };
			sqlite3_result_int(ctx, res); 
		} else {
			sqlite3_result_error(ctx, STR(NAME)": arg 1 has invalid value!\n    hamming\n",-1);	
		}
	} else {
		sqlite3_result_error(ctx, STR(NAME)": arg 2 has invalid value, use a number or a named encoding scheme ( ascii85, etc. )",-1);
		return; }; };

int CAT(NAME, _init)(sqlite3 *db, char **errmsg, const sqlite3_api_routines *api) {
	SQLITE_EXTENSION_INIT2(api);
	int rc = SQLITE_OK;
	unsigned long currentTableNumber = 0;
	for (unsigned int index=0; index < builderClassNodeArraySize; index += 1) {
		for (unsigned int base=BuilderClassNodes[index].keyStart; base <= ((BuilderClassNodes[index].keyEnd == 255) ? 255 : BuilderClassNodes[index].keyEnd); base += 1) {
			unsigned int indexBuilder = 0;
			unsigned char key[keySize];
			if (BuilderClassNodes[index].keyStart == BuilderClassNodes[index].keyEnd) {
				strcpy(key,BuilderClassNodes[index].key);
			} else {
				sprintf(key,"%u",base); };
			while ((indexBuilder < builderNodeArraySize) && 
				!(	(BuilderClassNodes[index].builders[indexBuilder].charStart == 0) && 
					(BuilderClassNodes[index].builders[indexBuilder].valueStart == 0) && 
					(BuilderClassNodes[index].builders[indexBuilder].valueEnd == 0))) {
				for (unsigned char value=BuilderClassNodes[index].builders[indexBuilder].valueStart; 
					value <= ((BuilderClassNodes[index].builders[indexBuilder].valueEnd == 255) ? base-1 : BuilderClassNodes[index].builders[indexBuilder].valueEnd); 
					value += 1) {
					strcpy(TableNodes[currentTableNumber].key, key);
					TableNodes[currentTableNumber].value[
						BuilderClassNodes[index].builders[indexBuilder].charStart + value - BuilderClassNodes[index].builders[indexBuilder].valueStart] = value; };
				indexBuilder += 1; }; 
			currentTableNumber += 1; }; };
	printf("Loaded sqlite3_distance!\n");
	printf("    any string or integer representing an integer between 2 and 255 (examples: 2, '2', 10, '16', '64'...)\n");
	for (int i=0; i < tableNodeArraySize; i += 1) {
		int numeric = 1;
		int j = 0;
		int running = 1;
		while (running) {
			if ((TableNodes[i].key[j] == '\0')||(!(j < keySize))) {
				running = 0; 
			} else {
				if ((TableNodes[i].key[j] < '0')||(TableNodes[i].key[j] > '9')) {
					numeric = 0; 
					running = 0;
				} else {
					j += 1; }; }; };
		if (!numeric) {
			printf("    %s\n",TableNodes[i].key); }; };
	printf("Usage:\n");
	printf("    SELECT distance({type},{base},{stringA},{stringB});\n");
	printf("Print Tables:\n");	
	printf("    SELECT distance({type},{base},NULL,NULL);\n");
	printf("Examples:\n");
	printf("    SELECT distance('hamming',2,'0110','1001');\n");
	printf("    SELECT distance('hamming','ascii85','Aa123','Bb456');\n");
	printf("    SELECT distance('hamming','64','Aa123','Bb456');\n");
	printf("    SELECT distance('hamming','64',NULL,NULL);\n");

	sqlite3_create_function_v2(
		db,
		STR(NAME),
		4,
		SQLITE_UTF8, 
		NULL,
		CAT(NAME, _func), 
		NULL, 
		NULL, 
		NULL);
	return rc; };
