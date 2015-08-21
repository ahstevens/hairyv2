#include "Slice.h"


Slice::Slice(void)
	: width( 1.0f ), height( 1.0f )
{
}

Slice::Slice( float width, float height )
	: width( width ), height( height )
{
}

Slice::Slice( float width, float height, std::vector<Seed> seeds )
	: width( width ), height( height ), seeds( seeds )
{
}

Slice::~Slice(void)
{
}

void Slice::setWidth( float width )
{
	this->width = width;
}

float Slice::getWidth( void )
{
	return width;
}

void Slice::setHeight( float height )
{
	this->height = height;
}

float Slice::getHeight( void )
{
	return height;
}

void Slice::addSeed( Seed s )
{
	seeds.push_back( s );
}

void Slice::addSeeds( std::vector<Seed> seeds )
{
	std::vector<Seed> newSeeds;
	newSeeds.reserve( this->seeds.size() + seeds.size() );
	newSeeds.insert( newSeeds.end(), this->seeds.begin(), this->seeds.end() );
	newSeeds.insert( newSeeds.end(), seeds.begin(), seeds.end() );
	this->seeds = newSeeds;
}

void Slice::removeSeed( void )
{
	seeds.pop_back();
}

void Slice::removeSeeds( int n )
{
	for( int i = 0; i < n; ++i ) seeds.pop_back();
}

void Slice::clearSeeds( void )
{
	seeds.clear();
}


void Slice::redraw( Shader shader )
{

}