#pragma once
#include <string>

#define TEST( tf, cond ) tf.assert( cond, std::string(#cond) )
#define WARN( tf, cond ) tf.warn( cond, std::string(#cond) )

class TestFixture {
public :
	TestFixture();
	~TestFixture();

	void assert( bool, const std::string & );
	void warn( bool, const std::string & );

	void report();
	bool passed();

private :
	unsigned passes;
	unsigned fails;
	unsigned warnings;
};
