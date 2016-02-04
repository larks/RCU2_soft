#include <cstdio>
#include "TestFixture.hpp"

static const char RED[] = "\e[31m";
static const char GREEN[] = "\e[32m";
static const char YELLOW[] = "\e[33m";
static const char NONE[] = "\e[0m";


TestFixture::TestFixture()
	: passes(0), fails(0), warnings(0)
{
}

TestFixture::~TestFixture()
{
}


void
TestFixture::assert( bool cond, const std::string &rep )
{
	if ( !cond ) {
		fprintf( stderr, "[%sFail%s] `%s' failed.\n", RED, NONE, rep.c_str() );
		++fails;
	} else {
		printf( "[%s OK %s] `%s' passed.\n", GREEN, NONE, rep.c_str() );
		++passes;
	}
}


void
TestFixture::warn( bool cond, const std::string &rep )
{
	if ( !cond ) {
		printf( "[%sWarn%s] `%s' failed.\n", YELLOW, NONE, rep.c_str() );
		++warnings;
	} else {
		printf( "[%s OK %s] `%s' passed.\n", GREEN, NONE, rep.c_str() );
		++passes;
	}
}


void
TestFixture::report()
{
	printf( "%s%8d%s passed.\n", GREEN, passes, NONE );
	if ( warnings )
		printf( "%s%8d%s warnings.\n", YELLOW, warnings, NONE );
	if ( fails )
		printf( "%s%8d%s fails.\n", RED, fails, NONE );
}


bool
TestFixture::passed()
{
	return fails == 0;
}

