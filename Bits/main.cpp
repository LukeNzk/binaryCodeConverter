#include <windows.h>
#include <fstream>
#include <iostream>
#include <assert.h>

typedef unsigned char Uint8;
typedef unsigned int Uint32;
typedef char AnsiChar;
typedef bool Bool;

class SimpleBuffer
{
public:
	SimpleBuffer()
		: m_data( nullptr )
		, m_size( 0 )
		, m_offset( 0 )
	{
	}

	~SimpleBuffer()
	{
		delete m_data;
	}

	void Resize( Uint32 size )
	{
		assert( size < 512 ); // If I write so big function I'm awesome ( larger than 256 B )
		m_size = size;
		m_data = realloc( m_data, size );
	}

	void Write( AnsiChar c )
	{
		if ( m_size == m_offset )
		{
			Resize( m_size * 2 + 1 );
		}

		( reinterpret_cast< char* >( m_data ) )[ m_offset++ ] = c;
	}

	const void* GetData() const
	{
		return m_data;
	}

	inline Uint32 Count() const { return m_offset + 1; }

private:
	void*	m_data;
	size_t	m_size;
	size_t	m_offset;
};

class Function
{
public:
	Function( const SimpleBuffer* buffer )
	{
		m_size = buffer->Count();

		m_byteCode = ( Uint8* )VirtualAlloc( NULL, buffer->Count(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
		CopyMemory( m_byteCode, buffer->GetData(), buffer->Count() );
	}
	
	const void* Get() const
	{
		return m_byteCode;
	}

	~Function()
	{
		VirtualFree( m_byteCode, m_size, MEM_RELEASE );
	}

private:
	size_t	m_size;
	Uint8*	m_byteCode;
};

class CodeParser
{
public:
	Function* Parse( const AnsiChar* text )
	{
		const AnsiChar* code = text;

		Uint8 bitCounter = 0;
		Uint8 currentByte = 0;
		Uint32 byteCounter = 0;

		AnsiChar c = *code;
		Bool isComment = false;

		SimpleBuffer buffer;

		while ( c )
		{
			isComment |= IsComment( c );

			if ( isComment )
			{
				if ( IsEOL( c ) )
					isComment = false;				
			}
			else
			{
				if ( IsEOL( c ) || IsWhiteSpace( c ) )
				{
					c = *( ++text );
					continue;
				}

				if ( bitCounter == 8 )
				{
					buffer.Write( currentByte );

					bitCounter = 0;
					currentByte = 0;
				}

				currentByte = ( currentByte << 1 ) | ( c == '1' ? 1 : 0 );

				++bitCounter;
			}

			c = *(++text);
		}

		buffer.Write( currentByte );

		Function* func = new Function( &buffer );
		return func;
	}

private:
	inline Bool IsEOL( char c )
	{
		return ( c == '\r' || c == '\n' );
	}

	inline Bool IsEOF( char c )
	{
		return ( c == 0 );
	}

	inline Bool IsWhiteSpace( char c )
	{
		return c == ' ';
	}

	inline Bool IsComment( char c )
	{
		return c == ';';
	}
};

void* LoadFile( const char* fileName )
{
	std::ifstream stream( fileName, std::ios::binary | std::ios::ate | std::ios::in );

	int lastError = GetLastError();
	assert( stream.is_open() );
	
	std::streampos size = stream.tellg();

	void* result = new char[ size ];
	stream.seekg( 0, std::ios::beg );
	stream.read( ( char* )result, size );
	stream.close();

	return result;
}

#define RET_INT( x ) reinterpret_cast< int(*)() >( x )()

int main( int argc, char** argv )
{

	Function* function = nullptr;

	if ( argc == 2 )
	{
		char* fileName = argv[ 1 ];
		const AnsiChar* source = (char*)LoadFile( fileName );

		CodeParser parser;
		function = parser.Parse( source );
	}

	if ( function )
	{
		int result = RET_INT( function->Get() );
		printf( "Returned: %d", result );
		return 0;
	}

	return -1;
}